#-------------------------------------------------
#
# Project created by QtCreator 2020-11-04T13:32:09
#
#-------------------------------------------------

INCLUDEPATH += $$quote(C:/Program Files/OpenSSL-Win64/include)

LIBS +=$$quote(C:/Program Files/OpenSSL-Win64/lib/libcrypto.lib)
LIBS +=$$quote(C:/Program Files/OpenSSL-Win64/lib/libssl.lib)

QT       += core gui
QT       += serialport
QT       += xml
QT       += charts
QT       += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KilnSampler
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

FORMS += \
    mainwindow.ui

HEADERS += \
    Serial/serial_bool.h \
    Serial/serial_dev.h \
    Timer/bool_timer.h \
    Timer/dev_timer.h \
    Timer/count_timer.h \
    Timer/network_timer.h \
    mainwindow.h \
    Device/device_ellipsometer_classical.h \
    Device/device_ellipsometer_new.h \
    Device/device_laser_big.h \
    Device/device_laser_small.h \
    Device/IDevice.h \
    Thread/sample_task.h \
    Thread/chart_task.h \
    Thread/network_task.h \
    Profile/profile.h \
    Project/IProject.h \
    Project/fake_map.h \
    global.h \
    Project/IPara.h \
    Chart/IChart.h \
    Network/network.h \
    Cryption/rsa/rsa.h \
    Cryption/aes/aes.h \
    Logs/logs.h

SOURCES += \
    Serial/serial_bool.cpp \
    Serial/serial_dev.cpp \
    Timer/bool_timer.cpp \
    Timer/dev_timer.cpp \
    Timer/count_timer.cpp \
    Timer/network_timer.cpp \
    main.cpp \
    mainwindow.cpp \
    sample_about.cpp \
    Device/device_ellipsometer_classical.cpp \
    Device/device_ellipsometer_new.cpp \
    Device/device_laser_big.cpp \
    Device/device_laser_small.cpp \
    Device/IDevice.cpp \
    device_about.cpp \
    serial_about.cpp \
    dev_fun_about.cpp \
    Thread/sample_task.cpp \
    Thread/chart_task.cpp \
    Thread/network_task.cpp \
    file_about.cpp \
    Profile/profile.cpp \
    Project/IProject.cpp \
    project_about.cpp \
    global.cpp \
    Project/IPara.cpp \
    Chart/IChart.cpp \
    Network/network.cpp \
    network_about.cpp \
    chart_about.cpp \
    profile_about.cpp \
    Cryption/rsa/rsa.cpp \
    Cryption/aes/aes.cpp \
    Logs/logs.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    profile.xml \
    config.txt


