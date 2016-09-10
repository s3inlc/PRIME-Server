/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include <QObject>
#include <QList>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <time.h>
#include <iostream>
#include <QByteArray>
#include <QTimer>

#include "def.h"
#include "logger.h"
using namespace std;

class Messages : public QObject
{
    Q_OBJECT
public:
    explicit Messages(QObject *parent = 0);

    QList<Message> getMessages(QString id);
    bool addMessage(Message);
    QByteArray getData(QStringList ids);
    QList<Message> getMessageBatch(int time);
    QList<QString> getAllIds();
    int getNewestTime();

    QString getId(QString fullPath);
    int getTime(QString fullPath);

signals:

public slots:
    void clean(); //remove older messages

private:
    QList<QString> knownIds;
    QString path;
    QList<MessageIndex> index;
    int newestTime;
    QTimer cleanTimer;
};

#endif // MESSAGES_H
