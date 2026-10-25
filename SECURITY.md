# Security

OpenPaint v2.0.0 is the current stable release. Under the
[release support policy](docs/RELEASES.md), the latest stable minor line (currently
2.0.x) receives fixes; older lines have no promised backports and prereleases have
no stability promise. Report the exact affected version. Platform validation
limits are recorded in the [v2 release evidence](docs/plans/0062-macos-v2-release.md).

Do not attach credentials, private artwork, personal file paths, or exploit files
containing personal information to public issues.

Use GitHub's **Report a vulnerability** option on this repository's Security tab
when it is enabled. Its availability is a repository setting managed by the
maintainer; this file does not enable it. If the option is absent, open an issue requesting a
private reporting channel without disclosing vulnerability details.

Relevant attack surfaces include image/project decoding, archive extraction,
clipboard data, and release checks/package verification. AI/model loading and
automatic update installation are future surfaces, not shipped functionality. Reports
should include the affected version, a minimal synthetic reproducer, impact,
and reproduction steps. Do not test against other people's systems.

The [release plan](docs/RELEASES.md) specifies signing, dependency review, and
support expectations. Current Linux downloads are unsigned; macOS is ad-hoc
signed without Developer ID or notarization. Checksums detect changed bytes but
do not authenticate a download. Signed automatic updates remain future work.
