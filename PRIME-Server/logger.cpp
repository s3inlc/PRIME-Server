/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#include "logger.h"

LogLevel Logger::lvl(LOG_NORMAL);
LogMode Logger::mode(LOGMODE_STDOUT);

void Logger::log(QString msg, LogLevel level){
    if(mode == LOGMODE_INVALID || lvl == LOG_INVALID){
        return;
    }
    else if(level < lvl || lvl == LOG_NOTHING || mode == LOGMODE_NOTHING){
        return;
    }
    QString type = "";
    switch(level){
        case LOG_DEBUG:
            type = "DEBUG";
            break;
        case LOG_INFO:
            type = "INFO";
            break;
        case LOG_NORMAL:
            type = "NORMAL";
            break;
        case LOG_ERROR:
            type = "ERROR";
            break;
        default:
            type = "UNKNOWN";
    }
    QString message = "[" + type + "]: " + msg;
    if(mode == LOGMODE_BOTH || LOGMODE_STDOUT){
        cout << message.toStdString() << endl;
    }
    if(mode == LOGMODE_BOTH || LOGMODE_FILE){
        QFile file("prime.log");
        message += "\n";
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        if(file.isOpen()){
            file.write(message.toUtf8());
        }
        file.close();
    }
}

LogLevel Logger::parseLevel(QString input){
    if(input.compare("LOG_DEBUG") == 0){
        return LOG_DEBUG;
    }
    else if(input.compare("LOG_INFO") == 0){
        return LOG_INFO;
    }
    else if(input.compare("LOG_NORMAL") == 0){
        return LOG_NORMAL;
    }
    else if(input.compare("LOG_ERROR") == 0){
        return LOG_ERROR;
    }
    else if(input.compare("LOG_NOTHING") == 0){
        return LOG_NOTHING;
    }
    return LOG_INVALID;
}

LogMode Logger::parseMode(QString input){
    if(input.compare("LOGMODE_NOTHING") == 0){
        return LOGMODE_NOTHING;
    }
    else if(input.compare("LOGMODE_FILE") == 0){
        return LOGMODE_FILE;
    }
    else if(input.compare("LOGMODE_STDOUT") == 0){
        return LOGMODE_STDOUT;
    }
    else if(input.compare("LOGMODE_BOTH") == 0){
        return LOGMODE_BOTH;
    }
    return LOGMODE_INVALID;
}

void Logger::setLevel(LogLevel level){
    lvl = level;
}

void Logger::setMode(LogMode m){
    mode = m;
}










