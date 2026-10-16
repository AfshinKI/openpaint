// SPDX-License-Identifier: GPL-3.0-or-later
#include "updates/UpdateService.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTemporaryDir>
#include <QtTest>
#include <cstring>

struct Response {
    QByteArray body = R"({"tag_name":"v999.0.0","draft":false,"prerelease":false})";
    int status = 200;
    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    QList<QPair<QByteArray, QByteArray>> headers;
    bool finish = true;
};
class FakeReply : public QNetworkReply {
  public:
    FakeReply(const QNetworkRequest& request, Response response, QObject* parent)
        : QNetworkReply(parent), body_(response.body) {
        setRequest(request);
        setUrl(request.url());
        setOperation(QNetworkAccessManager::GetOperation);
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, response.status);
        for (const auto& header : response.headers)
            setRawHeader(header.first, header.second);
        open(QIODevice::ReadOnly);
        if (response.finish)
            QTimer::singleShot(0, this, [this, response] {
                if (isFinished())
                    return;
                if (response.error != NoError)
                    setError(response.error, "Synthetic failure");
                emit readyRead();
                if (isFinished())
                    return;
                setFinished(true);
                emit finished();
            });
    }
    void abort() override {
        if (isFinished())
            return;
        setError(OperationCanceledError, "Cancelled");
        setFinished(true);
        emit finished();
    }
    qint64 bytesAvailable() const override {
        return body_.size() - offset_ + QNetworkReply::bytesAvailable();
    }

  protected:
    qint64 readData(char* data, qint64 maximum) override {
        const auto count = std::min(maximum, qint64(body_.size() - offset_));
        if (count <= 0)
            return -1;
        std::memcpy(data, body_.constData() + offset_, size_t(count));
        offset_ += count;
        return count;
    }

  private:
    QByteArray body_;
    qint64 offset_ = 0;
};
class FakeNetwork : public QNetworkAccessManager {
  public:
    Response next;
    QList<QNetworkRequest> requests;

