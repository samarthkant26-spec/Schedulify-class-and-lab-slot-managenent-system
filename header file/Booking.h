/*
Author: Samarth kant
Description: Node structure for bookings and manual global linked list registry.
*/

#ifndef BOOKING_H
#define BOOKING_H

#include "Config.h"
#include <string>

struct Booking {
    int id = 0;
    std::string roomId;
    std::string user;
    Purpose purpose = Purpose::Lecture;
    int strength = 0;
    int day = 0;
    int from = 0;
    int to = 0;
    Booking *next = nullptr;
    Booking *registryNext = nullptr; 


class BookingList {
private:
    Booking *head = nullptr;

public:
    BookingList() = default;
    ~BookingList();

    BookingList(const BookingList &) = delete;
    BookingList &operator=(const BookingList &) = delete;

    void add(Booking *b);
    bool remove(int id);
    Booking *find(int id) const;
    Booking *getHead() const { return head; }
};

#endif
}