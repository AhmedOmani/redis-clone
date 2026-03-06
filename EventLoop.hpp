#pragma once
#include <vector>
#include <sys/epoll.h>

#define MAX_EVENTS 1024

class EventLoop {
private:
    int epollFd;
public:
    EventLoop();
    bool addEvent(int fd , uint32_t events);
    int wait(std::vector<epoll_event>& activeEvents);
};
