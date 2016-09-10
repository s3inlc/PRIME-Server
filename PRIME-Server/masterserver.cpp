/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#include "masterserver.h"

MasterServer::MasterServer(int port, int clientport, QObject *parent) : QObject(parent){
    slaveSocket = new QTcpServer(this);
    slaveSocket->listen(QHostAddress::Any, port);
    Logger::log("Master Server: Listening on port " + QString::number(port) + " for slave servers...", LOG_NORMAL);
    connect(slaveSocket, SIGNAL(newConnection()), this, SLOT(newSlaveServer()));
    clientSocket = new QTcpServer(this);
    clientSocket->listen(QHostAddress::Any, clientport);
    Logger::log("Master Server: Listening on port " + QString::number(clientport) + " for clients...", LOG_NORMAL);
    connect(clientSocket, SIGNAL(newConnection()), this, SLOT(newClient()));

    timer.setInterval(5000);
    connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
    timer.start();
}

MasterServer::~MasterServer(){
    delete slaveSocket;
    delete clientSocket;
}

void MasterServer::newSlaveServer(){
    QTcpSocket *sock = slaveSocket->nextPendingConnection();
    Logger::log("Master Server: New slave server connection from " + sock->peerAddress().toString(), LOG_INFO);
    MasterServerProtocolV1 *protocol = new MasterServerProtocolV1(sock, &messages, this);
    slaves.append(protocol);
}

void MasterServer::newClient(){
    QTcpSocket *sock = clientSocket->nextPendingConnection();
    Logger::log("Master Server: New client connection from " + sock->peerAddress().toString(), LOG_INFO);
    MasterClientProtocolV1 *protocol = new MasterClientProtocolV1(sock, &messages, this);
    connect(protocol, SIGNAL(getSlaveServers()), this, SLOT(getSlaveServers()));
    connect(protocol, SIGNAL(newMessage(Message)), this, SLOT(newMessage(Message)));
    clients.append(protocol);
}

QList<SlaveServerConnection> MasterServer::getSlaveServers(){
    QList<SlaveServerConnection> list;
    for(int x=0;x<slaves.size();x++){
        if(slaves.at(x)->isReady()){
            list.append(slaves.at(x)->getServer());
        }
    }
    Logger::log("Master Server: Currently " + QString::number(list.size()) + " slave servers available", LOG_DEBUG);
    return list;
}

void MasterServer::newMessage(Message m){
    Logger::log("Master Server: Distributing new message to " + QString::number(slaves.size()) + " slaves...", LOG_DEBUG);
    for(int x=0;x<slaves.size();x++){
        slaves.at(x)->issueNewMessage(m);
    }
}

void MasterServer::update(){
    for(int x=0;x<slaves.size();x++){
        if(!slaves.at(x)->isValid()){
            Logger::log("Master Server: Removed invalid slave from stack!", LOG_INFO);
            delete slaves.at(x);
            slaves.removeAt(x);
            x--;
        }
    }
    for(int x=0;x<clients.size();x++){
        if(!clients.at(x)->isValid()){
            Logger::log("Master Server: Removed invalid client from stack!", LOG_INFO);
            delete clients.at(x);
            clients.removeAt(x);
            x--;
        }
    }
}

