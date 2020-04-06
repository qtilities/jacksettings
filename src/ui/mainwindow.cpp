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
#include "mainwindow.h"
#include "servicelogger.h"
#include "src/servicecontrol.h"
#include "src/sysinfo.h"
#include "src/userinfo.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QDir>
#include <QList>
#include <QMenu>
#include <QRadioButton>
#include <QSettings>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTextStream>

#include <iostream>

static int onSampleRateChanged(jack_nframes_t nframes, void* arg)
{
    if (arg == nullptr)
        return 0;

    auto label = (QLabel*)arg;
    std::cerr << "ℹ Sample rate changed to " << nframes << ".\n";
    label->setText(QString::number(nframes));
    return 0;
}
static int onBufferSizeChanged(jack_nframes_t nframes, void* arg)
{
    if (arg == nullptr)
        return 0;

    auto label = (QLabel*)arg;
    std::cerr << "ℹ Buffer size changed to " << nframes << ".\n";
    label->setText(QString::number(nframes));
    return 0;
}
static int onXrun(void* arg) // FIXME: onXrun() doesn't work, need documentation
{
    if (arg == nullptr)
        return 0;

    auto mainWindow = (MainWindow*)arg;
    mainWindow->addXrun();
    return 0;
}
MainWindow::MainWindow(QSystemTrayIcon* icon, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , actA2j(new QAction(this))
    , actJack(new QAction(this))
    , actQuit(new QAction(QIcon::fromTheme("application-exit"), tr("&Quit"), this))
    , mnuTray(new QMenu(this))
    , icoTray(icon)
    , jackClient(nullptr)
    , xRunCount(0)
    , settings(new jack::Settings)
{
    ui->setupUi(this);

    mnuTray->addAction(actJack);
    mnuTray->addAction(actA2j);
    mnuTray->addSeparator();
    mnuTray->addAction(actQuit);

    icoTray->setContextMenu(mnuTray);
    icoTray->show();

    ui->pbnCancel->setIcon(QIcon(":/icons/cancel.png"));
    ui->pbnReset->setIcon(QIcon::fromTheme("edit-clear", QIcon(":/icons/edit-clear.png")));
    ui->pbnSave->setIcon(QIcon::fromTheme("document-save", QIcon(":/icons/document-save.png")));

    grpAutoConnect = {
        ui->rbnNoRestrict,
        ui->rbnFailExt,
        ui->rbnIgnoreExt,
        ui->rbnFailAll,
        ui->rbnIgnoreAll
    };
    grpClockSource = {
        ui->rbnSystem,
        ui->rbnHpet
    };
    // TODO: (Hw) Aliases, Replace Registry? Fix DSP load, xruns and block latency
    ui->chkAliases->setHidden(true);
    ui->chkAliasesHw->setHidden(true);
    ui->chkMemReg->setHidden(true);
    ui->lblBlkLatency->setHidden(true);
    ui->lblBlkLatencyLbl->setHidden(true);
    ui->lblXruns->setHidden(true);
    ui->lblXrunsLbl->setHidden(true);
    ui->lblDspLoad->setHidden(true);
    ui->lblDspLoadLbl->setHidden(true);

    ui->centralwidget->setLayout(ui->layout);

    ui->tabDriver->setLayout(ui->layDriver);
    ui->tabJack->setLayout(ui->layJack);
    ui->tabServices->setLayout(ui->layServices);
    ui->tabSystem->setLayout(ui->laySystem);
    ui->tabLog->setLayout(ui->layLog);

    ui->gbxA2j->setLayout(ui->layA2j);
    ui->gbxJackSvc->setLayout(ui->layJackSvc);
    ui->gbxProfile->setLayout(ui->layProfile);
    ui->gbxSysInfo->setLayout(ui->laySysInfo);
    ui->gbxPrefs->setLayout(ui->layPrefs);

    ui->gbxDevice->setLayout(ui->layDevice);
    ui->gbxProps->setLayout(ui->layProps);
    ui->gbxLatencySet->setLayout(ui->layLatencySet);
    ui->gbxMisc->setLayout(ui->layMisc);

    ui->gbxJackProps->setLayout(ui->layJackProps);
    ui->gbxClockSrc->setLayout(ui->layClockSrc);
    ui->gbxAdvanced->setLayout(ui->layAdvanced);
    ui->gbxAutoConn->setLayout(ui->layAutoConn);

    ui->layServices->addStretch();
    ui->layJack->addStretch();
    ui->layDriver->addStretch();
    ui->laySystem->addStretch();

    loadSettings();

    ui->cbxDevIn->addItem(tr("None"));
    ui->cbxDevOut->addItem(tr("None"));
    ui->cbxDriver->setCurrentIndex(settings->driverType());
    for (int i = 0; i < settings->deviceCount(); ++i) {
        QString text("hw:" + settings->deviceCardIdAt(i)); // TODO: add device/card index
        ui->cbxDevice->addItem(text);
        ui->cbxDevIn->addItem(text);
        ui->cbxDevOut->addItem(text);

        QVector<uint> sampleRates(settings->supportedSampleRatesAt(i));
        foreach (uint sampleRate, sampleRates)
            ui->cbxSampleRate->addItem(QString::number(sampleRate));
    }
    SysInfo sysInfo;
#ifdef Q_OS_LINUX
    QString governor;
    bool hasPerformance = false;
    for (QString gov : sysInfo.governors()) {
        governor = gov;
        if (governor == "performance") {
            hasPerformance = true;
            continue;
        } else {
            hasPerformance = false;
            break;
        }
    }
    QIcon icoWarn(QIcon::fromTheme("dialog-warning", QIcon(":/icons/dialog-warning.png")));

    if (hasPerformance)
        ui->lblGovernorIcon->setPixmap(QIcon(":/icons/check.png").pixmap(QSize(16, 16)));
    else
        ui->lblGovernorIcon->setPixmap(icoWarn.pixmap(QSize(16, 16)));

    ui->lblGovernor->setText(governor);

    if (sysInfo.isRealtime())
        ui->lblKernelVerIcon->setPixmap(QIcon(":/icons/check.png").pixmap(QSize(16, 16)));
    else
        ui->lblKernelVerIcon->setPixmap(icoWarn.pixmap(QSize(16, 16)));
#endif
    ui->lblOS->setText(sysInfo.name());
    setOsPixmap(sysInfo.name());
    ui->lblKernelVer->setText(sysInfo.version());

    UserInfo userInfo;
    ui->lblUsrAudioGrp->setText(userInfo.inAudioGroupLabel());
    ui->lblUsrRtGrp->setText(userInfo.inRealtimeGroupLabel());

    ui->lblUsrAudioGrpIcon->setPixmap(userInfo.inAudioGroupPixmap());
    ui->lblUsrRtGrpIcon->setPixmap(userInfo.inRealtimeGroupPixmap());

    resetJackStatus();

    jack_status_t jackStatus;
    jackClient = jack_client_open("JACKSettings", JackNullOption, &jackStatus);
    if (!jackClient) {
        std::cerr << "⚠ Could not open JACK client.\n";
    } else {
        if (jack_set_buffer_size_callback(jackClient, onBufferSizeChanged, ui->lblBufSize) != 0) {
            std::cerr << "⚠ Could not set buffer size callback.\n";
        }
        if (jack_set_sample_rate_callback(jackClient, onSampleRateChanged, ui->lblSampleRate) != 0) {
            std::cerr << "⚠ Could not set sample rate callback.\n";
        }
        if (jack_set_xrun_callback(jackClient, onXrun, this) != 0) {
            std::cerr << "⚠ Could not set XRun callback.\n";
        }
        if (jackStatus & JackNameNotUnique) {
            std::string name_ = jack_get_client_name(jackClient);
            std::cerr << "ℹ Name was taken: changed to " << name_ << '\n';
        }
        if (jackStatus & JackServerStarted)
            std::cerr << "ℹ Connected to JACK.\n";

        updateJackStatus();
    }
    QString jackSvcName = "jack@" + settings->profileName() + ".service";
    QString a2jSvcName = "a2jmidi@" + settings->profileName() + ".service";
    jackService = new ServiceControl(jackSvcName, this);
    a2jService = new ServiceControl(a2jSvcName, this);
    enumerateProfiles();
    updateJackSettingsUI();
    updateDriverSettingsUI();
    setEnabledButtons(false);

    connect(actA2j, &QAction::triggered, this, &MainWindow::onA2jStartStop);
    connect(actJack, &QAction::triggered, this, &MainWindow::onJackStartStop);
    connect(actQuit, &QAction::triggered, qApp, &QCoreApplication::quit);

    connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::onAboutToQuit);

    connect(ui->pbnCancel, &QPushButton::clicked, this, &MainWindow::onClickedCancel);
    connect(ui->pbnReset, &QPushButton::clicked, this, &MainWindow::onClickedReset);
    connect(ui->pbnSave, &QPushButton::clicked, this, &MainWindow::onClickedSave);

    connect(jackService, &ServiceControl::sigActiveStateChanged,
        this, &MainWindow::onJackActiveStateChanged);
    onJackActiveStateChanged();

    connect(ui->tbnStartJack, &QToolButton::clicked,
        this, &MainWindow::onJackStartStop);

    connect(a2jService, &ServiceControl::sigActiveStateChanged,
        this, &MainWindow::onA2jActiveStateChanged);
    onA2jActiveStateChanged();

    connect(ui->tbnStartA2j, &QToolButton::clicked, this, &MainWindow::onA2jStartStop);

    connect(icoTray, &QSystemTrayIcon::activated,
        [=](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
                setVisible(!isVisible());
        });
    connect(ui->rbnSystem, &QRadioButton::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->rbnHpet, &QRadioButton::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->rbnNoRestrict, &QRadioButton::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->rbnFailExt, &QRadioButton::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->rbnIgnoreExt, &QRadioButton::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->rbnFailAll, &QRadioButton::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->rbnIgnoreAll, &QRadioButton::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->chkRealtime, &QCheckBox::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->chkSvrSync, &QCheckBox::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->chkTemporary, &QCheckBox::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->chkVerbose, &QCheckBox::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->sbxTimeout, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->sbxPortMax, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->sbxRtPrio, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSettingsChanged);

    connect(ui->cbxDevice, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->cbxDevIn, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->cbxDevOut, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->cbxSampleRate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->cbxBufSize, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->cbxDithMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->cbxMidiDriver, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->chkDuplex, &QCheckBox::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->chkMonitor, &QCheckBox::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->chkMonitorHw, &QCheckBox::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->chkMeterHw, &QCheckBox::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->chkSoftMode, &QCheckBox::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->chkForce16Bit, &QCheckBox::clicked, this, &MainWindow::onSettingsChanged);
    connect(ui->sbxChanIn, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->sbxChanOut, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->sbxBufferN, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->sbxLatencyIn, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSettingsChanged);
    connect(ui->sbxLatencyOut, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSettingsChanged);

    txtLog = new ServiceLogger(jackSvcName, this);
    txtLog->setReadOnly(true);
    txtLog->setParent(ui->tabLog);
    txtLog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->layLog->addWidget(txtLog);

    setCentralWidget(ui->centralwidget);
    setWindowTitle(tr("JACK Settings"));
    setIconSize(QSize(24, 24));
}
MainWindow::~MainWindow()
{
}
void MainWindow::closeEvent(QCloseEvent* event)
{
#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif
    if (icoTray->isVisible()) {
        hide();
        event->ignore();
    }
}
void MainWindow::onAboutToQuit()
{
    ui->chkAutoStartJack->isChecked() ? jackService->enable() : jackService->disable();
    ui->chkAutoStartA2j->isChecked() ? a2jService->enable() : a2jService->disable();
    ui->chkAutoStart->isChecked() ? createAutostartFile() : deleteAutostartFile();
    saveSettings();

    //  jack_client_close(jackClient);
    delete ui;
}
void MainWindow::loadSettings()
{
    QSettings qSettings(QSettings::IniFormat, QSettings::UserScope,
        QCoreApplication::organizationName(),
        QCoreApplication::applicationName());

    qSettings.beginGroup("UI");
    resize(qSettings.value("Size", QSize(480, 600)).toSize());
    move(qSettings.value("Position", QPoint(200, 200)).toPoint());
    int index = qSettings.value("TabIndex", 0).toInt();
    ui->chkMinimized->setChecked(qSettings.value("StartMinimized", false).toBool());
    setVisible(!ui->chkMinimized->isChecked());
    if (index < ui->tbwMain->count())
        ui->tbwMain->setCurrentIndex(index);
    qSettings.endGroup();

    qSettings.beginGroup("System");
    ui->chkAutoStart->setChecked(qSettings.value("AutoStart", false).toBool());
    ui->chkAutoStartJack->setChecked(qSettings.value("AutoStartJack", false).toBool());
    ui->chkAutoStartA2j->setChecked(qSettings.value("AutoStartAlsaBridge", false).toBool());
    QString profileName(qSettings.value("CurrentProfile", "default").toString());
    settings->load(profileName);
    qSettings.endGroup();
}
void MainWindow::saveSettings()
{
    QSettings qSettings(QSettings::IniFormat, QSettings::UserScope,
        QCoreApplication::organizationName(),
        QCoreApplication::applicationName());

    updateJackSettings();
    updateDriverSettings();
    settings->save();

    qSettings.beginGroup("UI");
    qSettings.setValue("Size", this->size());
    qSettings.setValue("Position", this->pos());
    qSettings.setValue("TabIndex", ui->tbwMain->currentIndex());
    qSettings.setValue("StartMinimized", ui->chkMinimized->isChecked());
    qSettings.endGroup();

    qSettings.beginGroup("System");
    qSettings.setValue("AutoStart", ui->chkAutoStart->isChecked());
    qSettings.setValue("AutoStartJack", ui->chkAutoStartJack->isChecked());
    qSettings.setValue("AutoStartAlsaBridge", ui->chkAutoStartA2j->isChecked());
    qSettings.setValue("CurrentProfile", settings->profileName());
    qSettings.endGroup();
}
void MainWindow::onClickedCancel()
{
    setEnabledButtons(false);
    updateJackSettingsUI();
    updateDriverSettingsUI();
}
void MainWindow::onClickedReset()
{
    setEnabledButtons(false);
    settings->reset();
    updateJackSettingsUI();
    updateDriverSettingsUI();
}
void MainWindow::onClickedSave()
{
    setEnabledButtons(false);
    saveSettings();
}
void MainWindow::setEnabledButtons(bool enable)
{
    ui->pbnCancel->setEnabled(enable);
    ui->pbnReset->setEnabled(enable);
    ui->pbnSave->setEnabled(enable);
}
void MainWindow::onSettingsChanged()
{
    if (!ui->chkDuplex->isChecked()) {
        ui->cbxDevIn->setEnabled(false);
        ui->cbxDevOut->setEnabled(false);
        ui->cbxDevice->setEnabled(true);
    } else {
        ui->cbxDevIn->setEnabled(true);
        ui->cbxDevOut->setEnabled(true);
        if (ui->cbxDevIn->currentIndex() > 0 || ui->cbxDevOut->currentIndex() > 0)
            ui->cbxDevice->setEnabled(false);
        else
            ui->cbxDevice->setEnabled(true);
    }
    setEnabledButtons(true);
}
void MainWindow::onJackActiveStateChanged()
{
    if (jackService->isRunning()) {
        ui->lblStatusJack->setText(tr("Started"));
        ui->lblStatusJackIcon->setPixmap(QIcon(":/icons/check.png").pixmap(QSize(16, 16)));
        ui->tbnStartJack->setIcon(QIcon::fromTheme("media-playback-stop"));
        ui->tbnStartJack->setText(tr("Stop"));

        actJack->setIcon(QIcon::fromTheme("media-playback-stop"));
        actJack->setText(tr("Stop JACK Server"));

        setWindowIcon(QIcon(":/icons/icon.png"));
        icoTray->setIcon(QIcon(":/icons/icon.png"));
        updateJackStatus();
    } else {
        ui->lblStatusJack->setText(tr("Stopped"));
        ui->lblStatusJackIcon->setPixmap(QIcon(":/icons/cancel.png").pixmap(QSize(16, 16)));
        ui->tbnStartJack->setIcon(QIcon::fromTheme("media-playback-start"));
        ui->tbnStartJack->setText(tr("Start"));

        actJack->setIcon(QIcon::fromTheme("media-playback-start"));
        actJack->setText(tr("Start JACK Server"));

        setWindowIcon(QIcon(":/icons/icon-off.png"));
        icoTray->setIcon(QIcon(":/icons/icon-off.png"));
        resetJackStatus();
        updateJackSettingsUI();
    }
}
void MainWindow::onA2jActiveStateChanged()
{
    if (a2jService->isRunning()) {
        ui->lblStatusA2j->setText(tr("Started"));
        ui->lblStatusA2jIcon->setPixmap(QIcon(":/icons/check.png").pixmap(QSize(16, 16)));
        ui->tbnStartA2j->setIcon(QIcon::fromTheme("media-playback-stop"));
        ui->tbnStartA2j->setText(tr("Stop"));

        actA2j->setIcon(QIcon::fromTheme("media-playback-stop"));
        actA2j->setText(tr("Stop ALSA Bridge"));
    } else {
        ui->lblStatusA2j->setText(tr("Stopped"));
        ui->lblStatusA2jIcon->setPixmap(QIcon(":/icons/cancel.png").pixmap(QSize(16, 16)));
        ui->tbnStartA2j->setIcon(QIcon::fromTheme("media-playback-start"));
        ui->tbnStartA2j->setText(tr("Start"));

        actA2j->setIcon(QIcon::fromTheme("media-playback-start"));
        actA2j->setText(tr("Start ALSA Bridge"));
    }
}
void MainWindow::onJackStartStop()
{
    jackService->isRunning() ? jackService->stop() : jackService->start();
}
void MainWindow::onA2jStartStop()
{
    a2jService->isRunning() ? a2jService->stop() : a2jService->start();
}
void MainWindow::updateJackSettingsUI()
{
    grpClockSource[settings->clockSource()]->setChecked(true);
    grpAutoConnect[settings->autoConnectMode()]->setChecked(true);
    ui->chkRealtime->setChecked(settings->isRealtime());
    ui->chkSvrSync->setChecked(settings->isSync());
    ui->chkTemporary->setChecked(settings->isTemporary());
    ui->chkVerbose->setChecked(settings->isVerbose());
    ui->sbxTimeout->setValue(settings->clientTimeOut());
    ui->sbxPortMax->setValue(settings->portMax());
    ui->sbxRtPrio->setValue(settings->realtimePriority());
}
void MainWindow::updateDriverSettingsUI()
{
    int bufferSize = settings->period();
    for (int index = 0; index < ui->cbxBufSize->count(); ++index) {
        if (ui->cbxBufSize->itemText(index) == QString::number(bufferSize)) {
            ui->cbxBufSize->setCurrentIndex(index);
            break;
        }
    }
    int sampleRate = settings->sampleRate();
    for (int index = 0; index < ui->cbxSampleRate->count(); ++index) {
        if (ui->cbxSampleRate->itemText(index) == QString::number(sampleRate)) {
            ui->cbxSampleRate->setCurrentIndex(index);
            break;
        }
    }
    //ui->cbxDevice->setCurrentIndex(0); TODO: Load device from config file
    //ui->cbxDevIn->setCurrentIndex(0);
    //ui->cbxDevOut->setCurrentIndex(0);
    ui->sbxChanIn->setValue(settings->inputChannelCount());
    ui->sbxChanOut->setValue(settings->outputChannelCount());
    ui->chkDuplex->setChecked(settings->isDuplex());
    ui->chkMonitor->setChecked(settings->hasMonitor());
    ui->chkMonitorHw->setChecked(settings->hasMonitorHw());
    ui->chkMeterHw->setChecked(settings->hasMeterHw());
    ui->sbxBufferN->setValue(settings->periodCount());
    ui->cbxDithMode->setCurrentIndex(settings->ditherMode());
    ui->chkSoftMode->setChecked(settings->isSoftMode());
    ui->chkForce16Bit->setChecked(settings->use16bit());
    ui->sbxLatencyIn->setValue(settings->inputLatency());
    ui->sbxLatencyOut->setValue(settings->outputLatency());
    ui->cbxMidiDriver->setCurrentIndex(settings->alsaMidiType());
}
void MainWindow::updateJackSettings()
{
    for (size_t i = 0; i < grpClockSource.size(); ++i)
        if (grpClockSource.at(i)->isChecked()) {
            settings->setClockSource(static_cast<jack::Settings::ClockSource>(i));
            break;
        }
    for (size_t i = 0; i < grpAutoConnect.size(); ++i)
        if (grpAutoConnect.at(i)->isChecked()) {
            settings->setAutoConnectMode(static_cast<jack::Settings::AutoConnectMode>(i));
            break;
        }
    settings->setRealtime(ui->chkRealtime->isChecked());
    settings->setSync(ui->chkSvrSync->isChecked());
    settings->setTemporary(ui->chkTemporary->isChecked());
    settings->setVerbose(ui->chkVerbose->isChecked());
    settings->setClientTimeout(ui->sbxTimeout->value());
    settings->setPortMax(ui->sbxPortMax->value());
    settings->setRealtimePriority(ui->sbxRtPrio->value());
}
void MainWindow::updateDriverSettings()
{
    jack::Driver::DitherMode ditherMode = static_cast<jack::Driver::DitherMode>(ui->cbxDithMode->currentIndex());

    jack::Driver::AlsaMidiType alsaMidiType = static_cast<jack::Driver::AlsaMidiType>(ui->cbxMidiDriver->currentIndex());

    if (ui->cbxDevIn->currentIndex() > 0 && ui->chkDuplex->isChecked())
        settings->setDeviceInputName(ui->cbxDevIn->currentText());
    else
        settings->setDeviceInputName(QString());

    if (ui->cbxDevOut->currentIndex() > 0 && ui->chkDuplex->isChecked())
        settings->setDeviceOutputName(ui->cbxDevOut->currentText());
    else
        settings->setDeviceOutputName(QString());

    settings->setDeviceName(ui->cbxDevice->currentText());
    settings->setInputChannelCount(ui->sbxChanIn->value());
    settings->setOutputChannelCount(ui->sbxChanOut->value());
    settings->setIsDuplex(ui->chkDuplex->isChecked());
    settings->setHasMonitor(ui->chkMonitor->isChecked());
    settings->setHasMonitorHw(ui->chkMonitorHw->isChecked());
    settings->setHasMeterHw(ui->chkMeterHw->isChecked());
    settings->setSampleRate(ui->cbxSampleRate->currentText().toInt());
    settings->setPeriod(ui->cbxBufSize->currentText().toInt());
    settings->setPeriodCount(ui->sbxBufferN->value());
    settings->setDitherMode(ditherMode);
    settings->setIsSoftMode(ui->chkSoftMode->isChecked());
    settings->setUse16bit(ui->chkForce16Bit->isChecked());
    settings->setInputLatency(ui->sbxLatencyIn->value());
    settings->setOutputLatency(ui->sbxLatencyOut->value());
    settings->setAlsaMidiType(alsaMidiType);
}
void MainWindow::setOsPixmap(const QString& osName)
{
#ifdef Q_OS_LINUX
    if (osName == "ArchLinux")
        ui->lblOSIcon->setPixmap(QIcon(":icons/arch.png").pixmap(QSize(16, 16)));
#endif
}
void MainWindow::enumerateProfiles()
{
    QDir configDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/jack");

    QStringList filters;
    filters << "*.conf";
    QStringList configFiles(configDir.entryList(filters, QDir::Files, QDir::Name));
    for (QString fileName : configFiles)
        ui->cbxProfile->addItem(fileName.replace(".conf", ""));

    // If no profiles found then is a first run, create a default config
    if (configFiles.empty()) {
        if (settings->save())
            enumerateProfiles();
    }
}
void MainWindow::createAutostartFile()
{
    QDir configDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    QString filePath(configDir.filePath("autostart/jacksettings.desktop"));
    QFile file(filePath);

    if (file.exists() || !file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Name=JACKSettings\n";
    out << "Type=Application\n";
    out << "Exec=jacksettings\n";
    out << "Terminal=false\n";
}
void MainWindow::deleteAutostartFile()
{
    QDir configDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    QString filePath(configDir.filePath("autostart/jacksettings.desktop"));
    QFile file(filePath);

    if (!file.exists())
        return;

    file.remove();
}
void MainWindow::addXrun()
{
    ++xRunCount;
    ui->lblXruns->setText(QString::number(xRunCount));
}
void MainWindow::resetJackStatus()
{
    ui->lblDspLoad->setText("-");
    ui->lblXruns->setText("-");
    ui->lblBufSize->setText("-");
    ui->lblSampleRate->setText("-");
    ui->lblBlkLatency->setText("-");
}
void MainWindow::updateJackStatus()
{
    int nFrames = jack_get_buffer_size(jackClient);
    ui->lblBufSize->setText(QString::number(nFrames));

    nFrames = jack_get_sample_rate(jackClient);
    ui->lblSampleRate->setText(QString::number(nFrames));

    ui->lblXruns->setText(QString::number(xRunCount));
}
