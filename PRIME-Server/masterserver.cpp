/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#include "masterserver.h"

MasterServer::MasterServer(int port, int clientport, QObject *parent) : QObject(parent){
    randomFill = false;
    srand(time(NULL));
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
    if(randomFill){
        createMessages();
        randomFill = false;
    }
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

void MasterServer::triggerInitFill(){
    randomFill = true;
}

void MasterServer::createMessages(){
    Logger::log("Filling the message directory with some random data...", LOG_INFO);
    int numIds = rand() % 100 + 50;
    for(int i=0;i<numIds;i++){
        int numMessages = rand() % 10 + 2;
        QString id;
        for(int j=0;j<40;j++){
            id.append(QString::number(rand()%16, 16));
        }
        Logger::log("Created random ID: " + id, LOG_DEBUG);
        for(int j=0;j<numMessages;j++){
            int msgLength = rand() % 50 + 20;
            QString msg;
            for(int k=0;k<msgLength;k++){
                msg.append((char)(rand()%256));
            }
            Message m;
            m.id = id;
            m.time = time(NULL);
            m.data = msg.toUtf8();
            messages.addMessage(m);
            sleep(1);
            Logger::log("Created message with length " + QString::number(msgLength), LOG_DEBUG);
        }
        Logger::log("Added " + QString::number(numMessages) + " messages for ID " + id, LOG_DEBUG);
    }
}

