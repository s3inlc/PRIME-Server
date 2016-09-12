/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#ifndef MASTERSERVERPROTOCOLV1_H
#define MASTERSERVERPROTOCOLV1_H

#include <QObject>
#include <QTcpSocket>
#include <iostream>
#include <QHostAddress>
#include <QList>
#include <QByteArray>

#include "def.h"
#include "messages.h"
#include "logger.h"
using namespace std;

class MasterServerProtocolV1 : public QObject {
    Q_OBJECT
public:
    explicit MasterServerProtocolV1(QTcpSocket *sock, Messages *msg, QObject *parent = 0);
    ~MasterServerProtocolV1();
    bool isValid();
    bool isReady();
    SlaveServerConnection getServer();

signals:

public slots:
    void readyRead();
    void socketError(QAbstractSocket::SocketError err);
    void issueNewMessage(Message m);

private:
    QTcpSocket *socket;
    MasterServerState state;
    SlaveServerConnection server;
    Messages *messages;
    int lastPing;

    //handling functions
    void callRegister(QList<QByteArray> content);
    void callDiffMessages(QList<QByteArray> content);
    void callPing(QList<QByteArray> content);
    void callNewMessage(QList<QByteArray> content);
};

#endif // MASTERSERVERPROTOCOLV1_H
