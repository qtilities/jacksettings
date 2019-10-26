# JACK Settings

*** This is alpha-stage software and not fully functional yet. ***

![](screenshot.png)

This is a simplified JACK settings mimic of [Cadence],
but using a user [systemd service].

This application uses the [CLI11 command line parser].

## Installation

- copy `systemd/a2jmidid@.service` into `/usr/lib/systemd/user` (same folder
	where `jack@.service` is installed)

[Cadence]: https://kx.studio/Applications:Cadence
[systemd service]: https://github.com/jackaudio/jack2/tree/master/systemd
[CLI command line parser]: https://github.com/CLIUtils/CLI11
