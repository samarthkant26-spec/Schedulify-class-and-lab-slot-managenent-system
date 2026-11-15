# Schedulify

**A Smart Academic Classroom and Laboratory Slot Management System**

A C++17 command-line application that manages room bookings for classrooms, lecture theatres, labs and auditoriums. It validates every request, detects time clashes, keeps a FIFO waiting list for clashed requests, and automatically promotes a waiting request when a booking is cancelled.

**Institution:** Graphic Era Hill University (GEHU), Dehradun, Department of CSE
**Team:** Alpha squad

---

## Features

- Room booking with a 5-tier validation pipeline (room exists, valid input, purpose allowed, capacity, clash check)
- Clash detection using per-day sorted linked lists with early exit
- FIFO waiting list per room: clashed requests are queued automatically
- Auto-promotion: cancelling a booking promotes the earliest waiting request that fits the freed slot
- Day view and week grid view for any room, including free periods
- Free room search by day, period range, minimum capacity and purpose
- Timetable import from CSV (unknown rooms are created automatically)
- Persistence: bookings and waiting lists are saved to and loaded from `bookings.dat`
- Room listing sorted by room ID using a Binary Search Tree

---

## Data Structures and OOP Concepts

| Concept | Where it is used |
|---|---|
| Sorted singly linked list | Per-day booking slots inside each `Room` |
| Linked list (registry) | `BookingList`: global lookup of bookings by ID |
| Queue (FIFO, linked) | `WaitQueue`: waiting requests per room |
| Binary Search Tree | `RoomTree`: stores rooms keyed by room ID |
| Abstract class and polymorphism | `Room` with `LectureTheatre`, `Classroom`, `Lab`, `Auditorium` |
| Factory function | `makeRoom()` creates the right room type from a string |
| Templates and lambdas | `RoomTree::inorder()`, `WaitQueue::extractFitting()` |

---

## Room Types and Allowed Purposes

| Room Type | Default Capacity | Lecture | Tutorial | Practical | Project |
|---|---|:-:|:-:|:-:|:-:|
| Lecture Theatre | 120 | Yes | Yes | No | No |
| Classroom | 60 | Yes | Yes | No | Yes |
| Lab | 40 | No | No | Yes | Yes |
| Auditorium | 500 | Yes | No | No | Yes |

**Pre-loaded rooms:** `124`, `224`, `206` (Lecture Theatres), `117`, `114` (Classrooms), `009`, `209` (Labs), `529` (Auditorium)

---

## Day and Period Schedule

Days: `1 = Mon, 2 = Tue, 3 = Wed, 4 = Thu, 5 = Fri, 6 = Sat`

| Period | Time | Period | Time |
|---|---|---|---|
| P1 | 08:00 - 08:55 | P6 | 12:55 - 13:50 |
| P2 | 08:55 - 09:50 | P7 | 14:10 - 15:05 |
| P3 | 10:10 - 11:05 | P8 | 15:05 - 16:00 |
| P4 | 11:05 - 12:00 | P9 | 16:00 - 16:55 |
| P5 | 12:00 - 12:55 | P10 | 16:55 - 17:50 |

---

## Project Structure

```
Schedulify/
 include/
    Config.h
    Booking.h
    WaitQueue.h
    DerivedRooms.h
    RoomTree.h
    BookingEngine.h
    Room.h
   src/
    Booking.cpp
    WaitQueue.cpp
    DerivedRooms.cpp
    RoomTree.cpp
    BookingEngine.cpp
    Room.cpp
    main.cpp
 data/
    timetable.csv
 bookings.dat          
 README.md
```

---

## Build and Run

**Requirements:** a C++17 compiler (g++, clang++ or MSVC)

```bash
# Clone
git clone https://github.com/<your-username>/Schedulify.git
cd Schedulify

# Build
g++ -std=c++17 -Iinclude src/*.cpp -o schedulify

# Run
./schedulify          # Linux / macOS
schedulify.exe        # Windows
```

If you are using the single-file version:

```bash
g++ -std=c++17 schedulify.cpp -o schedulify
```

---

## Usage

