#pragma once
#include "HashTable.hpp"
#include <string>
#include <string_view>
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
    std::vector<std::string_view> tokens;
    HashTable* db;
    void processBuffer();
    

public:
    std::set<int> st;
    std::function<void(int)> onClose;
    Connection(int fd , HashTable* db);
    void handleRead();
    void handleCommand(const std::vector<std::string_view>& tokens);
    std::vector<std::string_view> parse();
};