#include "Connection.hpp"
#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>


using namespace std;

Connection::Connection(int fd, HashTable* db) : fd(fd), db(db) {}

//We will handle redis data chunks here
void Connection::handleRead() {    
    char buf[1024];
    int bytesRead = 0;
    while(true) {
        int n = read(fd, buf, sizeof(buf) - 1);
        bytesRead += (n > 0 ? n : 0);
        if (n <= 0) {
            if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) break;
            onClose(fd);
            return;
        }
        buf[n] = '\0';
        readBuffer.append(buf, n);
    }

    if (!readBuffer.empty()) {
        processBuffer();
    }
}

void Connection::processBuffer() {
    while(cursor < readBuffer.length()) {
        
        vector<string_view> tokens = parse();
        
        if (tokens.empty()) break;

        handleCommand(tokens);
    }

    if (!writeBuffer.empty()) {
        //cout << "writeBuffer: " << writeBuffer << endl;
        send(fd , writeBuffer.c_str(), writeBuffer.length() , 0);
        writeBuffer.clear();
    }

    if (cursor == readBuffer.length()) { // O(1)
        readBuffer.clear();
        cursor = 0;
    }
    else if (cursor > MAX_BUFFER_SIZE) {
        readBuffer.erase(0 , cursor);
        cursor = 0;
    }
}

void Connection::handleCommand(const vector<string_view>& tokens) {

    if (tokens[0] == "SET") {
        if (tokens.size() < 3) {
            writeBuffer += "-ERR wrong number of arguments for 'set' command\r\n";
            return;
        }
        string key = string(tokens[1]);
        string* value = new string(tokens[2]);

        InsertResult result = db->insert(key , value);

        if (result == InsertResult::OK) {
            writeBuffer += "+OK\r\n";
        }
        else if (result == InsertResult::ERR_OOM) {
            string err = "-OOM command not allowed when used memory > maxmemory\r\n";
            delete value;
            writeBuffer += err;
        }
        else if (result == InsertResult::ERR_FULL) {
            string err = "-ERR hash table is full\r\n";
            delete value;
            writeBuffer += err;
        }
        return;
    }
    if (tokens[0] == "GET") {
        if (tokens.size() < 2) {
            writeBuffer += "-ERR wrong number of arguments for 'get' command\r\n";
            return;
        }
        string key = string(tokens[1]);
        string* value = db->search(key);
        if (value == nullptr) {
            writeBuffer += "$-1\r\n";
            return;
        }
        writeBuffer += "$" + to_string(value->length()) + "\r\n" + *value + "\r\n";
        return;
    }
    writeBuffer += "+OK\r\n";
}

vector<string_view> Connection::parse() {
    int savedCursor = cursor;
    if (readBuffer[cursor] == '*') {
        //clear the vector
        tokens.clear();
        
        cursor++;
        if (cursor >= readBuffer.length()) {
            cursor = savedCursor;
            return {};
        }

        int arraySize = 0;
        while(cursor < readBuffer.length() && readBuffer[cursor] != '\r') {
            arraySize = arraySize * 10 + (readBuffer[cursor] - '0');
            cursor++;
            if (cursor >= readBuffer.length()) {
                cursor = savedCursor;
                return {};
            }
        }

        cursor += 2; // skip \r\n after the array size
        if (cursor > readBuffer.length()) {
            cursor = savedCursor;
            return {};
        }

        while (cursor < readBuffer.length() && tokens.size() < arraySize) {
            if (readBuffer[cursor] == '$') {
                cursor++;
                if (cursor >= readBuffer.length()) {
                    cursor = savedCursor ;
                    return {};
                }

                int len = 0;
                while(cursor < readBuffer.length() && readBuffer[cursor] != '\r') {
                    len = len * 10 + (readBuffer[cursor] - '0');
                    cursor += 1;
                }

                if (cursor >= readBuffer.length()) {
                    cursor = savedCursor ;
                    return {};
                }

                cursor += 2; // skip \r\n;
                if (cursor > readBuffer.length()) {
                    cursor = savedCursor ;
                    return {};
                }

                if (len < 0) continue;

                if (cursor + len > readBuffer.length()) {
                    cursor = savedCursor;
                    return {};
                }

                string_view token(readBuffer.data() + cursor , len);
                cursor += len;

                cursor += 2;
                if (cursor > readBuffer.length()) {
                    cursor = savedCursor ;
                    return {};
                }

                tokens.push_back(token);
            }
        }
        if (tokens.size() == arraySize) return tokens;
        cursor = savedCursor;
        return {};
    }
    else {
        cursor = savedCursor;
        return {};
    } 
}


    
