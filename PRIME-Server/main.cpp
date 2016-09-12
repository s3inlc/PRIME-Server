/*
 * Author: Sein Coray <s.coray@unibas.ch>
 */

#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#include "masterserver.h"
#include "slaveserver.h"
#include "logger.h"
using namespace std;

bool validIP(QString ip){
    QStringList split = ip.split(".");
    if(split.size() != 4){
        return false;
    }
    for(int x=0;x<split.size();x++){
        int num = split.at(x).toInt();
        if(num < 0 || num > 255){
            return false;
        }
    }
    return true;
}

void help(){
    cout << "PRIME Server v1.0.0" << endl << endl;
    cout << "-h\tShow help" << endl;
    cout << "-m\tServer should start in master-mode" << endl;
    cout << "-s\tMaster IP to connect to when running in slave mode" << endl;
    cout << "-p\tPort where the master service is running, either where the" << endl;
    cout << "\tslave connects to, or the master server is listening on." << endl;
    cout << "\tDefault port is 6666." << endl;
    cout << "-c\tPort number where the client can connect to, is used in both" << endl;
    cout << "\tmaster and slave mode. Default port is 6667." << endl;
    cout << "-i\tSpecifies the IP where the slave server should listening on." << endl;
    cout << "\tThis needs to be the IP where the slaves can connect to (public" << endl;
    cout << "\tIP if server is running behind a firewall." << endl;
    cout << "-d\tSet log level to DEBUG" << endl;
    cout << "-l\tSet log level (LOG_DEBUG, LOG_INFO, LOG_NORMAL, LOG_ERROR, LOG_NOTHING)" << endl;
    cout << "-o\tSet the log mode (LOGMODE_NOTHING, LOGMODE_FILE, LOGMODE_STDOUT, LOGMODE_BOTH)" << endl;
    cout << "-e\tExternal port of a slave server. This is needed in case the" << endl;
    cout << "\tslave server is running behind a firewall and the clientport" << endl;
    cout << "\tdiffers from the public port on the public ip." << endl << endl;
}

void errorHelp(){
    cout << "PRIME Server v1.0.0" << endl;
    cout << "Use -h to show help for command line arguments" << endl << endl;
}

int main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);

    //get command line arguments
    QString ip, own;
    bool isMaster = false;
    char c;
    int port = 6666;
    int clientport = 6667;
    int externPort = 0;
    LogLevel l;
    LogMode m;
    while ((c = getopt (argc, argv, "ms:p:c:i:e:hdl:o:")) != -1){
        switch (c){
            case 'm':
                isMaster = true;
                break;
            case 'd':
                Logger::setLevel(LOG_DEBUG);
                break;
            case 'o':
                m = Logger::parseMode(optarg);
                if(m == LOGMODE_INVALID){
                    cerr << "Invalid log mode!" << endl;
                    errorHelp();
                    return 1;
                }
                Logger::setMode(m);
                break;
            case 'l':
                l = Logger::parseLevel(optarg);
                if(l == LOG_INVALID){
                    cerr << "Invalid log level!" << endl;
                    errorHelp();
                    return 1;
                }
                Logger::setLevel(l);
                break;
            case 'e':
                externPort = QString(optarg).toInt();
                if(externPort < 1 || externPort > 65535){
                    cerr << "External port number must be from 1 to 65535!" << endl;
                    errorHelp();
                    return 1;
                }
                break;
            case 's':
                ip = optarg;
                if(!validIP(ip)){
                    cerr << "Invalid master server IP!" << endl;
                    errorHelp();
                    return 1;
                }
                break;
            case 'p':
                port = QString(optarg).toInt();
                if(port < 1 || port > 65535){
                    cerr << "Port number must be from 1 to 65535!" << endl;
                    errorHelp();
                    return 1;
                }
                break;
            case 'i':
                own = optarg;
                if(!validIP(own)){
                    cerr << "Invalid slave interface IP!" << endl;
                    errorHelp();
                    return 1;
                }
                break;
            case 'c':
                clientport = QString(optarg).toInt();
                if(clientport < 1 || clientport > 65535){
                    cerr << "Client port number must be from 1 to 65535!" << endl;
                    errorHelp();
                    return 1;
                }
                break;
            case 'h':
                help();
                return 0;
            case '?':
                if (optopt == 's' || optopt == 'p' || optopt == 'c'){
                    cerr << "Option -" << (char)optopt << " requires an argument." << endl;
                }
                else if (isprint(optopt)){
                    cerr << "Unknown option `-" << (char)optopt << "'." << endl;
                }
                errorHelp();
                return 1;
            default:
                help();
                return 1;
        }
    }

    if(isMaster){
        Logger::log("Starting master server on port " + QString::number(port) + "...", LOG_NORMAL);
        new MasterServer(port, clientport);
    }
    else{
        if(ip.length() == 0){
            cerr << "In slave mode the master IP:Port is required!" << endl;
            errorHelp();
            return 1;
        }
        Logger::log("Starting slave server on port " + QString::number(port) + " (master " + ip + ")...", LOG_NORMAL);
        new SlaveServer(QHostAddress(ip), port, QHostAddress(own), clientport, externPort);
    }

    return a.exec();
}
