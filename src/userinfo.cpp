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
#include "userinfo.h"

#include <QFile>
#include <QIcon>
#include <QProcess>
#include <QString>
#include <QTextStream>

UserInfo::UserInfo(QObject *parent) : QObject(parent)
{
    QProcess proc;
    proc.start("whoami");
    proc.waitForFinished();
    username = proc.readAllStandardOutput().simplified();

    proc.start("groups");
    proc.waitForFinished();
    QString groups(proc.readAllStandardOutput());

    if (groups.contains("audio"))
    {
        inAudioGrpLabel  = tr("Yes");
        inAudioGrpPixmap = QIcon(":/icons/check.png").pixmap(QSize(16, 16));
    }
    else
    {
        checkInGroup("audio");
    }
    if (groups.contains("realtime"))
    {
        inRealtimeGrpLabel  = tr("Yes");
        inRealtimeGrpPixmap = QIcon(":/icons/check.png").pixmap(QSize(16, 16));
    }
    else
    {
        checkInGroup("realtime");
    }
}
void UserInfo::checkInGroup(const QString &group)
{
    QFile file("/etc/group");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

    bool isInGroup = false;
    QTextStream in(&file);
    QString line;
    while (!in.atEnd())
    {
        line = in.readLine();
        if (line.contains(username))
        {
            if (line.startsWith(group))
            {
                isInGroup = true;
                break;
            }
        }
    }
    if (group == "audio")
    {
        if (isInGroup)
        {
            inAudioGrpLabel  = tr("Yes, need relogin");
            inAudioGrpPixmap = QIcon(":/icons/warning.png").pixmap(QSize(16, 16));
        }
        else
        {
            inAudioGrpLabel  = tr("No");
            inAudioGrpPixmap = QIcon(":/icons/cancel.png").pixmap(QSize(16, 16));
        }
    }
    else if (group == "realtime")
    {
        if (isInGroup)
        {
            inRealtimeGrpLabel  = tr("Yes, need relogin");
            inRealtimeGrpPixmap = QIcon(":/icons/warning.png").pixmap(QSize(16, 16));
        }
        else
        {
            inRealtimeGrpLabel  = tr("No");
            inRealtimeGrpPixmap = QIcon(":/icons/cancel.png").pixmap(QSize(16, 16));
        }
    }
}

