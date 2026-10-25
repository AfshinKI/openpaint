// SPDX-License-Identifier: GPL-3.0-or-later
#include "UpdateService.h"
#include "BuildConfig.h"
#include "platform/AppInfo.h"
#include <QDateTime>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QVersionNumber>
#include <algorithm>

namespace {
constexpr qsizetype MaxResponse = 256 * 1024;
constexpr qint64 Day = 24 * 60 * 60;
const QUrl
    Endpoint(QStringLiteral("https://api.github.com/repos/AfshinKI/openpaint/releases/latest"));
} // namespace
UpdateService::UpdateService(QObject* parent)
    : UpdateService(nullptr, {}, [] { return QDateTime::currentSecsSinceEpoch(); }, parent) {}
UpdateService::UpdateService(QNetworkAccessManager* network, const QString& file,
                             std::function<qint64()> clock, QObject* parent)
    : QObject(parent), network_(network ? network : new QNetworkAccessManager(this)),
      clock_(std::move(clock)) {
    settings_ = file.isEmpty()
                    ? std::make_unique<QSettings>(QSettings::IniFormat, QSettings::UserScope,
                                                  "OpenPaint", "OpenPaint")
                    : std::make_unique<QSettings>(file, QSettings::IniFormat);
    skippedVersion_ = settings_->value("updates/skippedVersion").toString();
    automatic_ = supported();
    lastAttempt_ = settings_->value("updates/lastAttempt", 0).toLongLong();
    retryAt_ = settings_->value("updates/retryAt", 0).toLongLong();
    if (lastAttempt_ < 0 || lastAttempt_ > clock_() + 365 * Day)
        lastAttempt_ = 0;
    if (retryAt_ < 0 || retryAt_ > clock_() + 365 * Day)
        retryAt_ = 0;
    message_ =
        supported()
            ? tr("No release check has been made in this session.")
            : tr("This Flatpak build has no network permission. View releases in your browser.");
    timer_.setInterval(60 * 60 * 1000);
    connect(&timer_, &QTimer::timeout, this, &UpdateService::checkAutomatic);
    timeout_.setSingleShot(true);
    timeout_.setInterval(15000);
    connect(&timeout_, &QTimer::timeout, this, [this] {
        if (reply_) {
            timedOut_ = true;
            reply_->abort();
        }
    });
    if (automatic_)
        timer_.start();
    QTimer::singleShot(1500, this, [this] {
        // Check once per launch, retaining persisted cooldown and server backoff.
        if (automatic_ && !busy() && clock_() >= retryAt_)
            request();
    });
}
bool UpdateService::supported() const {
    return QStringLiteral(OPENPAINT_INSTALL_KIND) != "flatpak";
}
QString UpdateService::summary() const {
    if (!supported())
        return tr("Latest releases");
    if (busy())
        return tr("Checking for updates…");
    if (skipped())
        return tr("This version is skipped");
    if (available_)
        return tr("An update is ready");
    if (verified_)
        return tr("You’re up to date");
    return attempted_ ? tr("Couldn’t check for updates") : tr("Waiting to check");
}
void UpdateService::setMessage(const QString& message) {
    message_ = message;
    emit changed();
}
void UpdateService::setAutomatic(bool enabled) {
    enabled = enabled && supported();
    if (enabled == automatic_)
        return;
    automatic_ = enabled;
    if (enabled) {
        timer_.start();
        QTimer::singleShot(0, this, &UpdateService::checkAutomatic);
    } else {
        timer_.stop();
        cancel();
    }
    emit changed();
}
void UpdateService::checkAutomatic() {
    const auto now = clock_();
    if (automatic_ && !busy() && now >= retryAt_ &&
        (!lastAttempt_ || (now >= lastAttempt_ && now - lastAttempt_ >= Day)))
        request();
}
void UpdateService::refresh() {
    // Opening the panel preserves recent results and respects server backoff.
    if (automatic_ && !busy() && clock_() >= retryAt_)
        request();
}
void UpdateService::check() {
    if (!supported() || busy())
        return;
    if (clock_() < retryAt_) {
        setMessage(tr("Please wait before checking again; GitHub request cooldown is active."));
        return;
    }
    request();
}
void UpdateService::request() {
    attempted_ = true;
    verified_ = false;
    lastAttempt_ = clock_();
    retryAt_ = lastAttempt_ + 60;
    settings_->setValue("updates/lastAttempt", lastAttempt_);
    settings_->setValue("updates/retryAt", retryAt_);
    settings_->sync();
    body_.clear();
    tooLarge_ = timedOut_ = false;
    QNetworkRequest request(Endpoint);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");
    request.setRawHeader("User-Agent", "OpenPaint-release-check");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         QNetworkRequest::ManualRedirectPolicy);
    request.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);
    request.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);
    request.setAttribute(QNetworkRequest::AuthenticationReuseAttribute, QNetworkRequest::Manual);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                         QNetworkRequest::AlwaysNetwork);
    request.setTransferTimeout(15000);
    reply_ = network_->get(request);
    reply_->setReadBufferSize(MaxResponse + 1);
    connect(reply_, &QNetworkReply::readyRead, this, [this] {
        if (!reply_)
            return;
        body_ += reply_->read(MaxResponse + 1 - body_.size());
        if (body_.size() > MaxResponse) {
            tooLarge_ = true;
            reply_->abort();
        }
    });
    connect(reply_, &QNetworkReply::finished, this, &UpdateService::finish);
    timeout_.start();
    setMessage(tr("Checking GitHub for a newer stable release…"));
}
void UpdateService::cancel() {
    if (!reply_)
        return;
    auto* reply = reply_.data();
    reply_ = nullptr;
    timeout_.stop();
    disconnect(reply, nullptr, this, nullptr);
    reply->abort();
    reply->deleteLater();
    body_.clear();
    setMessage(tr("Release check cancelled."));
}
bool UpdateService::newerStable(const QString& tag, const QString& installed, bool* valid) {
    static const QRegularExpression release(
        "^v(0|[1-9][0-9]{0,8})\\.(0|[1-9][0-9]{0,8})\\.(0|[1-9][0-9]{0,8})$");
    static const QRegularExpression local(
        "^(0|[1-9][0-9]{0,8})\\.(0|[1-9][0-9]{0,8})\\.(0|[1-9][0-9]{0,8})(-[0-9A-Za-z.-]+)?$");
    const auto r = release.match(tag), l = local.match(installed);
    *valid = r.hasMatch() && l.hasMatch();
    if (!*valid)
        return false;
    const QVersionNumber remote(r.captured(1).toInt(), r.captured(2).toInt(),
                                r.captured(3).toInt());
    const QVersionNumber current(l.captured(1).toInt(), l.captured(2).toInt(),
                                 l.captured(3).toInt());
    return remote > current || (remote == current && !l.captured(4).isEmpty());
}
void UpdateService::finish() {
    if (!reply_)
        return;
    timeout_.stop();
    auto* reply = reply_.data();
    reply_ = nullptr;
    body_ += reply->read(MaxResponse + 1 - body_.size());
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const auto error = reply->error();
    reply->deleteLater();
    if (status == 403 || status == 429) {
        const auto now = clock_();
        bool ok = false;
        const auto seconds = reply->rawHeader("Retry-After").toLongLong(&ok);
        const auto date = QDateTime::fromString(
            QString::fromLatin1(reply->rawHeader("Retry-After")), Qt::RFC2822Date);
        retryAt_ = now + 3600;
        if (ok && seconds > 0 && seconds <= 365 * Day)
            retryAt_ = std::max(retryAt_, now + seconds);
        else if (date.isValid())
            retryAt_ = std::max(retryAt_, date.toSecsSinceEpoch());
        const auto reset = reply->rawHeader("X-RateLimit-Reset").toLongLong(&ok);
        if (ok && reset > now && reset - now <= 365 * Day)
            retryAt_ = std::max(retryAt_, reset);
        settings_->setValue("updates/retryAt", retryAt_);
        settings_->sync();
        setMessage(tr("GitHub limited this request. Try again later; automatic checks will wait."));
        return;
    }
    if (tooLarge_ || body_.size() > MaxResponse) {
        setMessage(tr("GitHub returned excessive release metadata. No update was accepted."));
        return;
    }
    if (timedOut_ || error != QNetworkReply::NoError) {
        setMessage(timedOut_ ? tr("Release check timed out. Your artwork is unchanged.")
                             : tr("Could not reach GitHub securely. Check your connection and try "
                                  "again later."));
        return;
    }
    if (status != 200) {
        setMessage(tr("GitHub did not return release information. No update was accepted."));
        return;
    }
    QJsonParseError parseError;
    const auto json = QJsonDocument::fromJson(body_, &parseError);
    const auto object = json.object();
    bool valid = false;
    const auto tag = object.value("tag_name").toString();
    const bool newer = newerStable(tag, AppInfo::version(), &valid);
    if (parseError.error != QJsonParseError::NoError || !json.isObject() || !valid ||
        !object.value("draft").isBool() || object.value("draft").toBool() ||
        !object.value("prerelease").isBool() || object.value("prerelease").toBool()) {
        setMessage(tr("GitHub returned invalid stable-release metadata. No update was accepted."));
        return;
    }
    const bool notify = newer && (!available_ || latestVersion_ != tag.mid(1));
    verified_ = true;
    latestVersion_ = tag.mid(1);
    available_ = newer;
    setMessage(newer ? tr("OpenPaint %1 is available.").arg(latestVersion_)
                     : tr("No newer stable release is available on GitHub."));
    if (notify && !skipped())
        emit updateFound();
}
bool UpdateService::skipVersion() {
    if (!available_ || busy() || latestVersion_.isEmpty())
        return false;
    settings_->setValue("updates/skippedVersion", latestVersion_);
    settings_->sync();
    if (settings_->status() != QSettings::NoError) {
        settings_->setValue("updates/skippedVersion", skippedVersion_);
        setMessage(tr("Could not save the skipped version. Please try again."));
        return false;
    }
    skippedVersion_ = latestVersion_;
    setMessage(
        tr("Version %1 is skipped. You’ll be notified about newer releases.").arg(latestVersion_));
    return true;
}
bool UpdateService::openRelease() const {
    if (!available_ || latestVersion_.isEmpty())
        return false;
    return QDesktopServices::openUrl(
        QUrl("https://github.com/AfshinKI/openpaint/releases/tag/v" + latestVersion_));
}