```
--- Main Menu ---
  1. Book Room Slot
  2. Cancel Booking (With Waitlist Promotion)
  3. Room Day View & Free Periods
  4. Room Week Grid View
  5. Room Waiting List
  6. Search Free Rooms
  7. List All Rooms (BST)
  8. Import Timetable CSV
  9. Save Bookings (bookings.dat)
  0. Save and Exit
```

### Booking validation order

1. The room must exist
2. Day, period range and purpose code must be valid
3. The room type must allow the purpose
4. Student strength must not exceed room capacity
5. The slot must be free; otherwise the request joins the room's waiting list

### Example

```
Room ID: 124
Booked By: CSE-A
Purpose (1=Lecture, 2=Tutorial, 3=Practical, 4=Project): 1
Student Strength: 90
Day (1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat): 1
From Period (1-10): 1
To Period (1-10): 2

  [Confirmed] Booking #1001: 124 Mon P1-2 (08:00-09:50) for CSE-A
```

Note: the "Booked By" field is read as a single word, so it cannot contain spaces.

---

## File Formats

### Timetable CSV (`data/timetable.csv`)

The first line is treated as a header and skipped. Columns:

```
id,course,day,from,to,type,subject,room,kind
```

Example:

```
id,course,day,from,to,type,subject,room,kind
1,BTech-CSE-A,1,1,2,Lecture,DSA,124,LT
2,BTech-CSE-A,2,3,4,Lab,DSA-Lab,009,LAB
```

- `type` accepts `Lecture`, `Tutorial`, `Practical` or `Lab`, `Project` (or codes 1 to 4)
- `kind` (optional) accepts `LT`, `LAB`, `AUD`; anything else defaults to Classroom
- Unknown rooms are created automatically
- If the same course already holds the slot, the subjects are merged into one booking

### Saved data (`bookings.dat`)

```
B,id,roomId,user,purpose,strength,day,from,to     (confirmed booking)
W,id,roomId,user,purpose,strength,day,from,to     (waiting request)
```

---

## Team and Contributions

**Team Alpha squad**

| Part | Member | Roll No. | Student ID | Files |
|---|---|---|---|---|
| 1 | Samarth Kant (Team Lead) | 2519490 | 25101210534 | `include/Config.h`, `include/Booking.h`, `src/Booking.cpp`, `include/WaitQueue.h`, `src/WaitQueue.cpp` |
| 2 | Ayesha | 2519330 | 25101210496 | `include/DerivedRooms.h`, `src/DerivedRooms.cpp`, `include/RoomTree.h`, `src/RoomTree.cpp` |
| 3 | Mahi Rawat | 2519410 | 25101210458 | `include/BookingEngine.h`, `src/BookingEngine.cpp` |
| 4 | Ayush Jugran | 2519333 | 25101210539 | `include/Room.h`, `src/Room.cpp`, `src/main.cpp` |

### Responsibilities

**Part 1 - Samarth Kant:** Constants and timings (`Config.h`), the `Purpose` enum, the `Booking` node and `BookingList` registry, and the `WaitRequest` node with the FIFO `WaitQueue` (including `extractFitting()` for promotion).

**Part 2 - Ayesha:** The concrete room hierarchy (`LectureTheatre`, `Classroom`, `Lab`, `Auditorium`), the `makeRoom()` factory, and the `RoomTree` Binary Search Tree.

**Part 3 - Mahi Rawat:** The `BookingEngine` class, the 5-tier validator `book()`, the `loadTimetable()` CSV parser, and `saveBookings()` / `loadBookings()` persistence.

**Part 4 - Ayush Jugran:** The abstract `Room` class with per-day sorted linked lists, early-exit `conflict()`, `insert()` and `remove()`; plus `cancelBooking()` with FIFO auto-promotion, the day and week views, free room search, and the CLI main loop in `main.cpp`.

---

## Future Improvements

- Support multi-word names in input fields
- Booking edit and reschedule
- Per-faculty and per-course schedule views
- Automatic suggestion of alternate rooms on a clash
- GUI or web front end

---

## License

Developed as an academic project at Graphic Era Hill University.