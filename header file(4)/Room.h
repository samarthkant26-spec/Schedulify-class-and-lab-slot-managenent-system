/*
Author: Ayush Jugran
Description: Abstract base Room class with sorted linked lists per day.
*/

#ifndef ROOM_H
#define ROOM_H

#include "Config.h"
#include "Booking.h"
#include "WaitQueue.h"
#include <string>

class Room {
protected:
    std::string id_;
    int capacity_;
    Booking *slots_[DAYS + 1] = {};  // Sorted linked list per day (1..6)
    WaitQueue waitQueue_;            // Waiting list for this room

public:
    Room(std::string id, int cap);
    virtual ~Room();

    Room(const Room &) = delete;
    Room &operator=(const Room &) = delete;

    virtual std::string typeName() const = 0;
    virtual bool allows(Purpose p) const = 0;
    virtual void describe() const = 0;

    const std::string &id() const { return id_; }
    int capacity() const { return capacity_; }
    void setCapacity(int cap) { capacity_ = cap; }

    Booking *daySlots(int d) const { return (d >= 1 && d <= DAYS) ? slots_[d] : nullptr; }
    WaitQueue &waitQueue() { return waitQueue_; }
    const WaitQueue &waitQueue() const { return waitQueue_; }

    Booking *conflict(int day, int from, int to) const;
    void insert(Booking *b);
    bool remove(int id);
};

#endif 