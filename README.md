# JACK Settings

[![Travis Build Status](https://img.shields.io/travis/com/azdrums/JACKSettings.svg?label=Linux&style=popout&logo=travis)](https://travis-ci.com/azdrums/JACKSettings)

***Work In Progress***

![](screenshot.png)

I wanted to have a JACK server starting as a user [systemd service] at boot time
instead of using `jackdbus` or starting it from an external application like
[Cadence] or [QjackCtl], so I created this similar little application to satisfy
my personal needs.
If you like the idea and find it useful let me know.

This application uses the [CLI11 command line parser].

## Installation

- copy `systemd/a2jmidid@.service` into `/usr/lib/systemd/user` (same folder
	where `jack@.service` is installed)

[Cadence]: https://kx.studio/Applications:Cadence
[QjackCtl]: https://github.com/rncbc/qjackctl
[systemd service]: https://github.com/jackaudio/jack2/tree/master/systemd
[CLI11 command line parser]: https://github.com/CLIUtils/CLI11
