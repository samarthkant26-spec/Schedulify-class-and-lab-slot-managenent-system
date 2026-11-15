/*
Author: Samarth kant
Description: Implementation of the FIFO Waiting Queue for rooms.
*/

#include "WaitQueue.h"
#include <iostream>
#include <iomanip>

WaitQueue::~WaitQueue() {
    while (front_) {
        WaitRequest *temp = front_;
        front_ = front_->next;
        delete temp;
    }
    rear_ = nullptr;
    size_ = 0;
}

void WaitQueue::enqueue(int id, const std::string &roomId, const std::string &user,
                        Purpose purpose, int strength, int day, int from, int to) {
    WaitRequest *req = new WaitRequest{id, roomId, user, purpose, strength, day, from, to, nullptr};
    if (!rear_) {
        front_ = rear_ = req;
    } else {
        rear_->next = req;
        rear_ = req;
    }
    size_++;
}

bool WaitQueue::dequeue(WaitRequest &out) {
    if (!front_) return false;
    WaitRequest *temp = front_;
    out = *temp;
    front_ = front_->next;
    if (!front_) rear_ = nullptr;
    delete temp;
    size_--;
    return true;
}

WaitRequest *WaitQueue::extractFitting(int day, int from, int to, int capacity,
                                      const std::function<bool(Purpose)> &allowsFn) {
    if (!front_) return nullptr;

    WaitRequest *prev = nullptr;
    WaitRequest *curr = front_;

    while (curr) {
        if (curr->day == day && curr->from >= from && curr->to <= to &&
            curr->strength <= capacity && allowsFn(curr->purpose)) {
            if (!prev) {
                front_ = curr->next;
                if (!front_) rear_ = nullptr;
            } else {
                prev->next = curr->next;
                if (curr == rear_) rear_ = prev;
            }
            curr->next = nullptr;
            size_--;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return nullptr;
}

void WaitQueue::display() const {
    if (!front_) {
        std::cout << "    (No waiting requests)\n";
        return;
    }
    for (WaitRequest *w = front_; w; w = w->next) {
        std::cout << "    [Wait #" << w->id << "] " << DAY_NAME[w->day]
                  << " P" << w->from << "-" << w->to
                  << " (" << PERIOD_START_TIME[w->from] << "-" << PERIOD_END_TIME[w->to] << ") "
                  << std::left << std::setw(10) << purposeName(w->purpose)
                  << w->user << " (" << w->strength << " students)\n";
    }
}