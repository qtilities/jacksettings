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

#include <QObject>
#include <QVector>

class SysInfo : public QObject
{
	Q_OBJECT

public:
	explicit SysInfo(QObject *parent = nullptr);

#ifdef Q_OS_LINUX
	QVector<QString> governors() const {return governors_;}
	bool isRealtime() const {return realtime;}
#endif
	QString name() const;
	QString version() const;
	int cpuCount() const;

private:
	QString name_;
#ifdef Q_OS_LINUX
	QVector<QString> governors_;
	bool realtime;
#endif
	short cpuCount_;
};
