#pragma once

#include "Tree.hpp"

template<typename Node, size_t N = 0>
class TreeAVL : public Tree<Node, N> {
public:
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
        std::cout << "    r1" << std::endl;
        rotate(n, p, dir);
        assert(balance(p) == -sign(dir));
        if (balance(n) == 0) { // only during deletion
            balance(p) = -sign(dir);
            balance(n) = sign(dir);
        } else { // during deletion and insertion
            assert(balance(n) == -sign(dir));
            balance(p) = balance(n) = 0;
        }
    }

    inline void rotate2AVL(Node* n, Node* p, Node* g, bool dir) { // temporary balance is -2/2 for g (recorded as -1/1), 1/-1 for p, rotation is right/left
        std::cout << "    r2" << std::endl;
        rotate2(n, p, g, dir);
        assert(balance(g) == -sign(dir) && balance(p) == sign(dir));
        if (balance(n) == 0) { // only during deletion
            balance(g) = balance(p) = 0;
        } else { // during deletion and insertion
            if (balance(n) == -sign(dir)) {
                balance(g) = sign(dir);
                balance(p) = 0;
            } else {
                balance(g) = 0;
                balance(p) = -sign(dir);
            }
        }
        balance(n) = 0;
    }

public:
    using Tree<Node, N>::root;
    using Tree<Node, N>::side;
    using Tree<Node, N>::rotate;
    using Tree<Node, N>::rotate2;
    using Tree<Node, N>::insert;
    using Tree<Node, N>::parent;
    using Tree<Node, N>::child;

    TreeAVL() {}

    TreeAVL(std::function<bool(const Node* a, const Node* b)> greater)
        : Tree<Node, N>(greater) {}

    virtual void insert(Node* n, Node* p, bool dir) {
        std::cout << "insert" << std::endl;
        parent(n) = p;
        child(n)[false] = child(n)[true] = nullptr;
        balance(n) = 0;
        if (p == nullptr) {
            root = n;
            return;
        }
        assert(balance(p) != 1 || child(p)[false] == nullptr && child(p)[true] != nullptr);
        assert(balance(p) != -1 || child(p)[true] == nullptr && child(p)[false] != nullptr);
        assert(balance(p) != 0 || child(p)[false] == nullptr && child(p)[true] == nullptr);
        assert(balance(p) != sign(dir));
        child(p)[dir] = n;
        if ((balance(p) += sign(dir)) == 0)
            return;
        for (auto g = parent(p); g != nullptr; g = parent(p = g)) { // updating the balance of g; the height of p has increased
            dir = side(p, g);
            if (balance(g) == sign(dir)) {
                if (balance(p) == -sign(dir))
                    rotate2AVL(child(p)[!dir], p, g, !dir);
                else // balance(p) == sign(dir)
                    rotateAVL(p, g, !dir);
                break;
            }
            if (balance(g) != 0) {
                balance(g) = 0;
                break;
            }
            balance(g) = sign(dir);
        }
    }

    virtual void eraseBottom(Node* n) override {
        std::cout << "delete" << std::endl;
        auto p = parent(n);
        if (balance(n) != 0) { // has one child
            auto k = child(n)[balance(n) == 1];
            assert(k != nullptr && child(n)[balance(n) != 1] == nullptr);
            parent(k) = p;
            if (p == nullptr) {
                root = k;
                return;
            }
            n = child(p)[side(n, p)] = k;
        } else {
            auto dir = side(n, p);
            child(p)[dir] = nullptr;
            if ((balance(p) -= sign(dir)) != 0)
                return;
            p = parent(n = p);
        }
        for (; p != nullptr; p = parent(n = p)) { // updating the balance of g; the height of p has decreased
            auto dir = side(n, p);
            if (balance(p) == -sign(dir)) {
                auto s = child(p)[!dir];
                if (balance(s) == sign(dir))
                    rotate2AVL(child(s)[dir], s, p, dir);
                else {
                    rotate(s, p, dir);
                    if (balance(p) != 0)
                        break;
                }
            } else if (balance(p) == 0) {
                balance(p) = -sign(dir);
                break;
            } else
                balance(p) = 0;
        }
    }
};
