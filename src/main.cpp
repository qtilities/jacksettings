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
#include "ui/mainwindow.h"

#include <iostream>

#include <QApplication>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QTranslator>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

    app.setOrganizationName("AZDrums");
    app.setOrganizationDomain("azdrums.org");
    app.setApplicationName("JACKSettings");

    QIcon icon = QIcon(":/icons/icon.png");
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(icon, &app);
    trayIcon->setVisible(true);

    QTranslator translator;
    if (translator.load(QLocale(), QLatin1String("jacksettings"), QLatin1String("_"),
            QLatin1String(":/translations")))
        app.installTranslator(&translator);

    MainWindow window(trayIcon);

    return app.exec();
}
