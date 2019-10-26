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
#include "ui/mainwindow.h"
#include "ui/debuglogger.h"
#include "src/debugstream.h"

#include <iostream>

#include <QApplication>
#include <QMessageBox>
#include <QTranslator>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	if (!QSystemTrayIcon::isSystemTrayAvailable())
	{
		QMessageBox::critical(nullptr, QObject::tr("Systray"),
													QObject::tr("I couldn't detect any system tray "
																			"on this system."));
			return 1;
	}
	QApplication::setQuitOnLastWindowClosed(false);

	// Set application data also for settings
	app.setOrganizationName("AZDrums");
	app.setOrganizationDomain("azdrums.org");
	app.setApplicationName("JACKSettings");

	// Load locale translation file if any
	QTranslator translator;
	if (translator.load(QLocale(), QLatin1String("jacksettings"), QLatin1String("_"),
											QLatin1String(":/translations")))
			app.installTranslator(&translator);

	DebugLogger *txtLog(new DebugLogger());

	MainWindow window;
	window.setLogger(txtLog);
	window.show();

	return app.exec();
}
