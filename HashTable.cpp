#include "HashTable.hpp"
#include <iostream>
#include <cstdint>

using namespace std;

HashTable::HashTable(uint64_t size) : buckets(size) {
    table = new KvNode[buckets];
    // KvNode default initializer already sets all keys/values to 0 (our sentinel)
}

HashTable::~HashTable() {
    delete[] table;
}

uint64_t HashTable::hash_fnv1a(const string& key) {
    uint64_t hash = 14695981039346656037ULL;
    uint64_t prime = 1099511628211ULL;
    for (char c : key) {
        hash ^= c;
        hash *= prime;
    }
    return hash ;
}

InsertResult HashTable::insert(const string& key , string* value) {
    uint64_t hashKey = hash_fnv1a(key);
    cout << "hashKey from insert: " << hashKey << endl;
    uint64_t index = hashKey % buckets;

    for (int i = 0 ; i < 4 ; i++) {
        if (table[index].key[i] == 0) {            
            if (currentMemoryUsage + value->length() > maxMemoryLimit) {
                cout << "Memory Heap limit exceeded!" << endl;
                return InsertResult::ERR_OOM;
            }
            table[index].key[i] = hashKey;
            table[index].value[i] = value;
            currentMemoryUsage += value->length();
            cout << "Value inserted at index: " << index << endl;
            return InsertResult::OK; 
        }
    }

    uint64_t turnedIndex = (index + 1) % buckets;
    while(turnedIndex != index) {
        for (int i = 0 ; i < 4 ; i++) {
            if(table[turnedIndex].key[i] == 0) {
                if (currentMemoryUsage + value->length() > maxMemoryLimit) {
                    cout << "Memory Heap limit exceeded!" << endl;
                    return InsertResult::ERR_OOM;
                }
                table[turnedIndex].key[i] = hashKey;
                table[turnedIndex].value[i] = value;
                currentMemoryUsage += value->length();
                cout << "Value inserted at index: " << turnedIndex << endl;
                return InsertResult::OK;
            }
        }
        turnedIndex = (turnedIndex + 1) % buckets;
    }

    return InsertResult::ERR_FULL;
}

string* HashTable::search(const string& key) {
    uint64_t hashKey = hash_fnv1a(key);
    cout << "hashKey from search: " << hashKey << endl;
    uint64_t index = hashKey % buckets;

    for (int i = 0 ; i < 4 ; i++) {
        if (table[index].key[i] == hashKey) return table[index].value[i];
        if (table[index].key[i] == 0) return nullptr;
    }

    uint64_t turnedIndex = (index + 1) % buckets;
    while(turnedIndex != index) {
        for (int i = 0 ; i < 4 ; i++) {
            if (table[turnedIndex].key[i] == hashKey) return table[turnedIndex].value[i];
            if (table[turnedIndex].key[i] == 0) return nullptr;
        }
        turnedIndex = (turnedIndex + 1) % buckets;
    }

    cout << "Not found!" << endl;

    return nullptr;
}