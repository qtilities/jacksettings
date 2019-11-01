# JACK Settings

[![Travis Build Status](https://img.shields.io/travis/com/azdrums/JACKSettings.svg?label=Linux&style=popout&logo=travis)](https://travis-ci.com/azdrums/JACKSettings)

***This is alpha-stage software and not fully functional yet.***

![](screenshot.png)

This is a simplified JACK settings application mimic of [Cadence],
but using a user [systemd service].

This application uses the [CLI11 command line parser].

## Installation

- copy `systemd/a2jmidid@.service` into `/usr/lib/systemd/user` (same folder
	where `jack@.service` is installed)

[Cadence]: https://kx.studio/Applications:Cadence
[systemd service]: https://github.com/jackaudio/jack2/tree/master/systemd
[CLI11 command line parser]: https://github.com/CLIUtils/CLI11
