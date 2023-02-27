#pragma once

template<typename Node>
class List {
public:
    struct Hook {
        Hook* prev, * next;
    };

private:
    Hook tail;
    size_t _size = 0;

    template<typename _Hook, typename _Node>
    class _iterator {
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef _Node value_type;
        typedef std::ptrdiff_t difference_type;
        typedef _Node* pointer;
        typedef _Node& reference;

        _Hook* ptr;

        _iterator(_Hook* ptr)
            : ptr(ptr) {
        }

        _Node& operator*() {
            return *(_Node*)ptr;
        }

        _Node* operator->() {
            return (_Node*)ptr;
        }

        _iterator& operator--() {
            ptr = ptr->prev;
            return *this;
        }

        _iterator operator--(int) {
            auto ret = *this;
            ++(*this);
            return ret;
        }

        _iterator& operator++() {
            ptr = ptr->next;
            return *this;
        }

        _iterator operator++(int) {
            auto ret = *this;
            ++(*this);
            return ret;
        }

        bool operator==(const _iterator<_Hook, _Node>& rhs) const {
            return ptr == rhs.ptr;
        }

        bool operator!=(const _iterator<_Hook, _Node>& rhs) const {
            return ptr != rhs.ptr;
        }
    };

public:
    typedef _iterator<Hook, Node> iterator;
    typedef _iterator<const Hook, const Node> const_iterator;

    List() {
        tail.prev = tail.next = &tail;
    }

    Node* insert(Node* node, Hook* dest) {
        ++_size;
        if (dest == nullptr)
            dest = &tail;
        node->prev = dest->prev;
        node->next = dest;
        return (Node*)(dest->prev = dest->prev->next = node);
    }

    void erase(Node* node, bool destroy) {
        --_size;
        node->prev->next = node->next;
        node->next->prev = node->prev;
        if (destroy)
            delete node;
    }

    void clear(bool destroy) {
        _size = 0;
        if (destroy) {
            for (auto node = tail.next; node != &tail;) {
                auto next = node->next;
                delete node;
                node = next;
            }
        }
        tail.prev = tail.next = &tail;
    }

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    iterator begin() {
        return iterator(tail.next);
    }

    iterator end() {
        return iterator(&tail);
    }

    const_iterator begin() const {
        return const_iterator(tail.next);
    }

    const_iterator end() const {
        return const_iterator(&tail);
    }

    Node& front() const {
        return *(Node*)tail.next;
    }

    Node& back() const {
        return *(Node*)tail.prev;
    }

    void erase(iterator it, bool destroy) {
        return erase(&*it, destroy);
    }
};

template<typename Node>
class TreeRB {
public:
    enum colour_t : bool { BLACK, RED };

    struct Hook {
        Node* parent;
        Node* child[2];
        colour_t colour;
    };

    std::function<bool(const Node* a, const Node* b)> greater;
    Node* root = nullptr;

public:
    // n = node now, p = parent, g = grandparent, k = kid, c = close nephew, d = distant nephew
    static inline bool getDir(Node* n, Node* p) { // returns the side of parent on which the node is located
        assert(n->parent = p);
        return n == p->child[true] ? true : false;
    }

    static Node* step(Node* n, bool side) {
        auto x = n->child[side];
        if (x != nullptr) {
            n = x;
            while (true) {
                x = n->child[!side];
                if (x == nullptr)
                    return n;
                n = x;
            }
        }
        while (true) {
            x = n->parent;
            if (x == nullptr)
                return nullptr;
            if (x->child[!side] == n)
                return x;
            n = x;
        }
    }

    TreeRB() {}

    TreeRB(std::function<bool(const Node* a, const Node* b)> greater)
        : greater(greater) {}

    Node* rotate(Node* p, bool dir) { // move p down in direction dir
        auto g = p->parent;
        auto n = p->child[!dir]; // n takes place of p
        if (g != nullptr)
            g->child[getDir(p, g)] = n;
        else
            root = n;
        assert(n != nullptr);
        auto k = n->child[dir];
        p->child[!dir] = k;
        if (k != nullptr)
            k->parent = p;
        n->child[dir] = p;
        p->parent = n;
        n->parent = g;
        return n;
    }

    std::tuple<Node*, Node*, bool> find(std::function<bool(const Node* n)> greater, std::function<bool(const Node* n)> equal) { // returns: pointer to the node found (nullptr if not found), pointer to the parent if not found, direction from the parent if not found
        if (root == nullptr)
            return { nullptr, nullptr, false };
        auto n = root;
        while (true) {
            if (equal(n))
                return { n, nullptr, false };
            auto dir = greater(n);
            auto k = n->child[dir];
            if (k == nullptr)
                return { nullptr, n, dir };
            n = k;
        }
    }

