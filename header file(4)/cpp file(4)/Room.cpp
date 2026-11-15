/*
Author: Ayush Jugran
Description: Implementation of Room base class with clash detection and slot removal.
*/

#include "Room.h"

Room::Room(std::string id, int cap) : id_(std::move(id)), capacity_(cap) {}

Room::~Room() {
    for (int d = 1; d <= DAYS; d++) {
        Booking *curr = slots_[d];
        while (curr) {
            Booking *next = curr->next;
            delete curr;
            curr = next;
        }
        slots_[d] = nullptr;
    }
}

Booking *Room::conflict(int day, int from, int to) const {
    if (day < 1 || day > DAYS) return nullptr;
    for (Booking *b = slots_[day]; b && b->from <= to; b = b->next) {
        if (from <= b->to) return b;
    }
    return nullptr;
}

void Room::insert(Booking *b) {
    if (!b || b->day < 1 || b->day > DAYS) return;
    Booking **pp = &slots_[b->day];
    while (*pp && (*pp)->from < b->from) {
        pp = &(*pp)->next;
    }
    b->next = *pp;
    *pp = b;
}

bool Room::remove(int id) {
    for (int d = 1; d <= DAYS; d++) {
        Booking **pp = &slots_[d];
        while (*pp) {
            if ((*pp)->id == id) {
                Booking *target = *pp;
                *pp = (*pp)->next;
                delete target;
                return true;
            }
            pp = &(*pp)->next;
        }
    }
    return false;
}