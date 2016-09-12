/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#ifndef MASTERSERVER_H
#define MASTERSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <iostream>
#include <QTimer>

#include "masterserverprotocolv1.h"
#include "masterclientprotocolv1.h"
#include "messages.h"
#include "logger.h"
using namespace std;

class MasterServer : public QObject {
    Q_OBJECT
public:
    explicit MasterServer(int port, int clientport, QObject *parent = 0);
    ~MasterServer();

signals:

public slots:
    void newSlaveServer();
    void newClient();
    QList<SlaveServerConnection> getSlaveServers();
    void newMessage(Message m);
    void update();
    void triggerInitFill();
    void createMessages();

private:
    QTcpServer *slaveSocket;
    QTcpServer *clientSocket;
    QList<MasterServerProtocolV1*> slaves;
    QList<MasterClientProtocolV1*> clients;
    Messages messages;
    QTimer timer;
    bool randomFill;
};

#endif // MASTERSERVER_H
