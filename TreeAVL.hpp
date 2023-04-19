#pragma once

#include "Tree.hpp"

template<typename Node, size_t N = 0>
class TreeAVL : public Tree<Node, N> {
public:
    using Tree<Node, N>::root;
    using Tree<Node, N>::side;
    using Tree<Node, N>::rotate;
    using Tree<Node, N>::rotate2;
    using Tree<Node, N>::insert;
    using Tree<Node, N>::parent;
    using Tree<Node, N>::child;

    typedef int8_t balance_t;

    struct Hook : public Tree<Node, N>::Hook {
        balance_t balance;
    };

    static inline balance_t sign(bool dir) {
        return dir ? (balance_t)1 : (balance_t)-1;
    }

protected:
    static inline balance_t& balance(Node* n) {
        return ((Hook*)n)->balance;
    }

    virtual inline void swapAuxiliary(Node* a, Node* b) override {
        std::swap(balance(a), balance(b));
    }

    inline void rotateAVL(Node* n, Node* p, bool dir) { // temporary balance is -2/2 for p (recorded as -1/1), 0|-1/0|1 for n, rotation is right/left
        rotate(n, p, dir);
        if (balance(n) == 0) { // only during deletion
            balance(p) = -sign(dir);
            balance(n) = sign(dir);
        } else // balance(n) == !dir; during insertion and deletion
            balance(p) = balance(n) = 0;
    }

    inline void rotate2AVL(Node* n, Node* p, Node* g, bool dir) { // temporary balance is -2/2 for g (recorded as -1/1), 1/-1 for p, rotation is right/left
        rotate2(n, p, g, dir);
        // balance(n) can be 0 only during deletion, 1/-1 during insertion and deletion
        auto b = sign(dir);
        balance(g) = balance(n) == -b ? b : 0;
        balance(p) = balance(n) == b ? -b : 0;
        balance(n) = 0;
    }

public:
    TreeAVL() {}

    TreeAVL(std::function<bool(const Node* a, const Node* b)> greater)
        : Tree<Node, N>(greater) {}

    virtual void insert(Node* n, Node* p, bool dir) override {
        parent(n) = p;
        child(n)[false] = child(n)[true] = nullptr;
        balance(n) = 0;
        if (p == nullptr) {
            root = n;
            return;
        }
        child(p)[dir] = n;
        if ((balance(p) += sign(dir)) == 0)
            return;
        while (true) {
            if ((p = parent(n = p)) == nullptr) // updating the balance of p; the height of n has increased
                return;
            dir = side(n, p);
            if (balance(p) != 0) {
                if (balance(p) == sign(dir)) {
                    if (balance(n) == sign(dir))
                        rotateAVL(n, p, !dir);
                    else // balance(n) == -sign(dir)
                        rotate2AVL(child(n)[!dir], n, p, !dir);
                } else
                    balance(p) = 0;
                return;
            }
            balance(p) = sign(dir);
        }
    }

    virtual void eraseBottom(Node* n) override {
        auto p = parent(n);
        Node* k = nullptr;
        if (balance(n) != 0 && (parent(k = child(n)[balance(n) == 1]) = p) == nullptr) { // n has one child
            root = k;
            return;
        }
        auto dir = side(n, p);
        child(p)[dir] = k;
        while (true) { // updating the balance of p; the height of n has decreased
            if (balance(p) == 0) {
                balance(p) = -sign(dir);
                return;
            } else if (balance(p) == -sign(dir)) {
                auto s = child(p)[!dir];
                if (balance(s) == sign(dir))
                    rotate2AVL(n = child(s)[dir], s, p, dir);
                else {
                    rotateAVL(n = s, p, dir);
                    if (balance(p) != 0)
                        return;
                }
            } else
                balance(n = p) = 0;
            if ((p = parent(n)) == nullptr)
                return;
            dir = side(n, p);
        }
    }
};
