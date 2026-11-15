/*
Author: Ayush Jugran 
Description: Interactive console menu, timetable analytics, cancellation auto-promotion, and application entrypoint.
*/

#include "BookingEngine.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>

using namespace std;

// Safe integer reading with stream error recovery
bool readInt(const string &prompt, int &val) {
    while (true) {
        cout << prompt;
        if (cin >> val) return true;
        if (cin.eof()) return false;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  ! Invalid number. Please enter a valid integer.\n";
    }
}

bool readStr(const string &prompt, string &val) {
    cout << prompt;
    return static_cast<bool>(cin >> val);
}

// Cancellation with Automatic FIFO Waiting List Promotion
bool cancelBooking(BookingEngine &engine, int bookingId) {
    Booking *b = engine.registry().find(bookingId);
    if (!b) {
        cout << "  ! [Error] Booking #" << bookingId << " not found in registry.\n";
        return false;
    }

    string roomId = b->roomId;
    int day = b->day;
    int from = b->from;
    int to = b->to;

    Room *r = engine.rooms().find(roomId);
    if (!r) return false;

    r->remove(bookingId);
    engine.registry().remove(bookingId);
    cout << "  [Cancelled] Booking #" << bookingId << " removed successfully.\n";

    // Auto-promote earliest fitting request from waiting queue
    WaitRequest *promoted = r->waitQueue().extractFitting(day, from, to, r->capacity(),
        [r](Purpose p) { return r->allows(p); });

    if (promoted) {
        cout << "\n  >>> [AUTO-PROMOTION] Earliest waiting request #" << promoted->id
             << " promoted to CONFIRMED BOOKING!\n";
        engine.book(promoted->roomId, promoted->user, static_cast<int>(promoted->purpose),
                    promoted->strength, promoted->day, promoted->from, promoted->to);
        delete promoted;
    }
    return true;
}

void showDayView(const BookingEngine &engine, const string &roomId, int day) {
    Room *r = engine.rooms().find(roomId);
    if (!r) { cout << "  ! No such room.\n"; return; }
    if (day < 1 || day > DAYS) { cout << "  ! Invalid day.\n"; return; }

    bool used[PERIODS + 1] = {};
    cout << "\n=== Schedule for " << r->id() << " (" << r->typeName() << ") on " << DAY_NAME[day] << " ===\n";

    for (Booking *b = r->daySlots(day); b; b = b->next) {
        cout << "  P" << b->from << "-" << b->to
             << " (" << PERIOD_START_TIME[b->from] << "-" << PERIOD_END_TIME[b->to] << ")  "
             << "#" << b->id << "  "
             << left << setw(10) << purposeName(b->purpose)
             << b->user << " (" << b->strength << " students)\n";
        for (int p = b->from; p <= b->to; p++) used[p] = true;
    }

    cout << "  Free Periods:";
    bool anyFree = false;
    for (int p = 1; p <= PERIODS; p++) {
        if (!used[p]) {
            cout << " " << p << " (" << PERIOD_START_TIME[p] << "-" << PERIOD_END_TIME[p] << ")";
            anyFree = true;
        }
    }
    if (!anyFree) cout << " None (Fully Booked)";
    cout << "\n";
}

void showWeekView(const BookingEngine &engine, const string &roomId) {
    Room *r = engine.rooms().find(roomId);
    if (!r) { cout << "  ! No such room.\n"; return; }

    cout << "\n=== Weekly View for Room " << r->id() << " (" << r->typeName() << ") ===\n";
    cout << left << setw(16) << "Period / Time";
    for (int d = 1; d <= DAYS; d++) cout << "| " << setw(8) << DAY_NAME[d] << " ";
    cout << "\n" << string(75, '-') << "\n";

    for (int p = 1; p <= PERIODS; p++) {
        cout << left << setw(16)
             << ("P" + to_string(p) + " " + PERIOD_START_TIME[p]);
        for (int d = 1; d <= DAYS; d++) {
            Booking *b = r->conflict(d, p, p);
            cout << "| " << setw(8) << (b ? ("#" + to_string(b->id)) : "FREE") << " ";
        }
        cout << "\n";
    }
}

void showWaitingList(const BookingEngine &engine, const string &roomId) {
    Room *r = engine.rooms().find(roomId);
    if (!r) { cout << "  ! No such room.\n"; return; }
    cout << "\n=== Waiting List for Room " << r->id() << " ===\n";
    r->waitQueue().display();
}

