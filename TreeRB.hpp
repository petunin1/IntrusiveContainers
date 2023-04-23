#pragma once

#include "Tree.hpp"

template<typename Node, size_t N = 0>
class TreeRB : public Tree<Node, N> {
public:
    using Tree<Node, N>::root;
    using Tree<Node, N>::side;
    using Tree<Node, N>::rotate;
    using Tree<Node, N>::rotate2;
    using Tree<Node, N>::insert;
    using Tree<Node, N>::parent;
    using Tree<Node, N>::child;

    enum colour_t : bool { BLACK, RED };

    struct Hook : public Tree<Node, N>::Hook {
        colour_t colour;
    };

protected:
    static inline colour_t& colour(Node* n) {
        return ((Hook*)n)->colour;
    }

    virtual inline void swapAuxiliary(Node* a, Node* b) override {
        std::swap(colour(a), colour(b));
    }

public:
    TreeRB() {}

    TreeRB(std::function<bool(const Node* a, const Node* b)> greater)
        : Tree<Node, N>(greater) {}

    virtual void insert(Node* n, Node* p, bool dir) override {
        parent(n) = p;
        child(n)[false] = child(n)[true] = nullptr;
        colour(n) = RED;
        if (p == nullptr) {
            root = n;
            return;
        }
        child(p)[dir] = n;
        while (colour(p) == RED) { // p red
            auto g = parent(p);
            if (g == nullptr) { // p red root
                colour(p) = BLACK;
                return;
            }
            auto dir2 = side(p, g);
            auto u = child(g)[!dir2];
            if (u == nullptr || colour(u) == BLACK) { // p red, u black
                if (dir == dir2)
                    rotate(p, g, !dir2);
                else {
                    rotate2(n, p, g, !dir2);
                    p = n;
                }
                colour(p) = BLACK;
                colour(g) = RED;
                return;
            }
            // p red, u red
            colour(p) = colour(u) = BLACK;
            colour(n = g) = RED;
            if ((p = parent(n)) == nullptr)
                return;
            dir = side(n, p);
        }
    }

    virtual void eraseBottom(Node* n) override {
        if (colour(n) == RED) { // n red => without children => non-root
            auto p = parent(n);
            child(p)[side(n, p)] = nullptr;
            return;
        } else // n black with up to one child (=> red)
            for (auto dir : { false, true }) {
                auto k = child(n)[dir];
                if (k != nullptr) {
                    auto p = parent(n);
                    parent(k) = p;
                    colour(k) = BLACK;
                    (p == nullptr ? root : child(p)[side(n, p)]) = k;
                    return;
                }
            }
        // n black non-root without children
        auto p = parent(n);
        auto dir = side(n, p);
        child(p)[dir] = nullptr;
        while (true) {
            auto s = child(p)[!dir];
            if (colour(s) == RED) {
                rotate(s, p, dir);
                colour(p) = RED;
                colour(s) = BLACK;
                s = c;
                // p red, s black
            }
            // s black
            auto d = child(s)[!dir];
            if (d != nullptr && colour(d) == RED) { // d red, s black
                rotate(s, p, dir);
                colour(s) = colour(p);
                colour(p) = colour(d) = BLACK;
                return;
            }
            auto c = child(s)[dir];
            if (c != nullptr && colour(c) == RED) { // c red, s d black
                rotate2(c, s, p, dir);
                colour(c) = colour(p);
                colour(p) = BLACK;
                return;
            }
            if (colour(p) == RED) { // p red, s c d black
                colour(s) = RED;
                colour(p) = BLACK;
                return;
            }
            // p c s d black
            colour(s) = RED;
            if ((p = parent(n = p)) == nullptr)
                return;
            dir = side(n, p);
        }
    }
};
