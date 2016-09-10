/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#include "slaveclientprotocolv1.h"

SlaveClientProtocolV1::SlaveClientProtocolV1(QTcpSocket *sock, Messages *msg, QObject *parent) : QObject(parent){
    messages = msg;
    connectionIsValid = true;
    socket = sock;
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    Logger::log("Slave-Client Protocol: Initialized for " + socket->peerAddress().toString(), LOG_INFO);
}

SlaveClientProtocolV1::~SlaveClientProtocolV1(){
    delete socket;
}

bool SlaveClientProtocolV1::isValid(){
    return connectionIsValid;
}

void SlaveClientProtocolV1::readyRead(){
    if(!connectionIsValid){
        Logger::log("Slave-Client Protocol: Got data even if connection is not valid!", LOG_DEBUG);
        return;
    }
    QByteArray data = socket->readLine().replace("\r", "\n").replace("\n", "");
    Logger::log("Slave-Client Protocol: Received data from " + socket->peerAddress().toString() + ": " + QString::number(data.size()) + " Bytes", LOG_DEBUG);
    Logger::log("Slave-Client Protocol: " + data, LOG_DEBUG);
    QList<QByteArray> content = data.split(':');
    QString command = content.at(0);
    if(command.compare("GTMSG") == 0){
        callGetMessages(content);
    }
    else{
        Logger::log("Slave-Client Protocol: Unknown command from client: " + command, LOG_ERROR);
        Logger::log("Slave-Client Protocol: Faulty client connection is " + socket->peerAddress().toString(), LOG_INFO);
        socket->write(QString("ERROR:Invalid command!\n").toUtf8());
    }
}

void SlaveClientProtocolV1::socketError(QAbstractSocket::SocketError err){
    if(err == QTcpSocket::RemoteHostClosedError){
        Logger::log("Slave-Client Protocol: Client closed connection: " + socket->peerAddress().toString(), LOG_INFO);
    }
    else{
        Logger::log("Slave-Client Protocol: Socket error code " + QString::number(err) + " from " + socket->peerAddress().toString(), LOG_NORMAL);
    }
    Logger::log("Slave-Client Protocol: Closed socket to " + socket->peerAddress().toString(), LOG_DEBUG);
    socket->close();
    connectionIsValid = false;
}

void SlaveClientProtocolV1::callGetMessages(QList<QByteArray> content){
    Logger::log("Slave-Client Protocol: " + socket->peerAddress().toString() + " called GTMSG", LOG_DEBUG);
    QStringList ids = QString(content.at(1)).replace("{", "").replace("}", "").split(";");
    QByteArray data = messages->getData(ids);
    socket->write(QString("GTMSG:OK:" + data.toBase64() + "\n").toUtf8());
}

