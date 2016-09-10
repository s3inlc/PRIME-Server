#-------------------------------------------------
#
# Project created by QtCreator 2016-08-03T17:05:40
# Author: Sein Coray <s.coray@unibas.ch>
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = PRIME-Server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    messages.cpp \
    slaveserverprotocolv1.cpp \
    masterserverprotocolv1.cpp \
    masterserver.cpp \
    masterclientprotocolv1.cpp \
    slaveclientprotocolv1.cpp \
    slaveserver.cpp \
    logger.cpp

HEADERS += \
    messages.h \
    slaveserverprotocolv1.h \
    masterserverprotocolv1.h \
    masterserver.h \
    masterclientprotocolv1.h \
    slaveclientprotocolv1.h \
    def.h \
    slaveserver.h \
    logger.h
