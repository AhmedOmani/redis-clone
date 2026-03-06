#pragma once
#include <cstdint>
#include <string>

class HashTable {
private:
    struct alignas(64) KvNode {
        uint64_t key[4] = {0, 0, 0, 0};
        uint64_t value[4] = {0, 0, 0, 0};
    };

    KvNode* table;
    uint64_t buckets;

    uint64_t hash_fnv1a(const std::string &key);

public:
    HashTable() = default;
    HashTable(uint64_t size);
    ~HashTable();
    bool insert(const std::string &key, uint64_t value);
    uint64_t search(const std::string &key);
    bool remove(const std::string &key);
};