  protected:
    QNetworkReply* createRequest(Operation operation, const QNetworkRequest& request,
                                 QIODevice* outgoing) override {
        Q_ASSERT(operation == GetOperation && !outgoing);
        requests.append(request);
        return new FakeReply(request, next, this);
    }
};
class UpdatesTest : public QObject {
    Q_OBJECT
  private slots:
    void versionComparison() {
        bool valid = false;
        QVERIFY(UpdateService::newerStable("v1.10.0", "1.9.0", &valid));
        QVERIFY(valid);
        QVERIFY(!UpdateService::newerStable("v1.9.0", "1.10.0", &valid));
        QVERIFY(valid);
        QVERIFY(!UpdateService::newerStable("v1.0.0", "1.0.0", &valid));
        QVERIFY(UpdateService::newerStable("v1.0.0", "1.0.0-dev.12", &valid));
        for (const auto& bad :
             {"v1.0.0-alpha.1", "v01.0.0", "v1.2", "v999999999999.0.0", "v1.0.0/other", "1.0.0"}) {
            QVERIFY(!UpdateService::newerStable(bad, "1.0.0", &valid));
            QVERIFY(!valid);
        }
    }
    void invalidConsentStaysOffline() {
        QTemporaryDir directory;
        const auto file = directory.filePath("settings.ini");
        QSettings settings(file, QSettings::IniFormat);
        settings.setValue("updates/automatic", "invalid");
        settings.sync();
        FakeNetwork network;
        UpdateService service(&network, file, [] { return 1700000000; });
        QVERIFY(!service.automatic());
        service.checkAutomatic();
        QVERIFY(network.requests.isEmpty());
    }
    void explicitCheckAndRequestPolicy() {
        QTemporaryDir directory;
        FakeNetwork network;
        qint64 now = 1700000000;
        UpdateService service(&network, directory.filePath("settings.ini"), [&] { return now; });
        QVERIFY(!service.automatic());
        service.checkAutomatic();
        QVERIFY(network.requests.isEmpty());
        QSignalSpy found(&service, &UpdateService::updateFound);
        service.check();
        QVERIFY(service.busy());
        service.check(); // Never queue a second request.
        QTRY_VERIFY(!service.busy());
        QCOMPARE(network.requests.size(), 1);
        QVERIFY(service.available());
        QCOMPARE(service.latestVersion(), "999.0.0");
        QCOMPARE(found.size(), 1);
        const auto request = network.requests.first();
        QCOMPARE(request.url(),
                 QUrl("https://api.github.com/repos/AfshinKI/openpaint/releases/latest"));
        QVERIFY(!request.hasRawHeader("Authorization"));
        QVERIFY(!request.hasRawHeader("Cookie"));
        QCOMPARE(request.rawHeader("User-Agent"), "OpenPaint-release-check");
        QCOMPARE(request.attribute(QNetworkRequest::RedirectPolicyAttribute).toInt(),
                 int(QNetworkRequest::ManualRedirectPolicy));
        service.check();
        QCOMPARE(network.requests.size(), 1);
        now += 60;
        service.check();
        QTRY_VERIFY(!service.busy());
        QCOMPARE(network.requests.size(), 2);
        QCOMPARE(found.size(), 1); // Same release does not repeatedly interrupt.
    }
    void rejectedResponses_data() {
        QTest::addColumn<QByteArray>("body");
        QTest::addColumn<int>("status");
        QTest::addColumn<int>("error");
        QTest::newRow("malformed") << QByteArray("{") << 200 << 0;
        QTest::newRow("oversized") << QByteArray(256 * 1024 + 1, 'x') << 200 << 0;
        QTest::newRow("draft") << QByteArray(
                                      R"({"tag_name":"v999.0.0","draft":true,"prerelease":false})")
                               << 200 << 0;
        QTest::newRow("preview")
            << QByteArray(R"({"tag_name":"v999.0.0","draft":false,"prerelease":true})") << 200 << 0;
        QTest::newRow("missing-fields") << QByteArray(R"({"tag_name":"v999.0.0"})") << 200 << 0;
        QTest::newRow("wrong-type")
            << QByteArray(R"({"tag_name":"v999.0.0","draft":"false","prerelease":false})") << 200
            << 0;
        QTest::newRow("redirect") << QByteArray("{}") << 302 << 0;
        QTest::newRow("not-found") << QByteArray("{}") << 404 << 0;
        QTest::newRow("offline") << QByteArray() << 0 << int(QNetworkReply::HostNotFoundError);
        QTest::newRow("tls") << QByteArray() << 0 << int(QNetworkReply::SslHandshakeFailedError);
    }
    void rejectedResponses() {
        QFETCH(QByteArray, body);
        QFETCH(int, status);
        QFETCH(int, error);
        QTemporaryDir directory;
        FakeNetwork network;
        network.next = {body, status, QNetworkReply::NetworkError(error), {}, true};
        UpdateService service(&network, directory.filePath("settings.ini"),
                              [] { return 1700000000; });
        service.check();
        QTRY_VERIFY(!service.busy());
        QVERIFY(!service.available());
        QVERIFY(service.latestVersion().isEmpty());
        QVERIFY(!service.message().contains("No newer stable"));
        QCOMPARE(network.requests.size(), 1);
    }
    void rateLimitAndPersistentDailySchedule() {
        QTemporaryDir directory;
        FakeNetwork network;
        network.next.status = 429;
        network.next.body = QByteArray(256 * 1024 + 1, 'x');
        network.next.headers = {{"Retry-After", "7200"}};
        qint64 now = 1700000000;
        const auto file = directory.filePath("settings.ini");
        {
            UpdateService service(&network, file, [&] { return now; });
            service.setAutomatic(true);
            QTRY_COMPARE(network.requests.size(), 1);
            QTRY_VERIFY(!service.busy());
            QVERIFY(service.message().contains("limited"));
        }
        UpdateService restarted(&network, file, [&] { return now; });
        QVERIFY(restarted.automatic());
        now += 3600;
        restarted.check();
        restarted.checkAutomatic();
        QCOMPARE(network.requests.size(), 1);
        now += 3600;
        restarted.checkAutomatic(); // Still less than a day since the attempt.
        QCOMPARE(network.requests.size(), 1);
        now += 86400;
        network.next = {};
        restarted.checkAutomatic();
        QTRY_VERIFY(!restarted.busy());
        QCOMPARE(network.requests.size(), 2);
        QVERIFY(restarted.available());
        restarted.setAutomatic(false);
        now += 86400;
        restarted.checkAutomatic();
        QCOMPARE(network.requests.size(), 2);
    }
    void cancellationAndTimeout() {
        QTemporaryDir directory;
        FakeNetwork network;
        network.next.finish = false;
        qint64 now = 1700000000;
        UpdateService service(&network, directory.filePath("settings.ini"), [&] { return now; });
        service.check();
        QVERIFY(service.busy());
        auto* cancelledReply = network.findChild<QNetworkReply*>();
        QVERIFY(cancelledReply);
        service.cancel();
        QVERIFY(QMetaObject::invokeMethod(cancelledReply, "finished"));
        QVERIFY(!service.busy());
        QVERIFY(!service.available());
        QVERIFY(service.message().contains("cancelled"));
        QCoreApplication::processEvents();
        QVERIFY(!service.available());
        now += 60;
        service.check();
        QVERIFY(service.busy());
        QTRY_VERIFY_WITH_TIMEOUT(!service.busy(), 17000);
        QVERIFY(service.message().contains("timed out"));
        QVERIFY(!service.available());
    }
};
QTEST_GUILESS_MAIN(UpdatesTest)
#include "tst_updates.moc"