void listAllRooms(const BookingEngine &engine) {
    cout << "\n=== Registered Rooms (Sorted by ID via BST) ===\n";
    engine.rooms().inorder([](const Room &r) { r.describe(); });
}

void findFreeRooms(const BookingEngine &engine, int day, int from, int to, int minCap, int purposeCode) {
    if (day < 1 || day > DAYS || from < 1 || to > PERIODS || from > to) {
        cout << "  ! Invalid slot parameters.\n";
        return;
    }
    Purpose p = static_cast<Purpose>(purposeCode);
    cout << "\n=== Rooms FREE on " << DAY_NAME[day] << " P" << from << "-" << to
         << " (" << PERIOD_START_TIME[from] << "-" << PERIOD_END_TIME[to] << ") ===\n";

    int found = 0;
    engine.rooms().inorder([&](const Room &r) {
        if (r.capacity() >= minCap && r.allows(p) && !r.conflict(day, from, to)) {
            r.describe();
            found++;
        }
    });

    if (found == 0) cout << "  No rooms available for the specified criteria.\n";
    else cout << "  Found " << found << " free room(s).\n";
}

int main() {
    BookingEngine engine;
    engine.loadBookings("bookings.dat");

    cout << "==============================================================\n";
    cout << "  SCHEDULIFY: Smart Classroom & Lab Slot Booking System\n";
    cout << "  Graphic Era Hill University | Team: Alpha squad\n";
    cout << "==============================================================\n";

    while (true) {
        cout << "\n--- Main Menu ---\n"
             << "  1. Book Room Slot\n"
             << "  2. Cancel Booking (With Waitlist Promotion)\n"
             << "  3. Room Day View & Free Periods\n"
             << "  4. Room Week Grid View\n"
             << "  5. Room Waiting List\n"
             << "  6. Search Free Rooms\n"
             << "  7. List All Rooms (BST)\n"
             << "  8. Import Timetable CSV\n"
             << "  9. Save Bookings (bookings.dat)\n"
             << "  0. Save and Exit\n";

        int ch = -1;
        if (!readInt("Enter your choice (0-9): ", ch)) break;
        if (ch == 0) {
            engine.saveBookings("bookings.dat");
            cout << "\nAll data saved to bookings.dat. Goodbye!\n";
            break;
        }

        string rid, user;
        int p, s, d, f, t, bid;

        switch (ch) {
        case 1:
            readStr("Room ID: ", rid);
            readStr("Booked By (e.g. Faculty/Course): ", user);
            cout << "Purpose (1=Lecture, 2=Tutorial, 3=Practical, 4=Project): ";
            readInt("", p);
            readInt("Student Strength: ", s);
            cout << "Day (1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat): ";
            readInt("", d);
            readInt("From Period (1-10): ", f);
            readInt("To Period (1-10): ", t);
            engine.book(rid, user, p, s, d, f, t);
            break;
        case 2:
            readInt("Enter Booking ID to cancel: ", bid);
            cancelBooking(engine, bid);
            break;
        case 3:
            readStr("Room ID: ", rid);
            readInt("Day (1=Mon..6=Sat): ", d);
            showDayView(engine, rid, d);
            break;
        case 4:
            readStr("Room ID: ", rid);
            showWeekView(engine, rid);
            break;
        case 5:
            readStr("Room ID: ", rid);
            showWaitingList(engine, rid);
            break;
        case 6:
            readInt("Day (1-6): ", d);
            readInt("From Period: ", f);
            readInt("To Period: ", t);
            readInt("Min Capacity: ", s);
            cout << "Purpose (1=Lecture, 2=Tutorial, 3=Practical, 4=Project): ";
            readInt("", p);
            findFreeRooms(engine, d, f, t, s, p);
            break;
        case 7:
            listAllRooms(engine);
            break;
        case 8: {
            string path;
            readStr("CSV Filename (default: data/timetable.csv): ", path);
            if (path == "default" || path.empty()) path = "data/timetable.csv";
            int count = engine.loadTimetable(path);
            cout << "Loaded " << count << " class slots from " << path << ".\n";
            break;
        }
        case 9:
            engine.saveBookings("bookings.dat");
            cout << "Bookings successfully saved to bookings.dat.\n";
            break;
        default:
            cout << "  ! Invalid option. Please select 0 to 9.\n";
            break;
        }
    }
    return 0;
}