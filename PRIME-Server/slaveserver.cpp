/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#include "slaveserver.h"

SlaveServer::SlaveServer(QHostAddress master, int masterPort, QHostAddress own, int clientPort, int externPort,  QObject *parent) : QObject(parent){
    masterSocket = new SlaveServerProtocolV1(master, masterPort, own, clientPort, &messages, externPort);
    clientSocket = new QTcpServer(this);
    clientSocket->listen(own, clientPort);
    cout << "Slave Server listening on port " << clientPort << " for clients..." << endl;
    connect(clientSocket, SIGNAL(newConnection()), this, SLOT(newClient()));

    timer.setInterval(5000);
    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
    timer.start();
}

SlaveServer::~SlaveServer(){
    delete masterSocket;
    delete clientSocket;
}

void SlaveServer::newClient(){
    QTcpSocket *sock = clientSocket->nextPendingConnection();
    cout << "New client connection from " << sock->peerAddress().toString().toStdString() << endl;
    SlaveClientProtocolV1 *protocol = new SlaveClientProtocolV1(sock, &messages, this);
    clients.append(protocol);
}

void SlaveServer::update(){
    for(int x=0;x<clients.size();x++){
        if(!clients.at(x)->isValid()){
            delete clients.at(x);
            clients.removeAt(x);
            x--;
        }
    }
}

