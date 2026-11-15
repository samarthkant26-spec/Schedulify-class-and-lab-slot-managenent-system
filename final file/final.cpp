/*
 Samarth kant 

   Constants, Timings, Purpose Enum, Booking Node, BookingList, WaitRequest, WaitQueue


 Ayush Jugran 

    Abstract Room class, per-day sorted linked lists, early-exit conflict(), insert(), remove()


  Ayesha 

    Derived Rooms (LT, Classroom, Lab, Auditorium), makeRoom() factory, RoomTree (BST)


 Mahi Rawat 

    BookingEngine, 5-tier validator book(), loadTimetable() CSV parser, save/loadBookings()


 Ayush Jugran 

    cancelBooking() with FIFO auto-promotion, showDayView(), showWeekView(), CLI main loop
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <limits>
#include <functional>

using namespace std;

// SECTION 1: CONSTANTS, TIMINGS & PURPOSE ENUM 
// Samarth kant


constexpr int DAYS = 6;
constexpr int PERIODS = 10;

static const char *DAY_NAME[DAYS + 1] = {
    "", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char *PERIOD_START_TIME[PERIODS + 1] = {
    "", "08:00", "08:55", "10:10", "11:05", "12:00",
    "12:55", "14:10", "15:05", "16:00", "16:55"
};

static const char *PERIOD_END_TIME[PERIODS + 1] = {
    "", "08:55", "09:50", "11:05", "12:00", "12:55",
    "13:50", "15:05", "16:00", "16:55", "17:50"
};

enum class Purpose {
    Lecture = 1,
    Tutorial = 2,
    Practical = 3,
    Project = 4
};

inline const char *purposeName(Purpose p) {
    static const char *names[] = {"", "Lecture", "Tutorial", "Practical", "Project"};
    int idx = static_cast<int>(p);
    if (idx >= 1 && idx <= 4) return names[idx];
    return "Unknown";
}

inline Purpose parsePurpose(const string &str) {
    if (str == "Lab" || str == "Practical" || str == "3") return Purpose::Practical;
    if (str == "Tutorial" || str == "2") return Purpose::Tutorial;
    if (str == "Project" || str == "4") return Purpose::Project;
    return Purpose::Lecture;
}

// BOOKING REGISTRY & FIFO WAITING QUEUE 


struct Booking {
    int id = 0;
    string roomId;
    string user;
    Purpose purpose = Purpose::Lecture;
    int strength = 0;
    int day = 0;
    int from = 0;
    int to = 0;
    Booking *next = nullptr;
    Booking *registryNext = nullptr;
};

class BookingList {
private:
    Booking *head = nullptr;

public:
    BookingList() = default;
    ~BookingList() { head = nullptr; }

    BookingList(const BookingList &) = delete;
    BookingList &operator=(const BookingList &) = delete;

    void add(Booking *b) {
        if (!b) return;
        b->registryNext = head;
        head = b;
    }

    bool remove(int id) {
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

    Booking *find(int id) const {
        for (Booking *b = head; b; b = b->registryNext) {
            if (b->id == id) return b;
        }
        return nullptr;
    }

    Booking *getHead() const { return head; }
};

struct WaitRequest {
    int id;
    string roomId;
    string user;
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
    ~WaitQueue() {
        while (front_) {
            WaitRequest *temp = front_;
            front_ = front_->next;
            delete temp;
        }
        rear_ = nullptr;
        size_ = 0;
    }

    WaitQueue(const WaitQueue &) = delete;
    WaitQueue &operator=(const WaitQueue &) = delete;

    bool isEmpty() const { return front_ == nullptr; }
    int size() const { return size_; }

    void enqueue(int id, const string &roomId, const string &user,
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

    bool dequeue(WaitRequest &out) {
        if (!front_) return false;
        WaitRequest *temp = front_;
        out = *temp;
        front_ = front_->next;
        if (!front_) rear_ = nullptr;
        delete temp;
        size_--;
        return true;
    }

    WaitRequest *extractFitting(int day, int from, int to, int capacity,
                                const function<bool(Purpose)> &allowsFn) {
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

    void display() const {
        if (!front_) {
            cout << "    (No waiting requests)\n";
            return;
        }
        for (WaitRequest *w = front_; w; w = w->next) {
            cout << "    [Wait #" << w->id << "] " << DAY_NAME[w->day]
                 << " P" << w->from << "-" << w->to
                 << " (" << PERIOD_START_TIME[w->from] << "-" << PERIOD_END_TIME[w->to] << ") "
                 << left << setw(10) << purposeName(w->purpose)
                 << w->user << " (" << w->strength << " students)\n";
        }
    }

    WaitRequest *getFront() const { return front_; }
};

// ABSTRACT ROOM CLASS & CONFLICT DETECTION  
// Ayush Jugran

class Room {
protected:
    string id_;
    int capacity_;
    Booking *slots_[DAYS + 1] = {};
    WaitQueue waitQueue_;

public:
    Room(string id, int cap) : id_(move(id)), capacity_(cap) {}

    virtual ~Room() {
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

    Room(const Room &) = delete;
    Room &operator=(const Room &) = delete;

    virtual string typeName() const = 0;
    virtual bool allows(Purpose p) const = 0;
    virtual void describe() const = 0;

    const string &id() const { return id_; }
    int capacity() const { return capacity_; }
    void setCapacity(int cap) { capacity_ = cap; }

    Booking *daySlots(int d) const { return (d >= 1 && d <= DAYS) ? slots_[d] : nullptr; }
    WaitQueue &waitQueue() { return waitQueue_; }
    const WaitQueue &waitQueue() const { return waitQueue_; }

    Booking *conflict(int day, int from, int to) const {
        if (day < 1 || day > DAYS) return nullptr;
        for (Booking *b = slots_[day]; b && b->from <= to; b = b->next) {
            if (from <= b->to) return b;
        }
        return nullptr;
    }

    void insert(Booking *b) {
        if (!b || b->day < 1 || b->day > DAYS) return;
        Booking **pp = &slots_[b->day];
        while (*pp && (*pp)->from < b->from) {
            pp = &(*pp)->next;
        }
        b->next = *pp;
        *pp = b;
    }

    bool remove(int id) {
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
};

// SECTION 4: CONCRETE ROOM HIERARCHY & ROOM BST 
// Ayesha


class LectureTheatre : public Room {
private:
    bool hasProjector_;

public:
    LectureTheatre(string id, int cap = 120, bool projector = true)
        : Room(move(id), cap), hasProjector_(projector) {}

    string typeName() const override { return "Lecture Theatre"; }
    bool allows(Purpose p) const override {
        return p == Purpose::Lecture || p == Purpose::Tutorial;
    }
    void describe() const override {
        cout << left << setw(8) << id_
             << setw(18) << typeName()
             << "Cap: " << setw(5) << capacity_
             << "Feature: " << (hasProjector_ ? "Dual Projectors" : "Standard") << "\n";
    }
};

class Classroom : public Room {
private:
    bool hasSmartboard_;

public:
    Classroom(string id, int cap = 60, bool smartboard = true)
        : Room(move(id), cap), hasSmartboard_(smartboard) {}

    string typeName() const override { return "Classroom"; }
    bool allows(Purpose p) const override {
        return p != Purpose::Practical;
    }
    void describe() const override {
        cout << left << setw(8) << id_
             << setw(18) << typeName()
             << "Cap: " << setw(5) << capacity_
             << "Feature: " << (hasSmartboard_ ? "Interactive Smartboard" : "Whiteboard") << "\n";
    }
};

class Lab : public Room {
private:
    int numComputers_;

public:
    Lab(string id, int cap = 40, int computers = 40)
        : Room(move(id), cap), numComputers_(computers) {}

    string typeName() const override { return "Lab"; }
    bool allows(Purpose p) const override {
        return p == Purpose::Practical || p == Purpose::Project;
    }
    void describe() const override {
        cout << left << setw(8) << id_
             << setw(18) << typeName()
             << "Cap: " << setw(5) << capacity_
             << "Feature: " << numComputers_ << " Workstations\n";
    }
};

class Auditorium : public Room {
private:
    bool hasSoundSystem_;

public:
    Auditorium(string id, int cap = 500, bool sound = true)
        : Room(move(id), cap), hasSoundSystem_(sound) {}

    string typeName() const override { return "Auditorium"; }
    bool allows(Purpose p) const override {
        return p == Purpose::Lecture || p == Purpose::Project;
    }
    void describe() const override {
        cout << left << setw(8) << id_
             << setw(18) << typeName()
             << "Cap: " << setw(5) << capacity_
             << "Feature: " << (hasSoundSystem_ ? "Dolby Audio System" : "Basic PA") << "\n";
    }
};

Room *makeRoom(const string &kind, const string &id, int cap = 0) {
    string k = kind;
    transform(k.begin(), k.end(), k.begin(), ::toupper);

    if (k == "LT" || k.find("THEATRE") != string::npos) {
        return new LectureTheatre(id, cap > 0 ? cap : 120);
    } else if (k == "LAB" || k.find("LABORATORY") != string::npos) {
        return new Lab(id, cap > 0 ? cap : 40);
    } else if (k == "AUD" || k == "AUDITORIUM" || k.find("AUDI") != string::npos) {
        return new Auditorium(id, cap > 0 ? cap : 500);
    }
    return new Classroom(id, cap > 0 ? cap : 60);
}

class RoomTree {
private:
    struct Node {
        Room *room;
        Node *l = nullptr;
        Node *r = nullptr;
        explicit Node(Room *x) : room(x) {}
    };

    Node *root_ = nullptr;
    int count_ = 0;

    static void destroy(Node *n) {
        if (!n) return;
        destroy(n->l);
        destroy(n->r);
        delete n->room;
        delete n;
    }

    template <class Fn>
    static void walk(Node *n, Fn &fn) {
        if (!n) return;
        walk(n->l, fn);
        fn(*n->room);
        walk(n->r, fn);
    }

public:
    RoomTree() = default;
    ~RoomTree() {
        destroy(root_);
        root_ = nullptr;
        count_ = 0;
    }

    RoomTree(const RoomTree &) = delete;
    RoomTree &operator=(const RoomTree &) = delete;

    Room *find(const string &id) const {
        Node *n = root_;
        while (n) {
            if (id == n->room->id()) return n->room;
            n = (id < n->room->id()) ? n->l : n->r;
        }
        return nullptr;
    }

    void insert(Room *room) {
        if (!room) return;
        Node **pp = &root_;
        while (*pp) {
            if (room->id() == (*pp)->room->id()) {
                delete room;
                return;
            }
            pp = (room->id() < (*pp)->room->id()) ? &(*pp)->l : &(*pp)->r;
        }
        *pp = new Node(room);
        count_++;
    }

    int size() const { return count_; }

    template <class Fn>
    void inorder(Fn fn) const { walk(root_, fn); }
};

// 5-TIER BOOKING ENGINE & DATA PERSISTENCE 
// Mahi Rawat


class BookingEngine {
protected:
    RoomTree rooms_;
    BookingList registry_;
    int nextId_ = 1001;
    int nextWaitId_ = 5001;

public:
    BookingEngine() {
        rooms_.insert(new LectureTheatre("124", 120));
        rooms_.insert(new LectureTheatre("224", 120));
        rooms_.insert(new LectureTheatre("206", 100));
        rooms_.insert(new Classroom("117", 60));
        rooms_.insert(new Classroom("114", 60));
        rooms_.insert(new Lab("009", 40));
        rooms_.insert(new Lab("209", 40));
        rooms_.insert(new Auditorium("529", 500));
    }

    virtual ~BookingEngine() = default;

    RoomTree &rooms() { return rooms_; }
    const RoomTree &rooms() const { return rooms_; }
    BookingList &registry() { return registry_; }

    static bool parseSafeInt(const string &str, int &out) {
        if (str.empty()) return false;
        size_t i = (str[0] == '-' || str[0] == '+') ? 1 : 0;
        if (i >= str.size()) return false;
        for (; i < str.size(); i++) {
            if (!isdigit(static_cast<unsigned char>(str[i]))) return false;
        }
        out = stoi(str);
        return true;
    }

    bool book(const string &roomId, const string &user, int purpose,
              int strength, int day, int from, int to, bool suppressOutput = false) {
        Room *r = rooms_.find(roomId);
        if (!r) {
            if (!suppressOutput) cout << "  ! [Error] Room '" << roomId << "' does not exist.\n";
            return false;
        }

        if (day < 1 || day > DAYS || from < 1 || to > PERIODS || from > to || purpose < 1 || purpose > 4) {
            if (!suppressOutput) cout << "  ! [Error] Invalid day, period range, or purpose code.\n";
            return false;
        }

        Purpose p = static_cast<Purpose>(purpose);

        if (!r->allows(p)) {
            if (!suppressOutput) {
                cout << "  ! [Rejected] " << r->typeName() << " cannot host a "
                     << purposeName(p) << ".\n";
            }
            return false;
        }

        if (strength > r->capacity()) {
            if (!suppressOutput) {
                cout << "  ! [Rejected] Strength " << strength
                     << " exceeds room capacity of " << r->capacity() << ".\n";
            }
            return false;
        }

        if (Booking *c = r->conflict(day, from, to)) {
            if (!suppressOutput) {
                cout << "  ! [Clash] Clashes with Booking #" << c->id
                     << " (" << c->user << ", P" << c->from << "-" << c->to << ").\n";
                cout << "  -> Adding request to " << r->id() << " waiting list...\n";
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
            cout << "  [Confirmed] Booking #" << b->id << ": " << r->id() << " "
                 << DAY_NAME[day] << " P" << from << "-" << to
                 << " (" << PERIOD_START_TIME[from] << "-" << PERIOD_END_TIME[to] << ")"
                 << " for " << user << "\n";
        }
        return true;
    }

    int loadTimetable(const string &filename) {
        ifstream in(filename);
        if (!in.is_open()) return 0;

        string line;
        getline(in, line);
        int count = 0;

        while (getline(in, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string token;
            vector<string> row;

            while (getline(ss, token, ',')) {
                size_t s = token.find_first_not_of(" \r\n\t");
                size_t e = token.find_last_not_of(" \r\n\t");
                row.push_back((s == string::npos) ? "" : token.substr(s, e - s + 1));
            }

            if (row.size() < 8) continue;

            string course = row[1];
            int day = 0, from = 0, to = 0;
            if (!parseSafeInt(row[2], day) || !parseSafeInt(row[3], from) || !parseSafeInt(row[4], to)) continue;

            string typeStr = row[5];
            string subject = row[6];
            string roomId = row[7];
            string kind = (row.size() >= 9) ? row[8] : "CR";

            Room *r = rooms_.find(roomId);
            if (!r) {
                r = makeRoom(kind, roomId);
                rooms_.insert(r);
            }

            Purpose p = parsePurpose(typeStr);

            Booking *clash = r->conflict(day, from, to);
            if (clash && clash->user.find(course) != string::npos) {
                clash->user += "/" + subject;
                continue;
            }

            if (book(roomId, course + " (" + subject + ")", static_cast<int>(p), 30, day, from, to, true)) {
                count++;
            }
        }
        return count;
    }

    bool saveBookings(const string &filename = "bookings.dat") const {
        ofstream out(filename);
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

    int loadBookings(const string &filename = "bookings.dat") {
        ifstream in(filename);
        if (!in.is_open()) return 0;

        string line;
        int count = 0;
        while (getline(in, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string tag, idStr, roomId, user, purStr, strStr, dayStr, fromStr, toStr;

            getline(ss, tag, ',');
            getline(ss, idStr, ',');
            getline(ss, roomId, ',');
            getline(ss, user, ',');
            getline(ss, purStr, ',');
            getline(ss, strStr, ',');
            getline(ss, dayStr, ',');
            getline(ss, fromStr, ',');
            getline(ss, toStr, ',');

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
};

// TERMINAL CLI 
// Ayush Jugran


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

// PROGRAM ENTRYPOINT 
// Ayush Jugran

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