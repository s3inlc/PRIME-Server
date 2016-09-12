/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#ifndef SLAVESERVER_H
#define SLAVESERVER_H

#include <QObject>
#include <iostream>
#include <QTcpServer>

#include "def.h"
#include "slaveclientprotocolv1.h"
#include "slaveserverprotocolv1.h"
#include "messages.h"
#include "logger.h"
using namespace std;

class SlaveServer : public QObject {
    Q_OBJECT
public:
    explicit SlaveServer(QHostAddress master, int masterPort, QHostAddress own, int clientPort, int externPort, QObject *parent = 0);
    ~SlaveServer();

public slots:
    void newClient();
    void update();

private:
    QTcpServer *clientSocket;
    SlaveServerProtocolV1 *masterSocket;
    QList<SlaveClientProtocolV1*> clients;
    Messages messages;
    QTimer timer;
};

#endif // SLAVESERVER_H
