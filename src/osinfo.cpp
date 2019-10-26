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
#include "osinfo.h"

#include <QFile>
#include <QProcess>
#include <QSysInfo>

OsInfo::OsInfo(QObject *parent) : QObject(parent)
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
QString OsInfo::name() const
{
	return name_;
}
QString OsInfo::version() const
{
	return QSysInfo::kernelVersion();
}
int OsInfo::cpuCount() const
{
	return cpuCount_;
}
