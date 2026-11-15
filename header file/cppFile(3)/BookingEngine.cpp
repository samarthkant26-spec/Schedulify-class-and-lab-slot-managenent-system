/*
 * Author: Mahi Rawat (Part 3)
 * Description: Implementation of booking validation, CSV loading, and data persistence.
 */

#include "BookingEngine.h"
#include "DerivedRooms.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <cctype>

BookingEngine::BookingEngine() {
    rooms_.insert(new LectureTheatre("124", 120));
    rooms_.insert(new LectureTheatre("224", 120));
    rooms_.insert(new LectureTheatre("206", 100));
    rooms_.insert(new Classroom("117", 60));
    rooms_.insert(new Classroom("114", 60));
    rooms_.insert(new Lab("009", 40));
    rooms_.insert(new Lab("209", 40));
    rooms_.insert(new Auditorium("529", 500));
}

bool BookingEngine::parseSafeInt(const std::string &str, int &out) {
    if (str.empty()) return false;
    size_t i = (str[0] == '-' || str[0] == '+') ? 1 : 0;
    if (i >= str.size()) return false;
    for (; i < str.size(); i++) {
        if (!isdigit(static_cast<unsigned char>(str[i]))) return false;
    }
    out = std::stoi(str);
    return true;
}

bool BookingEngine::book(const std::string &roomId, const std::string &user, int purpose,
                         int strength, int day, int from, int to, bool suppressOutput) {
    Room *r = rooms_.find(roomId);
    if (!r) {
        if (!suppressOutput) std::cout << "  ! [Error] Room '" << roomId << "' does not exist.\n";
        return false;
    }

    if (day < 1 || day > DAYS || from < 1 || to > PERIODS || from > to || purpose < 1 || purpose > 4) {
        if (!suppressOutput) std::cout << "  ! [Error] Invalid day, period range, or purpose code.\n";
        return false;
    }

    Purpose p = static_cast<Purpose>(purpose);

    if (!r->allows(p)) {
        if (!suppressOutput) {
            std::cout << "  ! [Rejected] " << r->typeName() << " cannot host a "
                      << purposeName(p) << ".\n";
        }
        return false;
    }

    if (strength > r->capacity()) {
        if (!suppressOutput) {
            std::cout << "  ! [Rejected] Strength " << strength
                      << " exceeds room capacity of " << r->capacity() << ".\n";
        }
        return false;
    }

    if (Booking *c = r->conflict(day, from, to)) {
        if (!suppressOutput) {
            std::cout << "  ! [Clash] Clashes with Booking #" << c->id
                      << " (" << c->user << ", P" << c->from << "-" << c->to << ").\n";
            std::cout << "  -> Adding request to " << r->id() << " waiting list...\n";
        }
        r->waitQueue().enqueue(nextWaitId_++, roomId, user, p, strength, day, from, to);
        return false;
    }

    Booking *b = new Booking;
    b->id = nextId_++;
    b->roomId = roomId;
    b->user = user;
    b->purpose = p;
    b->strength = strength;
    b->day = day;
    b->from = from;
    b->to = to;

    r->insert(b);
    registry_.add(b);

    if (!suppressOutput) {
        std::cout << "  [Confirmed] Booking #" << b->id << ": " << r->id() << " "
                  << DAY_NAME[day] << " P" << from << "-" << to
                  << " (" << PERIOD_START_TIME[from] << "-" << PERIOD_END_TIME[to] << ")"
                  << " for " << user << "\n";
    }
    return true;
}

int BookingEngine::loadTimetable(const std::string &filename) {
    std::ifstream in(filename);
    if (!in.is_open()) return 0;

    std::string line;
    std::getline(in, line); // Skip header
    int count = 0;

    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> row;

        while (std::getline(ss, token, ',')) {
            size_t s = token.find_first_not_of(" \r\n\t");
            size_t e = token.find_last_not_of(" \r\n\t");
            row.push_back((s == std::string::npos) ? "" : token.substr(s, e - s + 1));
        }

        if (row.size() < 8) continue;

        std::string course = row[1];
        int day = 0, from = 0, to = 0;
        if (!parseSafeInt(row[2], day) || !parseSafeInt(row[3], from) || !parseSafeInt(row[4], to)) continue;

        std::string typeStr = row[5];
        std::string subject = row[6];
        std::string roomId = row[7];
        std::string kind = (row.size() >= 9) ? row[8] : "CR";

        Room *r = rooms_.find(roomId);
        if (!r) {
            r = makeRoom(kind, roomId);
            rooms_.insert(r);
        }

        Purpose p = parsePurpose(typeStr);

        Booking *clash = r->conflict(day, from, to);
        if (clash && clash->user.find(course) != std::string::npos) {
            clash->user += "/" + subject;
            continue;
        }

        if (book(roomId, course + " (" + subject + ")", static_cast<int>(p), 30, day, from, to, true)) {
            count++;
        }
    }
    return count;
}

bool BookingEngine::saveBookings(const std::string &filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    for (Booking *b = registry_.getHead(); b; b = b->registryNext) {
        out << "B," << b->id << "," << b->roomId << "," << b->user << ","
            << static_cast<int>(b->purpose) << "," << b->strength << ","
            << b->day << "," << b->from << "," << b->to << "\n";
    }

    rooms_.inorder([&out](const Room &r) {
        for (WaitRequest *w = r.waitQueue().getFront(); w; w = w->next) {
            out << "W," << w->id << "," << w->roomId << "," << w->user << ","
                << static_cast<int>(w->purpose) << "," << w->strength << ","
                << w->day << "," << w->from << "," << w->to << "\n";
        }
    });

    return true;
}

int BookingEngine::loadBookings(const std::string &filename) {
    std::ifstream in(filename);
    if (!in.is_open()) return 0;

    std::string line;
    int count = 0;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string tag, idStr, roomId, user, purStr, strStr, dayStr, fromStr, toStr;

        std::getline(ss, tag, ',');
        std::getline(ss, idStr, ',');
        std::getline(ss, roomId, ',');
        std::getline(ss, user, ',');
        std::getline(ss, purStr, ',');
        std::getline(ss, strStr, ',');
        std::getline(ss, dayStr, ',');
        std::getline(ss, fromStr, ',');
        std::getline(ss, toStr, ',');

        int id = 0, pur = 1, st = 0, d = 0, f = 0, t = 0;
        if (!parseSafeInt(idStr, id) || !parseSafeInt(purStr, pur) ||
            !parseSafeInt(strStr, st) || !parseSafeInt(dayStr, d) ||
            !parseSafeInt(fromStr, f) || !parseSafeInt(toStr, t)) continue;

        Room *r = rooms_.find(roomId);
        if (!r) {
            r = makeRoom("CR", roomId);
            rooms_.insert(r);
        }

        if (tag == "B") {
            Booking *b = new Booking{id, roomId, user, static_cast<Purpose>(pur), st, d, f, t, nullptr, nullptr};
            r->insert(b);
            registry_.add(b);
            if (id >= nextId_) nextId_ = id + 1;
            count++;
        } else if (tag == "W") {
            r->waitQueue().enqueue(id, roomId, user, static_cast<Purpose>(pur), st, d, f, t);
            if (id >= nextWaitId_) nextWaitId_ = id + 1;
            count++;
        }
    }
    return count;
}
