/*
 * Author: Mahi Rawat (Part 3)
 * Description: 5-tier booking validator, timetable CSV parser, and file persistence.
 */

#ifndef BOOKING_ENGINE_H
#define BOOKING_ENGINE_H

#include "Config.h"
#include "Booking.h"
#include "Room.h"
#include "RoomTree.h"
#include <string>

class BookingEngine {
protected:
    RoomTree rooms_;
    BookingList registry_;
    int nextId_ = 1001;
    int nextWaitId_ = 5001;

public:
    BookingEngine();
    virtual ~BookingEngine() = default;

    RoomTree &rooms() { return rooms_; }
    const RoomTree &rooms() const { return rooms_; }
    BookingList &registry() { return registry_; }

    static bool parseSafeInt(const std::string &str, int &out);

    bool book(const std::string &roomId, const std::string &user, int purpose,
              int strength, int day, int from, int to, bool suppressOutput = false);

    int loadTimetable(const std::string &filename);
    bool saveBookings(const std::string &filename = "bookings.dat") const;
    int loadBookings(const std::string &filename = "bookings.dat");
};

#endif // BOOKING_ENGINE_H