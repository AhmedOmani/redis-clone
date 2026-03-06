#pragma once
#include "EventLoop.hpp"
#include "Connection.hpp"
#include <vector>
#include <unordered_map>

class Server {
private:
    int port;
    int serverSocket;
    HashTable db{64 * 1024};
    EventLoop eventLoop;
    std::unordered_map<int, Connection*> connections;
    void handleNewConnection();
    void removeClient(int fd);

public:
    Server(int port);
    int setupSocket();
    void start();
};