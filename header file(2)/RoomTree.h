/*
Author: Ayesha 
Description: Binary Search Tree (BST) for fast room lookup and sorted traversal.
 */

#ifndef ROOM_TREE_H
#define ROOM_TREE_H

#include "Room.h"
#include <functional>

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

    static void destroy(Node *n);
    template <class Fn>
    static void walk(Node *n, Fn &fn) {
        if (!n) return;
        walk(n->l, fn);
        fn(*n->room);
        walk(n->r, fn);
    }

public:
    RoomTree() = default;
    ~RoomTree();

    RoomTree(const RoomTree &) = delete;
    RoomTree &operator=(const RoomTree &) = delete;

    Room *find(const std::string &id) const;
    void insert(Room *room);
    int size() const { return count_; }

    template <class Fn>
    void inorder(Fn fn) const { walk(root_, fn); }
};

#endif
