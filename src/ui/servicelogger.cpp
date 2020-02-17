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
#include "servicelogger.h"

#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QMenu>

ServiceLogger::ServiceLogger(QString unitName, QWidget *parent)
:   QPlainTextEdit(parent),
    actClear(new QAction(QIcon::fromTheme("edit-clear"), tr("Clear"), this)),
    journalProcess(new QProcess(this))
{
    QString cmd = QString("journalctl -f -S today --user-unit %1").arg(unitName);
    journalProcess->start(cmd);

    setLineWrapMode(QPlainTextEdit::NoWrap);

    connect(actClear,       &QAction::triggered,  this, &ServiceLogger::onClearTriggered);
    connect(journalProcess, &QProcess::readyRead, this, &ServiceLogger::onReadyRead);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
        this, &ServiceLogger::onAboutToQuit);
}
ServiceLogger::~ServiceLogger()
{
}
void ServiceLogger::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();

    if (this->toPlainText().isEmpty())
        actClear->setDisabled(true);

    menu->addSeparator();
    menu->addAction(actClear);
    menu->exec(event->globalPos());
    delete menu;
}
void ServiceLogger::onAboutToQuit()
{
    delete journalProcess;
}
void ServiceLogger::onClearTriggered()
{
    clear();
}
void ServiceLogger::onReadyRead()
{
    while (journalProcess->canReadLine())
    {
        QString line(journalProcess->readLine());
        moveCursor (QTextCursor::End);
        this->insertPlainText(line);
        moveCursor (QTextCursor::End); // Avoid double newline
    }
}
