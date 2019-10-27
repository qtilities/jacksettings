#-------------------------------------------------
#
# Project created by QtCreator 2019-10-11T19:30:00
#
#-------------------------------------------------
QT += core gui dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
CONFIG  += c++11
TARGET   = jacksettings

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += src/main.cpp \
					 src/device.cpp \
					 src/driver.cpp \
					 src/osinfo.cpp \
					 src/servicecontrol.cpp \
					 src/settings.cpp \
					 src/ui/debuglogger.cpp \
					 src/ui/mainwindow.cpp \
					 src/userinfo.cpp
HEADERS += src/ui/mainwindow.h \
					 src/debugstream.h \
					 src/device.h \
					 src/driver.h \
					 src/osinfo.h \
					 src/servicecontrol.h \
					 src/settings.h \
					 src/ui/debuglogger.h \
					 src/userinfo.h
FORMS   += src/ui/mainwindow.ui

INCLUDEPATH += $$_PRO_FILE_PWD_/external/CLI11/include

LIBS += -ljack -lasound

RESOURCES += resources/resources.qrc

TRANSLATIONS += resources/translations/jacksettings_it.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += .editorconfig \
						 README.md \
						 TODO.md
