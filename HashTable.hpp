#pragma once
#include <cstdint>
#include <string>

enum class InsertResult {
    OK,
    ERR_OOM,
    ERR_FULL
};

class HashTable {
private:
    struct alignas(64) KvNode {
        uint64_t key[4] = {0, 0, 0, 0};
        std::string* value[4] = {nullptr, nullptr, nullptr, nullptr};
    };

    KvNode* table;
    uint64_t buckets;
    uint64_t maxMemoryLimit{1024 * 1024 * 1024};
    uint64_t currentMemoryUsage{};

    uint64_t hash_fnv1a(const std::string &key);

public:
    HashTable() = default;
    HashTable(uint64_t size);
    ~HashTable();
    InsertResult insert(const std::string &key, std::string*value);
    std::string* search(const std::string &key);
    bool remove(const std::string &key);
};