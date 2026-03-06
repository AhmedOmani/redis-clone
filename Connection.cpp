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
    string data;
    while(true) {
        int n = read(fd, buf, sizeof(buf) - 1);
        bytesRead += (n > 0 ? n : 0);
        if (n <= 0) {
            if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                redisClient(data , bytesRead);
                break;
            }
            onClose(fd);
            return;
        }
        buf[n] = '\0';
        data.append(buf, n);
    }

    if (!data.empty()) {
        cout << "Read from " << fd << " : " << data << endl;
        // TODO: parse RESP protocol here
    }
}

void Connection::redisClient(string data , int n) {
    vector<string> tokens = parse(data);
    cout << "Tokens after parsing...\n" ;
    for (auto &s : tokens)
        cout << s << ' ';
    cout << endl;

    if (tokens[0] == "SET") {
        string key = tokens[1];
        uint64_t value = stoull(tokens[2]);
        db->insert(key , value);
        string response = "+OK\r\n";
        send(fd , response.c_str() , response.length() , 0);
        return;
    }
    if (tokens[0] == "GET") {
        string key = tokens[1];
        uint64_t value = db->search(key);
        string response = "+" + to_string(value) + "\r\n";
        send(fd , response.c_str() , response.length() , 0);
        return;
    }
    string response = "+OK\r\n";
    send(fd , response.c_str() , response.length() , 0);
}

vector<string> Connection::parse(string data) {
    if (data[0] == '*') {
        int curIdx = 1;
        string sizeStr = "";
        while (data[curIdx] != '\r') {
            sizeStr += data[curIdx];
            curIdx++;
        }
        int arraySize = stoi(sizeStr);
        vector<string> tokens;
        curIdx += 2; // skip \r\n after the size
        

        
        // Trace this: 
        //*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$3\r\n100\r\n

        while(tokens.size() < arraySize) {
            if (data[curIdx] == '$') {
                //Get the length of the string
                curIdx++;
                string lenStr = "";
                while (data[curIdx] != '\r') {
                    lenStr += data[curIdx];
                    curIdx += 1;
                }

                curIdx += 2; // skip \r\n after the length
                
                int len = stoi(lenStr);
                if (len < 0) continue;
                
                //Extract the string
                string token = "";
                for (int i = 0 ; i < len ; i++ , curIdx++) {
                    token += data[curIdx];
                }
                
                curIdx += 2; // skip \r\n after the string
                tokens.push_back(token);
            }
             
        }
        return tokens;
    }
    return {};
}


    
