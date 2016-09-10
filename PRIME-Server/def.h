/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#ifndef DEF_H
#define DEF_H

#include <QHostAddress>
#include <QString>
#include <QByteArray>

#define PING_TIMEOUT 30

#define MSG_FOLDER "msg/" //path where the message storage structure is located (relative or absolute)
#define MSG_TIMEOUT 86400*7 //7 days currently
#define MSG_LIST_LIMIT 20 //number of messages which are sent at once to a slave server

enum LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_NORMAL,
    LOG_ERROR,
    LOG_NOTHING,
    LOG_INVALID
};

enum LogMode {
    LOGMODE_NOTHING,
    LOGMODE_FILE,
    LOGMODE_STDOUT,
    LOGMODE_BOTH,
    LOGMODE_INVALID
};

enum MasterServerState {
    MSSINIT,
    MSSVALID,
    MSSREGISTERED,
    MSSREADY,
    MSSERR
};

enum SlaveServerState {
    SSSINIT,
    SSSREGISTER,
    SSSUPDATE,
    SSSREADY
};

struct SlaveServerConnection {
    QHostAddress address;
    int port;
};

struct MasterConnection {
    QHostAddress address;
    int port;
};

struct ClientConnection {
    QHostAddress address;
    int port;
};

struct Message {
    QString id;
    QByteArray data;
    long long int time;
};

struct MessageIndex {
    QString path;
    long long int time;
};

#endif // DEF_H

