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

bool HashTable::insert(const string& key , uint64_t value) {
    uint64_t hashKey = hash_fnv1a(key);
    cout << "hashKey from insert: " << hashKey << endl;
    uint64_t index = hashKey % buckets;

    for (int i = 0 ; i < 4 ; i++) {
        if (table[index].key[i] == 0) {
            table[index].key[i] = hashKey;
            table[index].value[i] = value;
            cout << "Value inserted at index: " << index << endl;
            return true; 
        }
    }

    uint64_t turnedIndex = (index + 1) % buckets;
    while(turnedIndex != index) {
        for (int i = 0 ; i < 4 ; i++) {
            if(table[turnedIndex].key[i] == 0) {
                table[turnedIndex].key[i] = hashKey;
                table[turnedIndex].value[i] = value;
                cout << "Value inserted at index: " << turnedIndex << endl;
                return true;
            }
        }
        turnedIndex = (turnedIndex + 1) % buckets;
    }

    return false;
}

uint64_t HashTable::search(const string& key) {
    uint64_t hashKey = hash_fnv1a(key);
    cout << "hashKey from search: " << hashKey << endl;
    uint64_t index = hashKey % buckets;

    for (int i = 0 ; i < 4 ; i++) {
        if (table[index].key[i] == hashKey) {
            return table[index].value[i];
        }
    }

    uint64_t turnedIndex = (index + 1) % buckets;
    while(turnedIndex != index) {
        for (int i = 0 ; i < 4 ; i++) {
            if(table[turnedIndex].key[i] == hashKey) {
                return table[turnedIndex].value[i];
            }
        }
        turnedIndex = (turnedIndex + 1) % buckets;
    }

    cout << "Not found!" << endl;

    return 0;
}