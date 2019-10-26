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

#include <QPlainTextEdit>

class DebugStream;
class DebugLogger : public QPlainTextEdit
{
	Q_OBJECT

public:
	DebugLogger(QWidget *parent = nullptr);
	~DebugLogger();

	void contextMenuEvent(QContextMenuEvent *);

private:
	void onClearTriggered();

	QAction     *actClear;
	DebugStream *errStream;
	DebugStream *outStream;
};
