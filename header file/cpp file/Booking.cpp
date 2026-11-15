/*
Author: Samarth kant 
Description: Implementation of the global booking registry.
*/

#include "Booking.h"

BookingList::~BookingList() {
    head = nullptr;
}

void BookingList::add(Booking *b) {
    if (!b) return;
    b->registryNext = head;
    head = b;
}

bool BookingList::remove(int id) {
    if (!head) return false;
    if (head->id == id) {
        head = head->registryNext;
        return true;
    }
    Booking *curr = head;
    while (curr->registryNext && curr->registryNext->id != id) {
        curr = curr->registryNext;
    }
    if (curr->registryNext) {
        curr->registryNext = curr->registryNext->registryNext;
        return true;
    }
    return false;
}

Booking *BookingList::find(int id) const {
    for (Booking *b = head; b; b = b->registryNext) {
        if (b->id == id) return b;
    }
    return nullptr;
}