/*
Author: Ayesha
Description: Implementation of RoomTree BST algorithms.
 */

#include "RoomTree.h"

RoomTree::~RoomTree() {
    destroy(root_);
    root_ = nullptr;
    count_ = 0;
}

void RoomTree::destroy(Node *n) {
    if (!n) return;
    destroy(n->l);
    destroy(n->r);
    delete n->room;
    delete n;
}

Room *RoomTree::find(const std::string &id) const {
    Node *n = root_;
    while (n) {
        if (id == n->room->id()) return n->room;
        n = (id < n->room->id()) ? n->l : n->r;
    }
    return nullptr;
}

void RoomTree::insert(Room *room) {
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
