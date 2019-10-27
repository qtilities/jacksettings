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

#include "src/settings.h"

#include <QMainWindow>
#include <QRadioButton>
#include <QSystemTrayIcon>

#include <jack/jack.h>

#include <array>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ServiceControl;
class DebugLogger;
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void addXrun();

	void setLogger(DebugLogger *);

private:
	void closeEvent(QCloseEvent *);

	void createTrayIcon();
	void iconActivated(QSystemTrayIcon::ActivationReason reason);

	void loadSettings();
	void saveSettings();

	void onClickedCancel();
	void onClickedReset();
	void onClickedSave();
	void setEnabledButtons(bool enable);
	void onSettingsChanged();

	void onAboutToQuit();
	void onJackActiveStateChanged();
	void onJackStartStop();

	void setJackEnabled(bool enabled);

	void resetJackStatus();
	void updateJackStatus();

	void onA2jActiveStateChanged();
	void onA2jStartStop();
	void setA2jEnabled(bool enabled);

	void updateJackSettingsUI();
	void updateDriverSettingsUI();

	void updateJackSettings();
	void updateDriverSettings();

	void setOsPixmap(const QString &osName);

	void enumerateProfiles();

	Ui::MainWindow *ui;

	QAction         *actA2j;
	QAction         *actJack;
	QAction         *actQuit;
	QMenu           *mnuTray;
	QSystemTrayIcon *icoTray;
	DebugLogger     *txtLog;

	std::array<QRadioButton *, 2> grpClockSource;
	std::array<QRadioButton *, 5> grpAutoConnect;

	ServiceControl *jackService;
	ServiceControl *a2jmidiService;

	jack_client_t *jackClient;
	int	           xRunCount;

	typedef QSharedPointer<jack::Settings> settingsPtr;
	settingsPtr settings;

	QString currProfileName;
};
