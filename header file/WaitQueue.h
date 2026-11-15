/*
Author: Samarth kant
Description: Linked FIFO Queue for rooms requested while already occupied.
*/

#ifndef WAIT_QUEUE_H
#define WAIT_QUEUE_H

#include "Config.h"
#include <string>
#include <functional>

struct WaitRequest {
    int id;
    std::string roomId;
    std::string user;
    Purpose purpose;
    int strength;
    int day;
    int from;
    int to;
    WaitRequest *next = nullptr;
};

class WaitQueue {
private:
    WaitRequest *front_ = nullptr;
    WaitRequest *rear_ = nullptr;
    int size_ = 0;

public:
    WaitQueue() = default;
    ~WaitQueue();

    WaitQueue(const WaitQueue &) = delete;
    WaitQueue &operator=(const WaitQueue &) = delete;

    bool isEmpty() const { return front_ == nullptr; }
    int size() const { return size_; }

    void enqueue(int id, const std::string &roomId, const std::string &user,
                 Purpose purpose, int strength, int day, int from, int to);

    bool dequeue(WaitRequest &out);

    WaitRequest *extractFitting(int day, int from, int to, int capacity,
                                const std::function<bool(Purpose)> &allowsFn);

    void display() const;
    WaitRequest *getFront() const { return front_; }
};

#endif 