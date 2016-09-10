/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <iostream>
#include <QFile>

#include "def.h"
using namespace std;

class Logger : public QObject
{
    Q_OBJECT
public:
    static void log(QString msg, LogLevel level);
    static LogLevel parseLevel(QString input);
    static LogMode parseMode(QString input);
    static void setLevel(LogLevel level);
    static void setMode(LogMode m);

private:
    static LogLevel lvl;
    static LogMode mode;
};

#endif // LOGGER_H