    Node* insert(Node* n) {
        if (root == nullptr)
            return insert(n, nullptr, false);
        auto p = root;
        bool dir;
        while (true) {
            dir = greater(n, p);
            auto k = p->child[dir];
            if (k == nullptr)
                return insert(n, p, dir);
            p = k;
        }
    }

    Node* insert(Node* n, Node* p, bool dir) {
        n->colour = RED;
        n->child[false] = n->child[true] = nullptr;
        n->parent = p;
        if (p == nullptr) {
            root = n;
            return n;
        }
        p->child[dir] = n;
        while (p->colour == RED) { // p red
            auto g = p->parent;
            if (g == nullptr) { // p red root
                p->colour = BLACK;
                return n;
            }
            auto dir2 = getDir(p, g);
            auto u = g->child[!dir2];
            if (u == nullptr || u->colour == BLACK) { // p red, u black
                if (dir != dir2) { // n is the inner grandchild of g
                    rotate(p, dir2);
                    std::swap(n, p);
                }
                rotate(g, !dir2);
                p->colour = BLACK;
                g->colour = RED;
                return n;
            }
            // p red, u red
            p->colour = u->colour = BLACK;
            (n = g)->colour = RED;
            if ((p = n->parent) == nullptr)
                return n;
        };
        return nullptr;
    }

    void erase(Node* n) { // the black height of paths passing through n is always less than the rest by one
        if (n->child[false] != nullptr && n->child[true] != nullptr) { // n with two children
            auto x = n->child[true];
            while (true) {
                auto x2 = x->child[false];
                if (x2 == nullptr)
                    break;
                x = x2;
            }
            // swapping values of n and next in order node x
            auto px = x->parent;
            auto family = px == n;
            auto p = n->parent;
            if (p == nullptr)
                root = x;
            else
                p->child[getDir(n, p)] = x;
            n->child[false]->parent = x;
            auto kx = x->child[true];
            if (kx != nullptr)
                kx->parent = n;
            if (!family) {
                n->child[true]->parent = x;
                px->child[false] = n;
            }
            std::swap(n->parent, x->parent);
            std::swap(n->child, x->child);
            std::swap(n->colour, x->colour);
            if (family) {
                n->parent = x;
                x->child[true] = n;
            }
        } else if (n == root && n->child[false] == nullptr && n->child[true] == nullptr) { // n childless root
            root = nullptr;
            return;
        }
        if (n->colour == RED) { // n red => without children => non-root
            auto p = n->parent;
            p->child[getDir(n, p)] = nullptr;
            return;
        } else { // n black with up to one child (=> red)
            for (auto dir : { false, true }) {
                auto k = n->child[dir];
                if (k != nullptr) {
                    k->colour = BLACK;
                    auto p = n->parent;
                    if (p == nullptr)
                        root = n;
                    else
                        k->parent = p;
                    p->child[getDir(n, p)] = k;
                    return;
                }
            }
        }
        // black without children
        Node* s, * c, * d;
        auto p = n->parent;
        auto dir = getDir(n, p);
        p->child[dir] = nullptr;
        goto start_d;
        while (true) {
            dir = getDir(n, p);
        start_d:
            s = p->child[!dir];
            d = s->child[!dir];
            c = s->child[dir];
            if (s->colour == RED)
                goto case_s;
            // s black
            if (d != nullptr && d->colour == RED)
                goto case_d;
            if (c != nullptr && c->colour == RED)
                goto case_c;
            if (p->colour == RED)
                goto case_p;
            // p c s d black
            s->colour = RED;
            n = p;
            if ((p = n->parent) == nullptr)
                return; // n is root
        }
    case_s: // s red, p c d black
        rotate(p, dir);
        p->colour = RED;
        s->colour = BLACK;
        s = c;
        // p red, s black
        d = s->child[!dir];
        if (d != nullptr && d->colour == RED)
            goto case_d;
        c = s->child[dir];
        if (c != nullptr && c->colour == RED)
            goto case_c;
    case_p: // p red, s c d black
        s->colour = RED;
        p->colour = BLACK;
        return;
    case_c: // c red, s d black
        rotate(s, !dir);
        s->colour = RED;
        c->colour = BLACK;
        d = s;
        s = c;
    case_d: // d red, s black
        rotate(p, dir);
        s->colour = p->colour;
        p->colour = BLACK;
        d->colour = BLACK;
    }

    void clear() {
        root = nullptr;
    }

    bool empty() {
        return root == nullptr;
    }
};
