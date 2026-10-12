# Security

OpenPaint has no application releases yet. No released version is currently
supported; the support window will be documented with the first release.

Do not attach credentials, private artwork, personal file paths, or exploit files
containing personal information to public issues.

Use GitHub's **Report a vulnerability** option on this repository's Security tab
when it is enabled. Private reporting still needs to be enabled by a maintainer;
this file does not enable it. If the option is absent, open an issue requesting a
private reporting channel without disclosing vulnerability details.

Relevant attack surfaces include image/project decoding, archive extraction,
clipboard data, optional model files, and release/update verification. Reports
should include the affected version, a minimal synthetic reproducer, impact,
and reproduction steps. Do not test against other people's systems.

The [release plan](docs/RELEASES.md) specifies signing, dependency review, and
support expectations before the first stable release.
