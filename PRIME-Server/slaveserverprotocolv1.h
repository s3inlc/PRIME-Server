/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#ifndef SLAVESERVERPROTOCOLV1_H
#define SLAVESERVERPROTOCOLV1_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <iostream>
#include <QByteArray>

#include "def.h"
#include "messages.h"
#include "logger.h"
using namespace std;

class SlaveServerProtocolV1 : public QObject {
    Q_OBJECT
public:
    explicit SlaveServerProtocolV1(QHostAddress addr, int port, QHostAddress clientAddr, int clientPort, Messages *msg, int extPort);
    ~SlaveServerProtocolV1();
    bool isValid();

public slots:
    void readyRead();
    void connectToMaster();
    void socketError(QAbstractSocket::SocketError err);
    void connectionEstablished();
    void doPing();
    void isDisconnected();

private:
    QTcpSocket *socket;
    SlaveServerState state;
    MasterConnection conn;
    ClientConnection clientConn;
    QTimer pingTimer;
    Messages *messages;
    int lastPing;
    int externalPort;

    void callDiffMessages(QList<QByteArray> content);
    void callNewMessage(QList<QByteArray> content);
};

#endif // SLAVESERVERPROTOCOLV1_H
