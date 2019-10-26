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
#include "debuglogger.h"
#include "src/debugstream.h"

#include <iostream>

#include <QContextMenuEvent>
#include <QMenu>

DebugLogger::DebugLogger(QWidget *parent) : QPlainTextEdit(parent),
actClear(new QAction(QIcon::fromTheme("edit-clear"), tr("Clear"), this)),
errStream(new DebugStream(std::cerr, this)),
outStream(new DebugStream(std::cout, this))
{
	connect(actClear, &QAction::triggered, this, &DebugLogger::onClearTriggered);
}
DebugLogger::~DebugLogger()
{
	delete errStream;
	delete outStream;
}
void DebugLogger::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu *menu = createStandardContextMenu();

	if (this->toPlainText().isEmpty())
		actClear->setDisabled(true);

	menu->addSeparator();
	menu->addAction(actClear);
	menu->exec(event->globalPos());
	delete menu;
}
void DebugLogger::onClearTriggered()
{
	clear();
}
