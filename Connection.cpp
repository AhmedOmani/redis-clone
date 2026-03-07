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
        buffer.append(buf, n);
    }

    if (!buffer.empty()) {
        cout << "Read from " << fd << " : " << buffer << endl;
        processBuffer();
    }
}

void Connection::processBuffer() {
    while(cursor < buffer.length()) {
        
        vector<string> tokens = parse();
        
        if (tokens.empty()) break;

        handleCommand(tokens);
    }

    if (cursor == buffer.length()) { // O(1)
        buffer.clear();
        cursor = 0;
    }
    else if (cursor > MAX_BUFFER_SIZE) {
        buffer.erase(0 , cursor);
        cursor = 0;
    }
}

void Connection::handleCommand(vector<string> tokens) {
    cout << "Tokens after parsing...\n" ;
    for (auto &s : tokens)
        cout << s << ' ';
    cout << endl;

    if (tokens[0] == "SET") {
        if (tokens.size() < 3) {
            send(fd, "-ERR wrong number of arguments for 'set' command\r\n", 48, 0);
            return;
        }
        string key = tokens[1];
        string* value = new string(tokens[2]);

        InsertResult result = db->insert(key , value);

        if (result == InsertResult::OK) {
            send(fd, "+OK\r\n", 5, 0);
        }
        else if (result == InsertResult::ERR_OOM) {
            string err = "-OOM command not allowed when used memory > maxmemory\r\n";
            delete value;
            send(fd, err.c_str(), err.length(), 0);
        }
        else if (result == InsertResult::ERR_FULL) {
            string err = "-ERR hash table is full\r\n";
            delete value;
            send(fd, err.c_str(), err.length(), 0);
        }
        return;
    }
    if (tokens[0] == "GET") {
        if (tokens.size() < 2) {
            send(fd, "-ERR wrong number of arguments for 'get' command\r\n", 48, 0);
            return;
        }
        string key = tokens[1];
        string* value = db->search(key);
        if (value == nullptr) {
            string response = "-Key not found\r\n";
            send(fd , response.c_str() , response.length() , 0);
            return;
        }
        string response = "$" + to_string(value->length()) + "\r\n" + *value + "\r\n";
        send(fd, response.c_str(), response.length(), 0);   
        return;
    }
    string response = "+OK\r\n";
    send(fd , response.c_str() , response.length() , 0);
}

vector<string> Connection::parse() {
    int savedCursor = cursor;
    if (buffer[cursor] == '*') {
        cursor++;
        if (cursor >= buffer.length()) {
            cursor = savedCursor;
            return {};
        }
        string sizeStr = "";
        while (cursor < buffer.length() && buffer[cursor] != '\r') {
            sizeStr += buffer[cursor];
            cursor++;
            if (cursor >= buffer.length()) {
                cursor = savedCursor;
                return {};
            }
        }
        int arraySize = stoi(sizeStr);
        vector<string> tokens;
        cursor += 2; // skip \r\n after the size
        if (cursor > buffer.length()) {
            cursor = savedCursor;
            return {};
        }
        
        // Trace this: 
        //*3\r\n$3\r\ nSET\r\n$5\r\nmykey\r\n$3\r\n100\r\n
        
        int slashCounts = 0 ;
        int tmpIdx = cursor;
/*
        //The idea from this loop is to verify that i have a complete command in the buffer 
        //If i have a complete command i will process it
        //either i will wait until the buffer has a complete command
        while (tmpIdx < buffer.length() && slashCounts < arraySize * 2) {
            if (buffer[tmpIdx] == '\r') {
                slashCounts += 1;                
            }
            if (buffer[tmpIdx] == '\n') {
                slashCounts += 1;                
            }
            tmpIdx++;
        }
        
        if (slashCounts < arraySize * 2) {
            cursor = savedCursor;
            return {};
        }
*/
        //Now i have a complete command in the buffer
        //i will process it

        while(cursor < buffer.length() && tokens.size() < arraySize) {
            if (buffer[cursor] == '$') {
                //Get the length of the string
                cursor++;
                if (cursor >= buffer.length()) {
                    cursor = savedCursor;
                    return {};
                }
                string lenStr = "";
                while (cursor < buffer.length() && buffer[cursor] != '\r') {
                    lenStr += buffer[cursor];
                    cursor += 1;
                    if (cursor >= buffer.length()) {
                        cursor = savedCursor;
                        return {};
                    }
                }

                cursor += 2; // skip \r\n after the length
                if (cursor > buffer.length()) {
                    cursor = savedCursor;
                    return {};
                }

                int len = stoi(lenStr);
                if (len < 0) continue;
                
                //Extract the string
                string token = "";
                for (int i = 0 ; i < len ; i++) {
                    token += buffer[cursor];
                    cursor += 1;
                    if (cursor >= buffer.length()) {
                        cursor = savedCursor;
                        return {};
                    }
                }
                
                cursor += 2; // skip \r\n after the string
                if (cursor > buffer.length()) {
                    cursor = savedCursor;
                    return {};
                }
                tokens.push_back(token);
            }
             
        }
        if (tokens.size() == arraySize) return tokens;
        cursor = savedCursor;
        return {};
    }
    return {};
}


    
