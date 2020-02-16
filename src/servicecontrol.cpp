/*
    From https://github.com/abranson/rockpool/blob/master/rockwork/servicecontrol.cpp
*/
#include "servicecontrol.h"
#include <iostream>
#include <QDBusReply>

ServiceControl::ServiceControl(const QString &name, QObject *parent) : QObject(parent),
    service("org.freedesktop.systemd1"),
    serviceName(name),
    systemd(new QDBusInterface(service, "/org/freedesktop/systemd1",
            "org.freedesktop.systemd1.Manager",
            QDBusConnection::sessionBus(), this)),
    oldPid(0),
    pid(0)
{
    systemd->call("Subscribe");

    QDBusReply<QDBusObjectPath> unit = systemd->call("LoadUnit", serviceName);
    if (unit.isValid())
    {
        unitPath = unit.value();
        updateUnitProperties();
        QDBusConnection::sessionBus().connect(service, unitPath.path(),
            "org.freedesktop.DBus.Properties", "PropertiesChanged",
            this, SLOT(onPropertiesChanged(QString, QMap<QString, QVariant>, QStringList)));
    }
    else
    {
        std::cerr << "Error: " << unit.error().message().toStdString() << '\n';
    }
}

void ServiceControl::onPropertiesChanged(QString interface,
                                         QMap<QString, QVariant> changed,
                                         QStringList invalidated)
{
    if (interface != "org.freedesktop.systemd1.Unit")
        return;

    std::cerr << "=== Interface: " << interface.toStdString() << " ===\n";
    QMap<QString, QVariant>::const_iterator it;
    for (it = changed.constBegin(); it != changed.constEnd(); ++it)
    {
        std::cerr << it.key().toStdString() << ": " << it.value().toString().toStdString() << '\n';
        if (it.key() == "MainPID")
        {
            pid = it.value().toInt();
        }
        else if (it.key() == "ExecMainPID")
        {
            oldPid = it.value().toInt();
        }
    }
//    if (pid != oldPid)
//        emit sigActiveStateChanged();

    std::cerr << "=== Invalidated ===\n";

    for (QString invalid: invalidated)
        std::cerr << invalid.toStdString() << '\n';

    if (invalidated.contains("Conditions") || invalidated.contains("Asserts"))
            updateUnitProperties();
}

bool ServiceControl::isRunning() const
{
    return unitProperties["ActiveState"].toString() == "active";
}

bool ServiceControl::start()
{
    QDBusError reply;
    systemd->call("StartUnitFiles", QStringList() << serviceName, false, true);

    if (reply.isValid())
    {
        std::cerr << "Start " << serviceName.toStdString() << ": " <<
        reply.message().toStdString() << '\n';
        return false;
    }
    else
    {
        systemd->call("Reload");
        systemd->call("StartUnit", serviceName, "replace");
        return true;
    }
}
bool ServiceControl::stop()
{
    QDBusError reply;
    systemd->call("StopUnit", serviceName, "replace");

    if (reply.isValid())
    {
        std::cerr << "Disabled " << serviceName.toStdString() << ": " <<
        reply.message().toStdString() << '\n';
        return false;
    }
    else
    {
        systemd->call("Reload");
        return true;
    }
}
bool ServiceControl::restart()
{
    return stop() && start();
}
bool ServiceControl::enable()
{
    QDBusError reply;
    systemd->call("EnableUnitFiles", QStringList() << serviceName, false, true);

    if (reply.isValid())
    {
        std::cerr << "Enable " << serviceName.toStdString() << ": " <<
        reply.message().toStdString() << '\n';
        return false;
    }
    else
    {
        systemd->call("Reload");
        systemd->call("StartUnit", serviceName, "replace");
        return true;
    }
}
bool ServiceControl::disable()
{
    QDBusError reply;
    systemd->call("DisableUnitFiles", QStringList() << serviceName, false);

    if (reply.isValid())
    {
        std::cerr << "Stop " << serviceName.toStdString() << ": " <<
        reply.message().toStdString() << '\n';
        return false;
    }
    else
    {
        systemd->call("Reload");
        return true;
    }
}
void ServiceControl::updateUnitProperties()
{
    QDBusMessage request =
        QDBusMessage::createMethodCall(service, unitPath.path(),
            "org.freedesktop.DBus.Properties", "GetAll");

    request << "org.freedesktop.systemd1.Unit";

    QDBusReply<QVariantMap> reply = QDBusConnection::sessionBus().call(request);

    if (reply.isValid())
    {
        QVariantMap newProperties = reply.value();

        bool activeStateChanged =
            (unitProperties["ActiveState"] != newProperties["ActiveState"]);

        unitProperties = newProperties;

        if (activeStateChanged)
            emit sigActiveStateChanged();
    }
    else
    {
        std::cerr << reply.error().message().toStdString() << '\n';
    }
}
