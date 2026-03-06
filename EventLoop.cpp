#include "EventLoop.hpp"
#include <iostream>
#include <sys/epoll.h>

EventLoop::EventLoop() {
    epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::cerr << "Failed to create epoll instance" << std::endl;
        exit(1);
    } 
}

bool EventLoop::addEvent(int fd, uint32_t events) {
    struct epoll_event event;
    event.events = events;
    event.data.fd = fd;
    return epoll_ctl(epollFd , EPOLL_CTL_ADD , fd , &event) != -1;
}

int EventLoop::wait(std::vector<epoll_event>& activeEvents) {
    activeEvents.resize(MAX_EVENTS);
    return epoll_wait(epollFd , activeEvents.data() , MAX_EVENTS , -1);
}