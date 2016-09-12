/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#ifndef MASTERCLIENTPROTOCOLV1_H
#define MASTERCLIENTPROTOCOLV1_H

#include <QObject>
#include <QTcpSocket>

#include "def.h"
#include "messages.h"
#include "logger.h"
using namespace std;

class MasterClientProtocolV1 : public QObject {
    Q_OBJECT
public:
    explicit MasterClientProtocolV1(QTcpSocket *sock, Messages *msg, QObject *parent = 0);
    ~MasterClientProtocolV1();
    bool isValid();

signals:
    QList<SlaveServerConnection> getSlaveServers();
    void newMessage(Message);

public slots:
    void readyRead();
    void socketError(QAbstractSocket::SocketError err);

private:
    QTcpSocket *socket;
    Messages *messages;
    int lastPing;
    bool connectionIsValid;

    //handling functions
    void callGetServers(QList<QByteArray> content);
    void callGetAddresses(QList<QByteArray> content);
    void callNewMessage(QList<QByteArray> content);
    void callPing(QList<QByteArray> content);
};

#endif // MASTERCLIENTPROTOCOLV1_H
