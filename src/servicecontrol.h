/*
	From https://github.com/abranson/rockpool/blob/master/rockwork/servicecontrol.h
*/
#pragma once

#include <QDBusInterface>

class ServiceControl : public QObject
{
		Q_OBJECT

public:
		explicit ServiceControl(const QString &name, QObject *parent = 0);

		bool isRunning() const;
		bool start();
		bool stop();
		bool restart();
		bool enable();
		bool disable();

		QString unitProperty(const QString &prop) const
		{
			return unitProperties[prop].toString();
		}

signals:
		void sigActiveStateChanged();

private slots:
		void onPropertiesChanged(QString interface,
														 QMap<QString, QVariant> changed,
														 QStringList invalidated);
private:
	void updateUnitProperties();

	const QString   service;
	QString         serviceName;
	QDBusInterface *systemd;
	QDBusObjectPath unitPath;
	QVariantMap     unitProperties;
	int             oldPid, pid;
};
