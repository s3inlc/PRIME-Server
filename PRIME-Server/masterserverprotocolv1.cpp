/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#include "masterserverprotocolv1.h"

MasterServerProtocolV1::MasterServerProtocolV1(QTcpSocket *sock, Messages *msg, QObject *parent) : QObject(parent){
    state = MSSINIT;
    socket = sock;
    messages = msg;
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    state = MSSVALID;
    lastPing = time(0);
    Logger::log("Master-Server Protocol: Initialized for " + socket->peerAddress().toString(), LOG_DEBUG);
}

MasterServerProtocolV1::~MasterServerProtocolV1(){
    delete socket;
}

bool MasterServerProtocolV1::isValid(){
    if(time(0) - lastPing > PING_TIMEOUT){
        Logger::log("Master-Server Protocol: Connection to " + socket->peerAddress().toString() + " got ping timeout", LOG_INFO);
        return false;
    }
    else if(state <= MSSREADY){
        return true;
    }
    return false;
}

bool MasterServerProtocolV1::isReady(){
    if(state == MSSREADY){
        return true;
    }
    return false;
}

SlaveServerConnection MasterServerProtocolV1::getServer(){
    return server;
}

void MasterServerProtocolV1::readyRead(){
    QByteArray data = socket->readLine().replace("\r", "\n").replace("\n", "");
    Logger::log("Master-Server Protocol: Received data from " + socket->peerAddress().toString() + ": " + QString::number(data.size()) + " Bytes", LOG_DEBUG);
    Logger::log("Master-Server Protocol: " + data, LOG_DEBUG);
    QList<QByteArray> content = data.split(':');
    QString command = content.at(0);
    if(command.compare("REGIS") == 0){
        callRegister(content);
    }
    else if(command.compare("DIFFM") == 0){
        callDiffMessages(content);
    }
    else if(command.compare("SPING") == 0){
        callPing(content);
    }
    else if(command.compare("NEMSG") == 0){
        callNewMessage(content);
    }
    else{
        Logger::log("Master-Server Protocol: Unknown command from slave: " + command, LOG_ERROR);
        Logger::log("Master-Server Protocol: Faulty slave connection is " + socket->peerAddress().toString(), LOG_INFO);
        socket->write(QString("ERROR:Invalid command!\n").toUtf8());
    }
}

void MasterServerProtocolV1::socketError(QAbstractSocket::SocketError err){
    if(err == QTcpSocket::RemoteHostClosedError){
        Logger::log("Master-Server Protocol: Slave closed connection: " + socket->peerAddress().toString(), LOG_INFO);
    }
    else{
        Logger::log("Master-Server Protocol: Socket error code " + QString::number(err) + " from " + socket->peerAddress().toString(), LOG_NORMAL);
    }
    Logger::log("Master-Server Protocol: Closed socket to " + socket->peerAddress().toString(), LOG_DEBUG);
    socket->close();
    state = MSSERR;
}

void MasterServerProtocolV1::issueNewMessage(Message m){
    socket->write(QString("NEMSG:" + m.id + ":" + QString::number(m.time) + ":" + m.data.toBase64() + "\n").toUtf8());
}

void MasterServerProtocolV1::callRegister(QList<QByteArray> content){
    //slave wants to register to master
    Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " called REGIS", LOG_DEBUG);
    if(state != MSSVALID){
        Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " has invalid state!", LOG_INFO);
        socket->write(QString("REGIS:FAIL:Server Status does not match!\n").toUtf8());
        return;
    }
    else if(content.size() != 3){
        Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " used invalid syntax!", LOG_INFO);
        socket->write(QString("REGIS:FAIL:Command syntax not matching!\n").toUtf8());
        return;
    }
    server.address = QHostAddress(QString(content.at(1)));
    if(server.address.isNull()){
        Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " provided an invalid address!", LOG_INFO);
        socket->write(QString("REGIS:FAIL:Provided address is invalid!\n").toUtf8());
        return;
    }
    server.port = content.at(2).toInt();
    if(server.port < 1 || server.port > 65535){
        Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " provided an invalid port!", LOG_INFO);
        socket->write(QString("REGIS:FAIL:Provided port is invalid!\n").toUtf8());
        return;
    }
    socket->write(QString("REGIS:OK\n").toUtf8());
    Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " registered successfully!", LOG_INFO);
    state = MSSREGISTERED;
}

void MasterServerProtocolV1::callDiffMessages(QList<QByteArray> content){
    Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " called DIFFM", LOG_DEBUG);
    if(state != MSSREGISTERED){
        Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " has invalid state!", LOG_INFO);
        socket->write(QString("DIFFM:FAIL:Server Status does not match!\n").toUtf8());
        return;
    }
    QList<Message> batch = messages->getMessageBatch(content.at(1).toInt());
    if(batch.size() == 0){
        //fully updated
        state = MSSREADY;
        Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " synced all messages", LOG_INFO);
        socket->write(QString("DIFFM:DONE\n").toUtf8());
        return;
    }
    QString data = "{";
    for(int x=0;x<batch.size();x++){
        Message m = batch.at(x);
        data += m.id + "," + QString::number(m.time) + "," + m.data.toBase64();
        if(x < batch.size() - 1){
            data += ";";
        }
    }
    data += "}";
    Logger::log("Master-Server Protocol: " + QString::number(batch.size()) + " message updates sent to " + socket->peerAddress().toString(), LOG_DEBUG);
    socket->write(QString("DIFFM:" + data + "\n").toUtf8());
}

void MasterServerProtocolV1::callPing(QList<QByteArray> content){
    Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " called SPING", LOG_DEBUG);
    if(content.size() != 2){
        socket->write(QString("SPING:FAIL:Command syntax not matching!\n").toUtf8());
        return;
    }
    lastPing = time(0);
    socket->write(QString("SPING:" + content.at(1) + "\n").toUtf8());
}

void MasterServerProtocolV1::callNewMessage(QList<QByteArray> content){
    Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " called NEMSG", LOG_DEBUG);
    if(content.size() < 2){
        return;
    }
    else if(QString(content.at(1)).compare("OK") == 0){
        //everything is ok
        Logger::log("Master-Server Protocol: Slave " + socket->peerAddress().toString() + " confirmed message", LOG_DEBUG);
    }
    else{
        Logger::log("Master-Server Protocol: " + socket->peerAddress().toString() + " failed to receive new message, state degraded!", LOG_INFO);
        state = MSSREGISTERED;
        socket->write(QString("REGIS:OK\n").toUtf8());
    }
}

