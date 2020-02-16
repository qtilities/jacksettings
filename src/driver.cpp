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
#include "src/driver.h"
#include "src/device.h"

#include <CLI/CLI.hpp>

#include <alsa/asoundlib.h>

#include <string.h>
#include <cstdio>
#include <vector>

namespace jack
{
	const QString Driver::DefaultDeviceName  = "hw:0";
	const int Driver::DefaultSampleRate      = 48000;
	const int Driver::DefaultChannelCountIn  = 0;
	const int Driver::DefaultChannelCountOut = 0;
	const int Driver::DefaultPeriod          = 1024;
	const int Driver::DefaultPeriodCount     = 2;
	const int Driver::DefaultLatencyIn       = 0;
	const int Driver::DefaultLatencyOut      = 0;
	const int Driver::DefaultWaitTime        = 0;

Driver::Driver()
:
#ifdef Q_OS_LINUX
type_(Type::ALSA),
#elif defined(Q_OS_MACOS)
type_(Type::CoreAudio),
#elif defined(Q_OS_WIN)
type_(Type::PortAudio),
#else
type_(Type::Unknown),
#endif
ditherMode_(DitherMode::NoDither),
alsaMidiType_(AlsaMidiType::NoMidi),
deviceName_(DefaultDeviceName),
inputDeviceName_(),
outputDeviceName_(),
inputChannelCount_(DefaultChannelCountIn),
outputChannelCount_(DefaultChannelCountOut),
sampleRate_(DefaultSampleRate),
period_(DefaultPeriod),
periodCount_(DefaultPeriodCount),
inputLatency_(DefaultLatencyIn),
outputLatency_(DefaultLatencyOut),
waitTime_(DefaultWaitTime),
isDuplex_(true),
hasMonitor_(false),
hasMonitorHw_(false),
hasMeterHw_(false),
isSoftMode_(false),
use16bit_(false)
{
	enumerateDevices();
}
Driver::Type Driver::type() const
{
	return type_;
}
Driver::DitherMode Driver::ditherMode() const
{
	return ditherMode_;
}
Driver::AlsaMidiType Driver::alsaMidiType() const
{
	return alsaMidiType_;
}
QString Driver::name() const
{
	if      (type_== Type::ALSA)      return "alsa";
	else if (type_== Type::CoreAudio) return "coreaudio";
	else if (type_== Type::PortAudio) return "portaudio";
	else if (type_== Type::Dummy)     return "dummy";
	else if (type_== Type::FireWire)  return "firewire";
	else if (type_== Type::LoopBack)  return "loopback";
	else if (type_== Type::Net)       return "net";
	else if (type_== Type::NetOne)    return "netone";
	else return "";
}
QString Driver::deviceName() const
{
	return deviceName_;
}
QString Driver::deviceInputName() const
{
	return inputDeviceName_;
}
QString Driver::deviceOutputName() const
{
	return outputDeviceName_;
}
QString Driver::ditherModeAsString() const
{
	if      (ditherMode_== DitherMode::Rectangular) return "r";
	else if (ditherMode_== DitherMode::Triangular)  return "t";
	else if (ditherMode_== DitherMode::Shaped)      return "s";
	else return "";
}
QString Driver::alsaMidiTypeAsString()
{
	if      (alsaMidiType_== AlsaMidiType::Sequencer) return "seq";
	else if (alsaMidiType_== AlsaMidiType::Raw)       return "raw";
	else return "";
}
/*static*/ Driver::Type typeFromString(const QString &name)
{
	if      (name == "alsa")      return Driver::Type::ALSA;
	else if (name == "coreaudio") return Driver::Type::CoreAudio;
	else if (name == "portaudio") return Driver::Type::PortAudio;
	else if (name == "dummy")     return Driver::Type::Dummy;
	else if (name == "firewire")  return Driver::Type::FireWire;
	else if (name == "loopback")  return Driver::Type::LoopBack;
	else if (name == "net")       return Driver::Type::Net;
	else if (name == "netone")    return Driver::Type::NetOne;
	else return Driver::Type::Unknown;
}
/*static*/ Driver::AlsaMidiType midiTypeFromString(const QString &name)
{
	if      (name == "seq") return Driver::AlsaMidiType::Sequencer;
	else if (name == "raw") return Driver::AlsaMidiType::Raw;
	else return Driver::AlsaMidiType::NoMidi;
}
/*static*/ Driver::DitherMode ditherModeFromString(const QString &name)
{
	if      (name == "r") return Driver::DitherMode::Rectangular;
	else if (name == "t") return Driver::DitherMode::Triangular;
	else if (name == "s") return Driver::DitherMode::Shaped;
	else return Driver::DitherMode::NoDither;
}
int Driver::inputChannelCount() const
{
	return inputChannelCount_;
}
int Driver::outputChannelCount() const
{
	return outputChannelCount_;
}
int Driver::sampleRate() const
{
	return sampleRate_;
}
int Driver::period() const
{
	return period_;
}
int Driver::periodCount() const
{
	return periodCount_;
}
int Driver::inputLatency() const
{
	return inputLatency_;
}
int Driver::outputLatency() const
{
	return outputLatency_;
}
int Driver::waitTime() const
{
	return waitTime_;
}
bool Driver::isDuplex() const
{
	return isDuplex_;
}
bool Driver::hasMonitor() const
{
	return hasMonitor_;
}
bool Driver::hasMonitorHw() const
{
	return hasMonitorHw_;
}
bool Driver::hasMeterHw() const
{
	return hasMeterHw_;
}
bool Driver::isSoftMode() const
{
	return isSoftMode_;
}
bool Driver::use16bit() const
{
	return use16bit_;
}
void Driver::setType(Driver::Type type)
{
	type_= type;
}
void Driver::setDitherMode(Driver::DitherMode mode)
{
	ditherMode_= mode;
}
void Driver::setAlsaMidiType(AlsaMidiType type)
{
	alsaMidiType_= type;
}
void Driver::setDeviceName(const QString &name)
{
	deviceName_= name;
}
void Driver::setDeviceInputName(const QString &name)
{
	inputDeviceName_= name;
}
void Driver::setDeviceOutputName(const QString &name)
{
	outputDeviceName_= name;
}
void Driver::setInputChannelCount(int count)
{
	inputChannelCount_= count;
}
void Driver::setOutputChannelCount(int count)
{
	outputChannelCount_= count;
}
void Driver::setSampleRate(int rate)
{
	sampleRate_= rate;
}
void Driver::setPeriod(int period)
{
	period_= period;
}
void Driver::setPeriodCount(int count)
{
	periodCount_= count;
}
void Driver::setInputLatency(int count)
{
	inputLatency_= count;
}
void Driver::setOutputLatency(int count)
{
	outputLatency_= count;
}
void Driver::setWaitTime(int count)
{
	waitTime_= count;
}
void Driver::setIsDuplex(bool isDuplex)
{
	isDuplex_= isDuplex;
}
void Driver::setHasMonitor(bool hasMonitor)
{
	hasMonitor_= hasMonitor;
}
void Driver::setHasMonitorHw(bool hasMonitorHw)
{
	hasMonitorHw_= hasMonitorHw;
}
void Driver::setHasMeterHw(bool hasMeterHw)
{
	hasMeterHw_= hasMeterHw;
}
void Driver::setIsSoftMode(bool isSoftMode)
{
	isSoftMode_= isSoftMode;
}
void Driver::setUse16bit(bool use16bit)
{
	use16bit_= use16bit;
}
void Driver::parseArgsCLI(const std::string &command)
{
	CLI::App app{"Audio driver CLI parser"};

	std::string  alsaMidiTypeName;
	std::string  ditherMode;

	app.add_option("-z,--dither",         ditherMode,          "");
	app.add_option("-X,--midi-driver",    alsaMidiTypeName,    "");
	app.add_option("-C,--capture",        inputDeviceName_,    "");
	app.add_option("-P,--playback",       outputDeviceName_,   "");
	app.add_option("-i,--inchannels",     inputChannelCount_,  "");
	app.add_option("-o,--outchannels",    outputChannelCount_, "");
	app.add_option("-r,--rate",           sampleRate_,         "");
	app.add_option("-p,--period",         period_,             "");
	app.add_option("-n,--nperiods",       periodCount_,        "");
	app.add_option("-I,--input-latency",  inputLatency_,       "");
	app.add_option("-O,--output-latency", outputLatency_,      "");
	app.add_option("-w,--wait",           waitTime_,           "");
	app.add_flag("-D,--duplex",           isDuplex_,           "");
	app.add_flag("-m,--monitor",          hasMonitor_,         "");
	app.add_flag("-H,--hwmon",            hasMonitorHw_,       "");
	app.add_flag("-M,--hwmeter",          hasMeterHw_,         "");
	app.add_flag("-s,--softmode",         isSoftMode_,         "");
	app.add_flag("-S,--shorts",           use16bit_,            "");

	try
	{
		app.parse(command);
	}
	catch (const CLI::ParseError &e)
	{
		std::cerr << "An exception occurred: " << e.what() << '\n';
	}
	ditherMode_  = ditherModeFromString(QString::fromStdString(ditherMode));
	alsaMidiType_= midiTypeFromString(QString::fromStdString(alsaMidiTypeName));
}

void Driver::enumerateDevices()
{
#ifdef Q_OS_LINUX
	int cardIndex = -1,
			deviceIndex = -1,
			errorNumber;

	snd_ctl_t           *handle;
	snd_pcm_info_t      *pcmInfo;
	snd_pcm_hw_params_t *hw_params;
	snd_ctl_card_info_t *cardInfo;
	snd_ctl_card_info_alloca(&cardInfo);
	snd_pcm_info_alloca(&pcmInfo);
	snd_pcm_hw_params_alloca(&hw_params);

	if (snd_card_next(&cardIndex) < 0 || cardIndex < 0)
	{
		std::cerr << "No soundcards found.\n";
		return;
	}
	while (cardIndex >= 0)
	{
		QString cardName(QString("hw:%1").arg(cardIndex));
		if ((errorNumber = snd_ctl_open(&handle, cardName.toStdString().c_str(), 0)) < 0)
		{
			std::cerr << "Control open card" << cardIndex << "): " <<
				snd_strerror(errorNumber) << '\n';
			goto nextSoundCard;
		}
		if ((errorNumber = snd_ctl_card_info(handle, cardInfo)) < 0)
		{
			std::cerr << "Control hardware card" << cardIndex << " info: " <<
				snd_strerror(errorNumber) << '\n';
			snd_ctl_close(handle);
			goto nextSoundCard;
		}
		while (true)
		{
			if (snd_ctl_pcm_next_device(handle, &deviceIndex) < 0)
				std::cerr << "No snd_ctl_pcm_next_device\n";

			if (deviceIndex < 0)
				break;

			snd_pcm_info_set_device(pcmInfo, deviceIndex);
			snd_pcm_info_set_subdevice(pcmInfo, 0);
			snd_pcm_info_set_stream(pcmInfo, SND_PCM_STREAM_PLAYBACK);

			if ((errorNumber = snd_ctl_pcm_info(handle, pcmInfo)) < 0)
			{
				if (errorNumber != -ENOENT)
					continue;
			}
			const char *cardId      = snd_ctl_card_info_get_id(cardInfo);
			const char *cardName    = snd_ctl_card_info_get_name(cardInfo);
			const char *cardPcmId   = snd_pcm_info_get_id(pcmInfo);
			const char *cardPcmName = snd_pcm_info_get_name(pcmInfo);

			std::cerr << "Card " << cardIndex << ": " << cardId << " " <<
				cardName << " device " << deviceIndex <<	": " << cardPcmId << " [" <<
				cardPcmName << "]\n";

			DevicePtr device(new Device(cardIndex, deviceIndex, cardId, cardName, cardPcmId));
			devices.push_back(device);
		}
		snd_ctl_close(handle);

	nextSoundCard:
		if (snd_card_next(&cardIndex) < 0)
			break;
	}
#endif // #ifdef Q_OS_LINUX
}
} // namespace jack
