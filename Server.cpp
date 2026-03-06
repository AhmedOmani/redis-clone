#include "Server.hpp"
#include "Utils.hpp"
#include "HashTable.hpp"
#include <sys/epoll.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <chrono>
#include <cassert>
#include <utility>
#include <list>
#include <memory>

using namespace std;

Server::Server(int port) {
    this->port = port;
    int serverSocket = setupSocket();
    eventLoop.addEvent(serverSocket , EPOLLIN | EPOLLET);
}

void Server::handleNewConnection() {
    while (true) {
        cout << "handleNewConnection called" << endl;
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        int clientSocket = accept(serverSocket , (struct sockaddr*)&clientAddress , &clientAddressLength);
        if (clientSocket == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            continue;
        }

        makeNonBlocking(clientSocket);
        
        auto connection = new Connection(clientSocket , &db);
        connection->onClose = [this](int fd) { removeClient(fd); };
        connections[clientSocket] = connection;

        if (!eventLoop.addEvent(clientSocket , EPOLLIN | EPOLLET)) {
            close(clientSocket);
            continue;
        }
        cout << "Cliene connection added" << endl;
    }
}
int Server::setupSocket() {
    serverSocket = socket(AF_INET, SOCK_STREAM , 0);
    if (serverSocket == -1) {
        cout << "Failed to create server socket" << endl;
        exit(1);
    }

    int option = 1;
    setsockopt(serverSocket , SOL_SOCKET , SO_REUSEADDR | SO_REUSEPORT , &option , sizeof(option)) ;
   
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket , (sockaddr*)&serverAddress , sizeof(serverAddress));
    listen(serverSocket , SOMAXCONN) ;

    makeNonBlocking(serverSocket);

    return serverSocket;
}

void Server::removeClient(int fd) {
    connections.erase(fd);
    close(fd);
}

void Server::start() {
    cout << "Redis TCP Server started , entering EventLoop..." << endl;

    std::vector<epoll_event> activeEvents;
    while (true) {
        cout << "waiting for events" << endl;
        int numEvents = eventLoop.wait(activeEvents);
        cout << "numEvents: " << numEvents << endl;

        for (int i = 0 ; i < numEvents ; i++) {
            if (activeEvents[i].data.fd == serverSocket) {
                handleNewConnection();
            }
            
            else {
                cout << "FD: " << activeEvents[i].data.fd << " is ready to read" << endl;
                connections[activeEvents[i].data.fd]->handleRead();
            } 
        }
    }
}