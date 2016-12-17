/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#include "slaveserverprotocolv1.h"

SlaveServerProtocolV1::SlaveServerProtocolV1(QHostAddress addr, int port, QHostAddress clientAddr, int clientPort, Messages *msg, int extPort) : QObject(){
    state = SSSINIT;
    socket = NULL;
    conn.address = addr;
    conn.port = port;
    externalPort = extPort;
    clientConn.address = clientAddr;
    clientConn.port = clientPort;
    messages = msg;
    lastPing = 0;
    pingTimer.setInterval(5000);
    pingTimer.connect(&pingTimer, SIGNAL(timeout()), this, SLOT(doPing()));
    pingTimer.start();
    connectToMaster();
    Logger::log("Slave-Server Protocol: Initialized", LOG_DEBUG);
}

SlaveServerProtocolV1::~SlaveServerProtocolV1(){
    delete socket;
}

bool SlaveServerProtocolV1::isValid(){
    if(state == SSSREADY){
        return true;
    }
    return false;
}

void SlaveServerProtocolV1::readyRead(){
    QByteArray data = socket->readLine().replace("\r", "\n").replace("\n", "");
    Logger::log("Slave-Server Protocol: Received data from " + socket->peerAddress().toString() + ": " + QString::number(data.size()) + " Bytes", LOG_DEBUG);
    Logger::log("Slave-Server Protocol: " + data, LOG_DEBUG);
    QList<QByteArray> content = data.split(':');
    QString command = content.at(0);
    if(command.compare("SPING") == 0){
        Logger::log("Slave-Server Protocol: " + socket->peerAddress().toString() + " called SPING", LOG_DEBUG);
        lastPing = content.at(1).toInt();
    }
    else if(command.compare("DIFFM") == 0 && state == SSSUPDATE){
        callDiffMessages(content);
    }
    else if(command.compare("NEMSG") == 0 && state == SSSREADY){
        callNewMessage(content);
    }
    else if(command.compare("REGIS") == 0 && state == SSSREGISTER){
        if(QString(content.at(1)).compare("OK") == 0){
            state = SSSUPDATE;
            Logger::log("Slave-Server Protocol: Successfully registered to master!", LOG_NORMAL);
            socket->write(QString("DIFFM:" + QString::number(messages->getNewestTime()) + "\n").toUtf8());
        }
        else{
            Logger::log("Slave-Server Protocol: Failed to register to master server: " + content.at(2), LOG_ERROR);
            Logger::log("Slave-Server Protocol: Reconnecting in 5 seconds...", LOG_NORMAL);
            QTimer::singleShot(5000, this, SLOT(connectionEstablished()));
        }
    }
    else{
        Logger::log("Slave-Server Protocol: Unknown/Invalid command (" + command + ") from " + socket->peerAddress().toString(), LOG_ERROR);
    }
}

void SlaveServerProtocolV1::connectToMaster(){
    state = SSSINIT;
    if(socket != NULL){
        Logger::log("Slave-Server Protocol: Reconnecting...", LOG_INFO);
    }
    else{
        socket = new QTcpSocket();
        connect(socket, SIGNAL(connected()), this, SLOT(connectionEstablished()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
        connect(socket, SIGNAL(disconnected()), this, SLOT(isDisconnected()));
    }
    Logger::log("Slave-Server Protocol: Start connection...", LOG_DEBUG);
    socket->connectToHost(conn.address, conn.port);
}

void SlaveServerProtocolV1::socketError(QAbstractSocket::SocketError err){
    if(err == QAbstractSocket::ConnectionRefusedError){
        Logger::log("Slave-Server Protocol: Connection refused, reconnection in 2 seconds!", LOG_INFO);
        connectToMaster();
    }
    Logger::log("Slave-Server Protocol: Connection error! (" + QString(err) + ")", LOG_NORMAL);
}

void SlaveServerProtocolV1::connectionEstablished(){
    state = SSSREGISTER;
    Logger::log("Slave-Server Protocol: Connection to master established successfully!", LOG_NORMAL);
    QString pp = QString::number(clientConn.port);
    if(externalPort > 0){
        pp = QString::number(externalPort);
    }
    socket->write(QString("REGIS:" + clientConn.address.toString() + ":" + pp + "\n").toUtf8());
}

void SlaveServerProtocolV1::doPing(){
    if(socket->isOpen()){
        socket->write(QString("SPING:" + QString::number(time(0)) + "\n").toUtf8());
    }
}

void SlaveServerProtocolV1::isDisconnected(){
    Logger::log("Slave-Server Protocol: Connection to master is disconnected, reconnecting...", LOG_INFO);
    connectToMaster();
}

void SlaveServerProtocolV1::callDiffMessages(QList<QByteArray> content){
    Logger::log("Slave-Server Protocol: " + socket->peerAddress().toString() + " sent DIFFM", LOG_DEBUG);
    if(QString(content.at(1)).compare("DONE") == 0){
        //messages are fully updated now
        Logger::log("Slave-Server Protocol: Messages are synced completely!", LOG_NORMAL);
        state = SSSREADY;
        return;
    }
    QStringList data = QString(content.at(1)).replace("{", "").replace("}", "").split(";");
    int count = 0;
    for(int x=0;x<data.size();x++){
        if(data.at(x).length() == 0){
            continue;
        }
        QStringList m = data.at(x).split(",");
        if(m.size() != 3){
            Logger::log("Slave-Server Protocol: Invalid message from server!", LOG_ERROR);
            continue;
        }
        Message msg;
        msg.id = m.at(0);
        msg.time = m.at(1).toInt();
        msg.data = QByteArray::fromBase64(m.at(2).toUtf8());
        messages->addMessage(msg);
        count++;
    }
    Logger::log("Received " + QString::number(count) + " message updates", LOG_INFO);
    socket->write(QString("DIFFM:" + QString::number(messages->getNewestTime()) + "\n").toUtf8());
}

void SlaveServerProtocolV1::callNewMessage(QList<QByteArray> content){
    Logger::log("Slave-Server Protocol: " + socket->peerAddress().toString() + " sent NEMSG", LOG_DEBUG);
    if(content.size() != 4){
        Logger::log("Slave-Server Protocol: Invalid message from server!", LOG_ERROR);
        socket->write(QString("NEMSG:FAIL:Invalid message\n").toUtf8());
        state = SSSREGISTER;
        return;
    }
    Message m;
    m.id = content.at(1);
    m.time = content.at(2).toInt();
    m.data = QByteArray::fromBase64(content.at(3));
    messages->addMessage(m);
    socket->write(QString("NEMSG:OK\n").toUtf8());
}

