/*
Author: Ayesha 
Description: Implementation of room subclasses and makeRoom factory.
 */

#include "DerivedRooms.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

LectureTheatre::LectureTheatre(std::string id, int cap, bool projector)
    : Room(std::move(id), cap), hasProjector_(projector) {}

std::string LectureTheatre::typeName() const { return "Lecture Theatre"; }

bool LectureTheatre::allows(Purpose p) const {
    return p == Purpose::Lecture || p == Purpose::Tutorial;
}

void LectureTheatre::describe() const {
    std::cout << std::left << std::setw(8) << id_
              << std::setw(18) << typeName()
              << "Cap: " << std::setw(5) << capacity_
              << "Feature: " << (hasProjector_ ? "Dual Projectors" : "Standard") << "\n";
}

Classroom::Classroom(std::string id, int cap, bool smartboard)
    : Room(std::move(id), cap), hasSmartboard_(smartboard) {}

std::string Classroom::typeName() const { return "Classroom"; }

bool Classroom::allows(Purpose p) const {
    return p != Purpose::Practical;
}

void Classroom::describe() const {
    std::cout << std::left << std::setw(8) << id_
              << std::setw(18) << typeName()
              << "Cap: " << std::setw(5) << capacity_
              << "Feature: " << (hasSmartboard_ ? "Interactive Smartboard" : "Whiteboard") << "\n";
}

Lab::Lab(std::string id, int cap, int computers)
    : Room(std::move(id), cap), numComputers_(computers) {}

std::string Lab::typeName() const { return "Lab"; }

bool Lab::allows(Purpose p) const {
    return p == Purpose::Practical || p == Purpose::Project;
}

void Lab::describe() const {
    std::cout << std::left << std::setw(8) << id_
              << std::setw(18) << typeName()
              << "Cap: " << std::setw(5) << capacity_
              << "Feature: " << numComputers_ << " Workstations\n";
}

Auditorium::Auditorium(std::string id, int cap, bool sound)
    : Room(std::move(id), cap), hasSoundSystem_(sound) {}

std::string Auditorium::typeName() const { return "Auditorium"; }

bool Auditorium::allows(Purpose p) const {
    return p == Purpose::Lecture || p == Purpose::Project;
}

void Auditorium::describe() const {
    std::cout << std::left << std::setw(8) << id_
              << std::setw(18) << typeName()
              << "Cap: " << std::setw(5) << capacity_
              << "Feature: " << (hasSoundSystem_ ? "Dolby Audio System" : "Basic PA") << "\n";
}

Room *makeRoom(const std::string &kind, const std::string &id, int cap) {
    std::string k = kind;
    std::transform(k.begin(), k.end(), k.begin(), ::toupper);

    if (k == "LT" || k.find("THEATRE") != std::string::npos) {
        return new LectureTheatre(id, cap > 0 ? cap : 120);
    } else if (k == "LAB" || k.find("LABORATORY") != std::string::npos) {
        return new Lab(id, cap > 0 ? cap : 40);
    } else if (k == "AUD" || k == "AUDITORIUM" || k.find("AUDI") != std::string::npos) {
        return new Auditorium(id, cap > 0 ? cap : 500);
    }
    return new Classroom(id, cap > 0 ? cap : 60);
}
