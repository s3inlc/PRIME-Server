/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#ifndef SLAVECLIENTPROTOCOLV1_H
#define SLAVECLIENTPROTOCOLV1_H

#include <QObject>
#include <QTcpSocket>
#include <iostream>

#include "def.h"
#include "messages.h"
#include "logger.h"
using namespace std;

class SlaveClientProtocolV1 : public QObject {
    Q_OBJECT
public:
    explicit SlaveClientProtocolV1(QTcpSocket *sock, Messages *msg, QObject *parent = 0);
    ~SlaveClientProtocolV1();
    bool isValid();

public slots:
    void readyRead();
    void socketError(QAbstractSocket::SocketError err);

private:
    QTcpSocket *socket;
    Messages *messages;
    bool connectionIsValid;

    //handling functions
    void callGetMessages(QList<QByteArray> content);
};

#endif // SLAVECLIENTPROTOCOLV1_H
