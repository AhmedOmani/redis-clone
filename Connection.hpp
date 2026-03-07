#pragma once
#include "HashTable.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>

#define MAX_BUFFER_SIZE 16384

class Connection {
private:
    int fd;
    int cursor{0};
    std::string buffer;
    HashTable* db;
    void processBuffer();
    

public:
    std::function<void(int)> onClose;
    Connection(int fd , HashTable* db);
    void handleRead();
    void handleCommand(std::vector<std::string> tokens);
    std::vector<std::string> parse();
};