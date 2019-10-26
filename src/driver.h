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
#pragma once

#include <QString>
#include <QSharedPointer>
#include <QVector>

#include <string>

namespace jack
{
class Device;
class Driver
{
	friend class Settings;

public:
	static const QString DefaultDeviceName;
	static const int DefaultSampleRate;
	static const int DefaultChannelCountIn;
	static const int DefaultChannelCountOut;
	static const int DefaultPeriod;
	static const int DefaultPeriodCount;
	static const int DefaultLatencyIn;
	static const int DefaultLatencyOut;
	static const int DefaultWaitTime;

	enum Type // TODO
	{
		ALSA = 0,
		CoreAudio,
		PortAudio,
		Dummy,
		FireWire,
		LoopBack,
		Net,
		NetOne,
		Unknown
	};
	enum AlsaMidiType
	{
		NoMidi = 0,
		Sequencer,
		Raw
	};
	enum DitherMode
	{
		NoDither = 0,
		Rectangular,
		Shaped,
		Triangular
	};
	Driver();

	Type type() const;
	DitherMode ditherMode() const;
	AlsaMidiType alsaMidiType() const;
	QString name() const;
	QString deviceName() const;
	QString deviceInputName() const;
	QString deviceOutputName() const;
	QString ditherModeAsString() const;
	QString alsaMidiTypeAsString();
	int inputChannelCount() const;
	int outputChannelCount() const;
	int sampleRate() const;
	int period() const;
	int periodCount() const;
	int inputLatency() const;
	int outputLatency() const;
	int waitTime() const;
	bool isDuplex() const;
	bool hasMonitor() const;
	bool hasMonitorHw() const;
	bool hasMeterHw() const;
	bool isSoftMode() const;
	bool use16bit() const;

	int deviceCount() const;
	QString deviceCardIdAt(int index);
	QString deviceCardNameAt(int index);

	void setType(Type type);
	void setDitherMode(DitherMode mode);
	void setAlsaMidiType(AlsaMidiType type);
	void setDeviceName(const QString &name);
	void setDeviceInputName(const QString &name);
	void setDeviceOutputName(const QString &name);
	void setInputChannelCount(int count);
	void setOutputChannelCount(int count);
	void setSampleRate(int rate);
	void setPeriod(int period);
	void setPeriodCount(int count);
	void setInputLatency(int count);
	void setOutputLatency(int count);
	void setWaitTime(int count);
	void setIsDuplex(bool isDuplex);
	void setHasMonitor(bool hasMonitor);
	void setHasMonitorHw(bool hasMonitorHw);
	void setHasMeterHw(bool hasMeterHw);
	void setIsSoftMode(bool isSoftMode);
	void setUse16bit(bool use16bit);

	void parseArgsCLI(const std::string &command);

	static Type typeFromString(const QString &name)
	{
		if      (name == "alsa")      return Type::ALSA;
		else if (name == "coreaudio") return Type::CoreAudio;
		else if (name == "portaudio") return Type::PortAudio;
		else if (name == "dummy")     return Type::Dummy;
		else if (name == "firewire")  return Type::FireWire;
		else if (name == "loopback")  return Type::LoopBack;
		else if (name == "net")       return Type::Net;
		else if (name == "netone")    return Type::NetOne;
		else return Type::Unknown;
	}
	static AlsaMidiType midiTypeFromString(const QString &name)
	{
		if      (name == "seq") return AlsaMidiType::Sequencer;
		else if (name == "raw") return AlsaMidiType::Raw;
		else return AlsaMidiType::NoMidi;
	}
	static DitherMode ditherModeFromString(const QString &name)
	{
		if      (name == "r") return DitherMode::Rectangular;
		else if (name == "t") return DitherMode::Triangular;
		else if (name == "s") return DitherMode::Shaped;
		else return DitherMode::NoDither;
	}

private:
	void enumerateDevices();

	typedef QSharedPointer<Device> DevicePtr;
	QVector<DevicePtr> devices;

	Type         type_;
	DitherMode   ditherMode_;
	AlsaMidiType alsaMidiType_;
	QString      deviceName_;
	QString      inputDeviceName_;
	QString      outputDeviceName_;
	int          inputChannelCount_;
	int          outputChannelCount_;
	int          sampleRate_;
	int          period_;
	int          periodCount_;
	int          inputLatency_;
	int          outputLatency_;
	int          waitTime_;
	bool         isDuplex_;
	bool         hasMonitor_;
	bool         hasMonitorHw_;
	bool         hasMeterHw_;
	bool         isSoftMode_;
	bool         use16bit_;
};
} // namespace jack
