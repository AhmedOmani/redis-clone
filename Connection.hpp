#pragma once
#include "HashTable.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>

class Connection {
private:
    int fd;
    std::string buffer;
    HashTable* db;
    

public:
    std::function<void(int)> onClose;
    Connection(int fd , HashTable* db);
    void handleRead();
    void redisClient(std::string data , int n);
    std::vector<std::string> parse(std::string data);
};