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
#include "sysinfo.h"

#include <QFile>
#include <QProcess>
#include <QSysInfo>

SysInfo::SysInfo(QObject *parent) : QObject(parent)
{
#ifdef Q_OS_LINUX
    QProcess proc;
    proc.start("nproc");
    proc.waitForFinished();
    cpuCount_= proc.readAllStandardOutput().simplified().toInt();
#elif defined(Q_OS_WIN)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    cpuCount_ = sysinfo.dwNumberOfProcessors;
#elif defined(Q_OS_MAC)
    cpuCount_= sysconf(_SC_NPROCESSORS_ONLN);
#else
    cpuCount_= 1; // TODO
#endif

#ifdef Q_OS_LINUX
    for (int i = 0; i < cpuCount_; ++i)
    {
        QString governor(QString("/sys/devices/system/cpu/cpu%1/cpufreq/scaling_governor").arg(i));
        QFile file(governor);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            governors_.push_back(tr("<invalid>"));
            continue;
        }
        else
        {
            governors_.push_back(file.readLine().simplified());
        }
    }
    proc.start("zcat /proc/config.gz");
    proc.waitForFinished();
    realtime = false;
    QString output(proc.readAllStandardOutput());
    if (output.contains("CONFIG_PREEMPT_RT=y") ||
            output.contains("CONFIG_PREEMPT_RT_FULL=y"))
        realtime = true;
#endif

    QString osName(QSysInfo::productType());
#ifdef Q_OS_LINUX
    if (osName == "arch")
        name_= "ArchLinux";
    else
        name_= osName;
#else
    name_= osName; // TODO
#endif
}
QString SysInfo::name() const
{
    return name_;
}
QString SysInfo::version() const
{
    return QSysInfo::kernelVersion();
}
int SysInfo::cpuCount() const
{
    return cpuCount_;
}
