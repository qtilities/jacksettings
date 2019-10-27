/*
	Copyright (C) 2019 Andrea Zanellato <redtid3@gmail.com>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	For a full copy of the GNU General Public License see the LICENSE file
*/
#include "settings.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>

#include <string>
#include <iostream>

#include <CLI/CLI.hpp>

#include <getopt.h>

#include "src/device.h"

namespace jack
{
	const int Settings::DefaultMaxPort    = 128;
	const int Settings::DefaultRTPriority = 10;
	const int Settings::DefaultTimeOut    = 0;

Settings::Settings()
:
	driver(new Driver()),
	curProfileName("default"),
	name_("default"),
	clockSrc_(ClockSource::System),
	connMode_(AutoConnectMode::NoRestrict),
	isRealTime_(true),
//isReplaceReg_(false),
	isSync_(false),
	isTemporary_(false),
	isVerbose_(false),
	portMax_(DefaultMaxPort),
	rtPriority_(DefaultRTPriority),
	timeOut_(DefaultTimeOut)
{
}
bool Settings::load(const QString &name)
{
	QDir configDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
	QString filePath(configDir.filePath("jack/" + name + ".conf"));
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QTextStream in(&file);
	QString jackOptions, driver_, deviceName_, driverSettings;

	while (!in.atEnd())
	{
		QString line = in.readLine().trimmed().replace("\"", "");
		QStringList keyValuePair(line.split('='));

		if (keyValuePair.size() < 2)
			continue;

		if (keyValuePair.at(0) == "JACK_DEFAULT_SERVER")
		{
			name_= keyValuePair.at(1);
		}
		else if (keyValuePair.at(0) == "JACK_OPTIONS")
		{
			jackOptions = keyValuePair.at(1);
			parseArgsCLI(jackOptions.toStdString());
		}
		else if (keyValuePair.at(0) == "DRIVER")
		{
			driver_= keyValuePair.at(1);
			driver->setType(Driver::typeFromString(driver_));
		}
		else if (keyValuePair.at(0) == "DEVICE")
		{
			deviceName_= keyValuePair.at(1);
			driver->setDeviceName(deviceName_);
		}
		else if (keyValuePair.at(0) == "DRIVER_SETTINGS")
		{
			driverSettings = keyValuePair.at(1);
			driver->parseArgsCLI(driverSettings.toStdString());
		}
	}
	curProfileName = name;
	return true;
}
bool Settings::save()
{
	QDir    configDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
	QString fileName;

	if (curProfileName != "")
		fileName = curProfileName;
	else
		fileName = "default";

	QString filePath(configDir.filePath("jack/" + fileName + ".conf"));
	QFile   file(filePath);

	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	QTextStream out(&file);
	QString jackOptions, driverSettings;

	if (clockSrc_== ClockSource::HPET) jackOptions.append("-c h");

	if (connMode_ != AutoConnectMode::NoRestrict)
		jackOptions.append(" -a " + autoConnectModeToString(connMode_));

	if (!isRealTime_) jackOptions.append(" -r");
	if (isSync_)      jackOptions.append(" -S");
	if (isTemporary_) jackOptions.append(" -T");
	if (isVerbose_)   jackOptions.append(" -v");

	if (timeOut_!= DefaultTimeOut)
		jackOptions.append(" -t " + QString::number(timeOut_));

	if (portMax_!= DefaultMaxPort)
		jackOptions.append(" -p " + QString::number(portMax_));

	if (rtPriority_!= DefaultRTPriority)
		jackOptions.append(" -P " + QString::number(rtPriority_));

	if (driver->ditherMode() != Driver::DitherMode::NoDither)
		driverSettings.append(" -z " + driver->ditherModeAsString());

	if (driver->alsaMidiType() != Driver::AlsaMidiType::NoMidi)
		driverSettings.append(" -X " + driver->alsaMidiTypeAsString());

	if (driver->deviceInputName() != "")
		driverSettings.append(" -C " + driver->deviceInputName());

	if (driver->deviceOutputName() != "")
		driverSettings.append(" -P " + driver->deviceOutputName());

	if (driver->inputChannelCount())
		driverSettings.append(" -i " + QString::number(driver->inputChannelCount()));

	if (driver->outputChannelCount())
		driverSettings.append(" -o " + QString::number(driver->outputChannelCount()));

	if (driver->sampleRate() != Driver::DefaultSampleRate)
		driverSettings.append(" -r " + QString::number(driver->sampleRate()));

	if (driver->period() != Driver::DefaultPeriod)
		driverSettings.append(" -p " + QString::number(driver->period()));

	if (driver->periodCount() != Driver::DefaultPeriodCount)
		driverSettings.append(" -n " + QString::number(driver->periodCount()));

	if (driver->inputLatency() != Driver::DefaultLatencyIn)
		driverSettings.append(" -I " + QString::number(driver->inputLatency()));

	if (driver->outputLatency() != Driver::DefaultLatencyOut)
		driverSettings.append(" -O " + QString::number(driver->outputLatency()));

	if (driver->waitTime() != Driver::DefaultWaitTime)
		driverSettings.append(" -w " + QString::number(driver->waitTime()));

	if (driver->hasMonitor())
		driverSettings.append(" -m");

	if (driver->hasMonitorHw())
		driverSettings.append(" -H");

	if (driver->hasMeterHw())
		driverSettings.append(" -M");

	if (driver->isSoftMode())
		driverSettings.append(" -s");

	if (driver->use16bit())
		driverSettings.append(" -S");

	jackOptions = jackOptions.trimmed();
	driverSettings = driverSettings.trimmed();

	out << "# This file is also handled by JACKSettings,\n";
	out << "# so it could be overwritten externally.\n";
	out << "JACK_DEFAULT_SERVER=\"" << name_ << "\"\n";
	out << "JACK_OPTIONS=\""        << jackOptions << "\"\n";
	out << "DRIVER=\""              << driver->name() << "\"\n";
	out << "DEVICE=\""              << driver->deviceName() << "\"\n";
	out << "DRIVER_SETTINGS=\""     << driverSettings << "\"\n";

	return true;
}
void Settings::reset()
{
	driver.reset(new Driver());
	name_        = "default";
	clockSrc_    = ClockSource::System;
	connMode_    = AutoConnectMode::NoRestrict;
	isRealTime_  = true;
//isReplaceReg_= false;
	isSync_      = false;
	isTemporary_ = false;
	isVerbose_   = false;
	timeOut_     = DefaultTimeOut;
	portMax_     = DefaultMaxPort;
	rtPriority_  = DefaultRTPriority;

	save();
}
QString Settings::name() const
{
	return name_;
}
QString Settings::profileName() const
{
	return curProfileName;
}
Settings::ClockSource Settings::clockSource() const
{
	return clockSrc_;
}
Settings::AutoConnectMode Settings::autoConnectMode() const
{
	return connMode_;
}
bool Settings::isRealtime() const
{
	return isRealTime_;
}/*
bool Settings::isReplaceRegistry() const
{
	return isReplaceReg_;
}*/
bool Settings::isSync() const
{
	return isSync_;
}
bool Settings::isTemporary() const
{
	return isTemporary_;
}
bool Settings::isVerbose() const
{
	return isVerbose_;
}
int Settings::clientTimeOut() const
{
	return timeOut_;
}
int Settings::portMax() const
{
	return portMax_;
}
int Settings::realtimePriority() const
{
	return rtPriority_;
}
int Settings::deviceCount() const
{
	return driver->devices.size();
}
QString Settings::deviceCardIdAt(int index)
{
	if (index >= driver->devices.size())
		return QString();

	return driver->devices.at(index)->cardId();
}
QVector<uint> Settings::supportedSampleRatesAt(int index)
{
	if (index >= driver->devices.size())
		return QVector<uint>();

	return driver->devices.at(index)->supportedSampleRates();
}
QString Settings::deviceCardNameAt(int index)
{
	if (index >= driver->devices.size())
		return QString();

	return driver->devices.at(index)->cardName();
}
void Settings::setClockSource(ClockSource source)
{
	clockSrc_= source;
}
void Settings::setAutoConnectMode(AutoConnectMode mode)
{
	connMode_= mode;
}
void Settings::setRealtime(bool realtime)
{
	isRealTime_= realtime;
}/*
void Settings::setReplaceRegistry(bool replace)
{
	isReplaceReg_= replace;
}*/
void Settings::setSync(bool sync)
{
	isSync_= sync;
}
void Settings::setTemporary(bool temporary)
{
	isTemporary_= temporary;
}
void Settings::setVerbose(bool verbose)
{
	isVerbose_= verbose;
}
void Settings::setClientTimeout(int timeout)
{
	timeOut_= timeout;
}
void Settings::setPortMax(int max)
{
	portMax_= max;
}
void Settings::setRealtimePriority(int priority)
{
	rtPriority_= priority;
}
void Settings::setDriverType(Driver::Type type)
{
	driver->setType(type);
}
void Settings::setDitherMode(Driver::DitherMode mode)
{
	driver->setDitherMode(mode);
}
void Settings::setAlsaMidiType(Driver::AlsaMidiType type)
{
	driver->setAlsaMidiType(type);
}
void Settings::setDeviceName(const QString &name)
{
	driver->setDeviceName(name);
}
void Settings::setDeviceInputName(const QString &name)
{
	driver->setDeviceInputName(name);
}
void Settings::setDeviceOutputName(const QString &name)
{
	driver->setDeviceOutputName(name);
}
void Settings::setInputChannelCount(int count)
{
	driver->setInputChannelCount(count);
}
void Settings::setOutputChannelCount(int count)
{
	driver->setOutputChannelCount(count);
}
void Settings::setSampleRate(int rate)
{
	driver->setSampleRate(rate);
}
void Settings::setPeriod(int period)
{
	driver->setPeriod(period);
}
void Settings::setPeriodCount(int count)
{
	driver->setPeriodCount(count);
}
void Settings::setInputLatency(int latency)
{
	driver->setInputLatency(latency);
}
void Settings::setOutputLatency(int latency)
{
	driver->setOutputLatency(latency);
}
void Settings::setWaitTime(int time)
{
	driver->setWaitTime(time);
}
void Settings::setIsDuplex(bool isDuplex)
{
	driver->setIsDuplex(isDuplex);
}
void Settings::setHasMonitor(bool hasMonitor)
{
	driver->setHasMonitor(hasMonitor);
}
void Settings::setHasMonitorHw(bool hasMonitorHw)
{
	driver->setHasMonitor(hasMonitorHw);
}
void Settings::setHasMeterHw(bool hasMeterHw)
{
	driver->setHasMeterHw(hasMeterHw);
}
void Settings::setIsSoftMode(bool isSoftMode)
{
	driver->setIsSoftMode(isSoftMode);
}
void Settings::setUse16bit(bool use16bit)
{
	driver->setUse16bit(use16bit);
}
void Settings::parseArgsCLI(const std::string &command)
{
	CLI::App app{"JACK engine CLI parser"};

	std::string clockSrc;
	std::string connMode;

	app.add_option("-c,--clock-source",       clockSrc,    "");
	app.add_option("-a,--autoconnect",        connMode,    "");
	app.add_option("-t,--timeout",            timeOut_,    "");
	app.add_option("-p,--port-max",           portMax_,    "");
	app.add_option("-P,--realtime-priority",  rtPriority_, "");
	app.add_flag("--realtime,!--no-realtime", isRealTime_, "");
	app.add_flag("-R,!-r",                    isRealTime_, "");
	app.add_flag("-S,--sync",                 isSync_,     "");
	app.add_flag("-T,--temporary",            isTemporary_,"");
	app.add_flag("-v,--verbose",              isVerbose_,  "");

	try
	{
		app.parse(command);
	}
	catch (const CLI::ParseError &e)
	{
		std::cerr << "An exception occurred: " << e.what() << '\n';
	}
	clockSrc_   = clockSourceFromString(QString::fromStdString(clockSrc));
	connMode_   = autoConnectModeFromString(QString::fromStdString(connMode));
}
} // namespace jack
