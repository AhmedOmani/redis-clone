#pragma once
#include "HashTable.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <set>

#define MAX_BUFFER_SIZE 16384

class Connection {
private:
    int fd;
    int cursor{0};
    std::string readBuffer;
    std::string writeBuffer;
    
    HashTable* db;
    void processBuffer();
    

public:
    std::set<int> st;
    std::function<void(int)> onClose;
    Connection(int fd , HashTable* db);
    void handleRead();
    void handleCommand(std::vector<std::string> tokens);
    std::vector<std::string> parse();
};