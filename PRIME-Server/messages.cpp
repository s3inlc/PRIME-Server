/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#include "messages.h"

Messages::Messages(QObject *parent) : QObject(parent){
    path = "./";
    newestTime = 0;
    clean();
    cleanTimer.setInterval(60000);
    connect(&cleanTimer, SIGNAL(timeout()), this, SLOT(clean()));
    cleanTimer.start();
}

QList<Message> Messages::getMessages(int pos){
    QString id = getIdForPos(pos);
    QList<Message> messages;
    int t = time(0);
    QDir receiver(path + MSG_FOLDER + id);
    if(!receiver.exists()){
        return messages;
    }
    QStringList entries = receiver.entryList();
    for(int x=0;x<entries.size();x++){
        if(entries.at(x).length() == 0 || entries.at(x)[0] == '.'){
            continue;
        }
        QStringList split = entries.at(x).split("-");
        QFile m(receiver.absolutePath() + "/" + entries.at(x));
        m.open(QIODevice::ReadOnly);
        if(!m.isOpen()){
            continue;
        }
        Message msg;
        msg.id = id;
        msg.time = split.at(1).toInt();
        msg.data = m.readAll();
        if(t - msg.time > MSG_TIMEOUT){
            continue;
        }
        messages.append(msg);
    }
    bool isSorted = false;
    while(!isSorted){
        isSorted = true;
        for(int x=0;x<messages.size() - 1;x++){
            if(messages.at(x).time > messages.at(x + 1).time){
                Message m = messages.at(x);
                messages.replace(x, messages.at(x+1));
                messages.replace(x+1, m);
                isSorted = false;
            }
        }
    }
    return messages;
}

/**
 * @brief Messages::addMessage
 * Add a message to this server with the given parameters. This message will then be written to file system.
 * @param msg
 * @return
 */
bool Messages::addMessage(Message msg){
    QDir receiver(path + MSG_FOLDER + msg.id);
    if(!receiver.exists()){
        QDir(path + MSG_FOLDER).mkdir(msg.id);
    }
    if(!knownIds.contains(msg.id)){
        knownIds.append(msg.id);
    }

    unsigned int checksum = qChecksum(msg.data.constData(), msg.data.length());
    QFile file(receiver.absolutePath() + "/" + QString::number(checksum) + "-" + QString::number(msg.time));
    if(file.exists()){
        return true;
    }
    file.open(QIODevice::WriteOnly);
    if(!file.isOpen()){
        cerr << "Failed to write message to " << file.fileName().toStdString() << endl;
        return false;
    }
    int written = file.write(msg.data);
    if(written != msg.data.size()){
        cerr << "Failed to write message data! " << file.fileName().toStdString() << endl;
        return false;
    }
    else{
        MessageIndex ind;
        ind.path = file.fileName();
        ind.time = msg.time;
        index.append(ind);
    }
    if(msg.time > newestTime){
        newestTime = msg.time;
    }
    return true;
}

QByteArray Messages::getData(QString ids){
    QList<QByteArray> data;
    int longest = 0;
    for(int x=0;x<ids.length();x++){
        if(ids.at(x) == '0'){
            continue;
        }
        QList<Message> msg = getMessages(x);
        QByteArray stream;
        for(int y=0;y<msg.size();y++){
            stream += msg.at(y).data;
        }
        if(stream.size() > longest){
            longest = stream.size();
        }
        data.append(stream);
    }
    QByteArray final;
    for(int x=0;x<longest;x++){
        unsigned int val = 0;
        for(int y=0;y<data.size();y++){
            if(x < data.at(y).size()){
                val = (val + (unsigned int)data.at(y).at(x))%256;
            }
        }
        final.append((char)val);
    }
    return final;
}

QList<Message> Messages::getMessageBatch(int time){
    QList<Message> list;
    for(int x=0;x<index.size();x++){
        if(index.at(x).time > time){
            QFile file(index.at(x).path);
            file.open(QIODevice::ReadOnly);
            Message m;
            m.data = file.readAll();
            m.id = getId(file.fileName());
            m.time = getTime(file.fileName());
            list.append(m);
        }
        if(list.size() > MSG_LIST_LIMIT && x < index.size() - 1 && index.at(x).time != index.at(x + 1).time){
            break;
        }
    }
    return list;
}

QList<QString> Messages::getAllIds(){
    return knownIds;
}

int Messages::getNewestTime(){
    return newestTime;
}

QString Messages::getIdForPos(int pos){
    QList<QString> ids = getAllIds();
    sort(ids.begin(), ids.end());
    return ids.at(pos);
}

QString Messages::getId(QString fullPath){
    QStringList split = fullPath.split("/");
    if(split.size() < 2){
        return "";
    }
    return split.at(split.size() - 2);
}

int Messages::getTime(QString fullPath){
    QStringList split1 = fullPath.split("/");
    QStringList split2 = split1.last().split("-");
    return split2.at(1).toInt();
}

/**
 * @brief Messages::clean
 * Goes trough all messages and directories and check which messages can be deleted due to age and updates the known IDs of receivers.
 */
void Messages::clean(){
    //Clean old messages
    QDir dir(path);
    QDir msg(path + "msg");
    if(!msg.exists()){
        dir.mkdir("msg");
    }
    QStringList entries = msg.entryList(QDir::Dirs);
    for(int x=0;x<entries.size();x++){
        if(entries.at(x).length() == 0 || entries.at(x)[0] == '.'){
            continue;
        }
        QDir entry(msg.absolutePath() + "/" + entries.at(x));

        //scan for messages which can be deleted
        QStringList messages = entry.entryList();
        int count = messages.size();
        for(int y=0;y<messages.size();y++){
            if(messages.at(y).length() == 0 || messages.at(y)[0] == '.'){
                count--;
                continue;
            }
            QStringList split = messages.at(y).split("-");
            if(time(0) - split.at(1).toInt() > MSG_TIMEOUT){
                //message can be deleted
                count--;
                QFile file(entry.absolutePath() + "/" + messages.at(y));
                if(!file.remove()){
                    cerr << "Failed to delete message! " << file.fileName().toStdString() << endl;
                }
            }
            else{
                MessageIndex ind;
                QFile file(entry.absolutePath() + "/" + messages.at(y));
                ind.path = file.fileName();
                ind.time = split.at(1).toInt();
                if(ind.time > newestTime){
                    newestTime = ind.time;
                }
                for(int i=0;i<index.size();i++){
                    if(ind.time < index.at(i).time){
                        index.insert(i, ind);
                        break;
                    }
                }
                if(index.size() == 0 || index.last().time <= ind.time){
                    index.append(ind);
                }
            }
        }
        if(count == 0){ //directory is empty, we can delete this id
            entry.removeRecursively();
            knownIds.removeAll(entries.at(x)); // remove when id is deleted
        }
        else if(!knownIds.contains(entries.at(x))){
            knownIds.append(entries.at(x)); // add new id
        }
    }
    Logger::log(QString::number(index.size()) + " messages indexed, " + QString::number(knownIds.size()) + " known IDs", LOG_DEBUG);
}

