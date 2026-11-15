/*
Author: Ayesha
Description: Specialized room subclasses with hardware attributes and room factory.
 */

#ifndef DERIVED_ROOMS_H
#define DERIVED_ROOMS_H

#include "Room.h"

class LectureTheatre : public Room {
private:
    bool hasProjector_;

public:
    LectureTheatre(std::string id, int cap = 120, bool projector = true);
    std::string typeName() const override;
    bool allows(Purpose p) const override;
    void describe() const override;
};

class Classroom : public Room {
private:
    bool hasSmartboard_;

public:
    Classroom(std::string id, int cap = 60, bool smartboard = true);
    std::string typeName() const override;
    bool allows(Purpose p) const override;
    void describe() const override;
};

class Lab : public Room {
private:
    int numComputers_;

public:
    Lab(std::string id, int cap = 40, int computers = 40);
    std::string typeName() const override;
    bool allows(Purpose p) const override;
    void describe() const override;
};

class Auditorium : public Room {
private:
    bool hasSoundSystem_;

public:
    Auditorium(std::string id, int cap = 500, bool sound = true);
    std::string typeName() const override;
    bool allows(Purpose p) const override;
    void describe() const override;
};

Room *makeRoom(const std::string &kind, const std::string &id, int cap = 0);

#endif
