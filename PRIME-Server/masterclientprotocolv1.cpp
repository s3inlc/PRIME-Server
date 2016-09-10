/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#include "masterclientprotocolv1.h"

MasterClientProtocolV1::MasterClientProtocolV1(QTcpSocket *sock, Messages *msg, QObject *parent) : QObject(parent){
    socket = sock;
    messages = msg;
    connectionIsValid = true;
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    Logger::log("Master-Client Protocol: Initialized for " + socket->peerAddress().toString(), LOG_INFO);
}

MasterClientProtocolV1::~MasterClientProtocolV1(){
    delete socket;
}

void MasterClientProtocolV1::readyRead(){
    if(!connectionIsValid){
        Logger::log("Master-Client Protocol: Got data even if connection is not valid!", LOG_DEBUG);
        return;
    }
    QByteArray data = socket->readLine().replace("\r", "\n").replace("\n", "");
    Logger::log("Master-Client Protocol: Received data from " + socket->peerAddress().toString() + ": " + QString::number(data.size()) + " Bytes", LOG_DEBUG);
    Logger::log("Master-Client Protocol: " + data, LOG_DEBUG);
    QList<QByteArray> content = data.split(':');
    QString command = content.at(0);
    if(command.compare("GTSRV") == 0){
        callGetServers(content);
    }
    else if(command.compare("GTADD") == 0){
        callGetAddresses(content);
    }
    else if(command.compare("NEMSG") == 0){
        callNewMessage(content);
    }
    else if(command.compare("SPING") == 0){
        callPing(content);
    }
    else{
        Logger::log("Master-Client Protocol: Unknown command from client: " + command, LOG_ERROR);
        Logger::log("Master-Client Protocol: Faulty client connection is " + socket->peerAddress().toString(), LOG_INFO);
        socket->write(QString("ERROR:Invalid command!\n").toUtf8());
    }
}

void MasterClientProtocolV1::socketError(QAbstractSocket::SocketError err){
    if(err == QTcpSocket::RemoteHostClosedError){
        Logger::log("Master-Client Protocol: Client closed connection: " + socket->peerAddress().toString(), LOG_INFO);
    }
    else{
        Logger::log("Master-Client Protocol: Socket error code " + QString::number(err) + " from " + socket->peerAddress().toString(), LOG_NORMAL);
    }
    Logger::log("Master-Client Protocol: Closed socket to " + socket->peerAddress().toString(), LOG_DEBUG);
    socket->close();
    connectionIsValid = false;
}

void MasterClientProtocolV1::callGetServers(QList<QByteArray> content){
    Logger::log("Master-Client Protocol: " + socket->peerAddress().toString() + " called GTSRV", LOG_DEBUG);
    QList<SlaveServerConnection> list = getSlaveServers();
    if(list.size() == 0){
        Logger::log("Master-Client Protocol: No slaves sent to client, functionality not given!", LOG_NORMAL);
        socket->write(QString("GTSRV:FAIL:No slave servers available!\n").toUtf8());
        return;
    }
    QString data = "{";
    for(int x=0;x<list.size();x++){
        data += list.at(x).address.toString() + "," + QString::number(list.at(x).port);
        if(x < list.size() - 1){
            data += ";";
        }
    }
    data += "}";
    content.clear();
    Logger::log("Master-Client Protocol: " + socket->peerAddress().toString() + " got " + QString::number(list.size()) + " servers", LOG_DEBUG);
    socket->write(QString("GTSRV:" + data + "\n").toUtf8());
}

void MasterClientProtocolV1::callGetAddresses(QList<QByteArray> content){
    Logger::log("Master-Client Protocol: " + socket->peerAddress().toString() + " called GTADD", LOG_DEBUG);
    QList<QString> list = messages->getAllIds();
    if(list.size() == 0){
        socket->write(QString("GTADD:FAIL:No ids available!\n").toUtf8());
        return;
    }
    QString data = "{";
    for(int x=0;x<list.size();x++){
        data += list.at(x);
        if(x < list.size() - 1){
            data += ";";
        }
    }
    data += "}";
    content.clear();
    Logger::log("Master-Client Protocol: " + socket->peerAddress().toString() + " got " + QString::number(list.size()) + " addresses", LOG_DEBUG);
    socket->write(QString("GTADD:" + data + "\n").toUtf8());
}

void MasterClientProtocolV1::callNewMessage(QList<QByteArray> content){
    Logger::log("Master-Client Protocol: " + socket->peerAddress().toString() + " called NEMSG", LOG_DEBUG);
    if(content.size() != 3){
        Logger::log("Master-Client Protocol: Didn't accept message from " + socket->peerAddress().toString() + " due to invalid syntax", LOG_INFO);
        socket->write(QString("NEMSG:FAIL:Invalid command syntax!\n").toUtf8());
        return;
    }
    Message m;
    m.id = content.at(1);
    m.time = time(0);
    m.data = QByteArray::fromBase64(content.at(2));
    messages->addMessage(m);
    emit newMessage(m);
    Logger::log("Master-Client Protocol: Handled new message from " + socket->peerAddress().toString(), LOG_DEBUG);
    socket->write(QString("NEMSG:OK\n").toUtf8());
}

void MasterClientProtocolV1::callPing(QList<QByteArray> content){
    Logger::log("Master-Client Protocol: " + socket->peerAddress().toString() + " called SPING", LOG_DEBUG);
    if(content.size() != 2){
        socket->write(QString("SPING:FAIL:Command syntax not matching!\n").toUtf8());
        return;
    }
    lastPing = time(0);
    socket->write(QString("SPING:" + content.at(1) + "\n").toUtf8());
}

bool MasterClientProtocolV1::isValid(){
    return connectionIsValid;
}

