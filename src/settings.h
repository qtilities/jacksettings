/*
    Copyright (C) 2019 - 2020 Andrea Zanellato <redtid3@gmail.com>

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

#include "src/driver.h"

#include <QString>
#include <vector>
#include <memory>

namespace jack
{
class Settings
{
public:
    static const int DefaultMaxPort;
    static const int DefaultRTPriority;
    static const int DefaultTimeOut;

    enum AutoConnectMode // -a, --autoconnect <modechar>
    {
        NoRestrict = 0, // ' ' - Don't restrict self connect requests (default)
        Fail,           // 'E' - Fail self connect requests to external ports only
        Ignore,         // 'e' - Ignore self connect requests to external ports only
        FailAll,        // 'A' - Fail all self connect requests
        IgnoreAll       // 'a' - Ignore all self connect requests
    };
    enum ClockSource // -c, --clocksource ( h(pet) | s(ystem) )
    {
        System = 0,
        HPET
    };
    Settings();

    bool load(const QString &profileName);
    bool save();
    void reset();

    QString name() const;
    QString profileName() const;
    ClockSource clockSource() const;
    AutoConnectMode autoConnectMode() const;
    bool isRealtime() const;
//  bool isReplaceRegistry() const;
    bool isSync() const;
    bool isTemporary() const;
    bool isVerbose() const;
    int  clientTimeOut() const;
    int  portMax() const;
    int  realtimePriority() const;

    void setClockSource(ClockSource source);
    void setAutoConnectMode(AutoConnectMode mode);
    void setRealtime(bool realtime);
//  void setReplaceRegistry(bool replace);
    void setSync(bool sync);
    void setTemporary(bool temporary);
    void setVerbose(bool verbose);
    void setClientTimeout(int timeout);
    void setPortMax(int max);
    void setRealtimePriority(int priority);

    Driver::Type         driverType()       const {return driver->type();}
    Driver::DitherMode   ditherMode()       const {return driver->ditherMode();}
    Driver::AlsaMidiType alsaMidiType()     const {return driver->alsaMidiType();}
    QString              deviceName()       const {return driver->deviceName();}
    QString              deviceInputName()  const {return driver->deviceInputName();}
    QString              deviceOutputName() const {return driver->deviceOutputName();}
    int                  deviceCount()      const;
    QString              deviceCardIdAt(int index);
    QString              deviceCardNameAt(int index);
    QVector<uint>        supportedSampleRatesAt(int index);
    int inputChannelCount()  const {return driver->inputChannelCount();}
    int outputChannelCount() const {return driver->outputChannelCount();}
    int sampleRate()         const {return driver->sampleRate();}
    int period()             const {return driver->period();}
    int periodCount()        const {return driver->periodCount();}
    int inputLatency()       const {return driver->inputLatency();}
    int outputLatency()      const {return driver->outputLatency();}
    int waitTime()           const {return driver->waitTime();}
    bool isDuplex()          const {return driver->isDuplex();}
    bool hasMonitor()        const {return driver->hasMonitor();}
    bool hasMonitorHw()      const {return driver->hasMonitorHw();}
    bool hasMeterHw()        const {return driver->hasMeterHw();}
    bool isSoftMode()        const {return driver->isSoftMode();}
    bool use16bit()          const {return driver->use16bit();}

    void setDriverType(Driver::Type type);
    void setDitherMode(Driver::DitherMode mode);
    void setAlsaMidiType(Driver::AlsaMidiType type);
    void setDeviceName(const QString &name);
    void setDeviceInputName(const QString &name);
    void setDeviceOutputName(const QString &name);
    void setInputChannelCount(int);
    void setOutputChannelCount(int);
    void setSampleRate(int);
    void setPeriod(int);
    void setPeriodCount(int);
    void setInputLatency(int);
    void setOutputLatency(int);
    void setWaitTime(int);
    void setIsDuplex(bool);
    void setHasMonitor(bool);
    void setHasMonitorHw(bool);
    void setHasMeterHw(bool);
    void setIsSoftMode(bool);
    void setUse16bit(bool);

    static ClockSource clockSourceFromString(const QString &value)
    {
        if (value == "h") return ClockSource::HPET;
        else return ClockSource::System;
    }
    static AutoConnectMode autoConnectModeFromString(const QString &value)
    {
        if      (value == "E") return AutoConnectMode::Fail;
        else if (value == "e") return AutoConnectMode::Ignore;
        else if (value == "A") return AutoConnectMode::FailAll;
        else if (value == "a") return AutoConnectMode::IgnoreAll;
        else return AutoConnectMode::NoRestrict;
    }
    static QString autoConnectModeToString(AutoConnectMode value)
    {
        if      (value == AutoConnectMode::Fail) return "E";
        else if (value == AutoConnectMode::Ignore) return "e";
        else if (value == AutoConnectMode::FailAll) return "A";
        else if (value == AutoConnectMode::IgnoreAll) return "a";
        else return "";
    }
private:
    void parseArgsCLI(const std::string &command);

    std::unique_ptr<Driver> driver;
    QString         curProfileName;// .conf profile filename
    QString         name_;         // -n, --name <servername> (unused argument)
    ClockSource     clockSrc_;     // -c, --clock-source <h(pet) | s(ystem)>
    AutoConnectMode connMode_;     // -a, --autoconnect <modechar>
    bool            isRealTime_;   // -r, --no-realtime or -R, --realtime
//  bool            isReplaceReg_; //     --replace-registry
    bool            isSync_;       // -S, --sync
    bool            isTemporary_;  // -T, --temporary
    bool            isVerbose_;    // -v, --verbose
    uint16_t        portMax_;      // -p, --port-max <n>
    uint8_t         rtPriority_;   // -P, --realtime-priority <priority>
    uint16_t        timeOut_;      // -t, --timeout <int>
};
} // namespace jack
