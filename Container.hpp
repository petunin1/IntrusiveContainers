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

    ~List() {}

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

    iterator insert(iterator it, Hook* hook) {
        ++_size;
        hook->prev = it->prev;
        hook->next = &*it;
        return iterator(it->prev = it->prev->next = hook);
    }

    void erase(iterator it, bool destroy = false) {
        --_size;
        it->prev->next = it->next;
        it->next->prev = it->prev;
        if (destroy)
            delete&* it;
    }

    void clear(bool destroy = false) {
        _size = 0;
        if (destroy) {
            for (auto hook = tail.next; hook != &tail;) {
                auto next = hook->next;
                delete (Node*)hook;
                hook = next;
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
};

template<typename Node>
class TreeRB {
public:
    enum colour_t : bool { BLACK, RED };

    struct Hook {
        Hook* parent;
        Hook* child[2];
        colour_t colour;
    };

    std::function<bool(const Node* a, const Node* b)> greater;

private:
    Hook* root = nullptr;

    // n = node now, p = parent, g = grandparent, k = kid, c = close nephew, d = distant nephew
    static bool getDir(Hook* n) { // returns the side of parent on which the hook is located
        return n == (n->parent)->child[true] ? true : false;
    }

    Hook* rotate(Hook* p, bool dir) { // move p down in direction dir
        auto g = p->parent;
        auto n = p->child[!dir]; // n takes place of p
        assert(n != nullptr);
        auto k = n->child[dir];
        p->child[!dir] = k;
        if (k != nullptr)
            k->parent = p;
        n->child[dir] = p;
        p->parent = n;
        n->parent = g;
        if (g != nullptr)
            g->child[p == g->child[true] ? true : false] = n;
        else
            root = n;
        return n;
    }

    void insert(Hook* n, Hook* p, bool dir) {
        n->colour = RED;
        n->child[false] = n->child[true] = nullptr;
        n->parent = p;
        if (p == nullptr) {
            root = n;
            return;
        }
        p->child[dir] = n;
        while (p->colour == RED) { // p red
            auto g = p->parent;
            if (g == nullptr) { // p red root
                p->colour = BLACK;
                return;
            }
            auto dir2 = getDir(p);
            auto u = g->child[!dir2];
            if (u == nullptr || u->colour == BLACK) { // p red, u black
                if (dir != dir2) { // n is the inner grandchild of g
                    rotate(p, dir2);
                    std::swap(n, p);
                }
                rotate(g, !dir2);
                p->colour = BLACK;
                g->colour = RED;
                return;
            }
            // p red, u red
            p->colour = u->colour = BLACK;
            (n = g)->colour = RED;
            if ((p = n->parent) == nullptr)
                break;
        };
    }

    void erase(Hook* n) {
        if (n == root && n->child[false] == nullptr && n->child[true] == nullptr) { // n childless root
            root = nullptr;
            return;
        }
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
                p->child[getDir(n)] = x;
            n->child[false]->parent = x;
            kx = x->child[true];
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
            n = x; // new node has up to one child (=> right)
        }
        if (n->colour == RED) { // n red => without children => non-root
            n->parent->child[getDir(n)] = nullptr;
            return;
        } else { // n black with up to one child (=> red)
            for (auto dir : { false, true }) {
                auto k = n->child[dir];
                if (k != nullptr) {
                    k->colour = BLACK;
                    auto p = n->parent;
                    k->parent = p;
                    p->child[getDir(n)] = k;
                    return;
                }
            }
        }
        // black without children
        Hook* s, * c, * d;
        auto dir = getDir(n);
        auto p = n->parent;
        p->child[dir] = nullptr;
        goto start_d;
        do {
            dir = getDir(n);
        start_d:
            s = p->child[!dir];
            d = s->child[!dir];
            c = s->child[dir];
            if (s->colour == RED)
                goto case_d3;
            // s black
            if (d != nullptr && d->colour == RED)
                goto case_d6;
            if (c != nullptr && c->colour == RED)
                goto case_d5;
            if (p->colour == RED)
                goto case_d4;
            // p c s d black
            s->colour = RED;
            n = p;
        } while ((p = n->parent) != nullptr);
    case_d3: // s red, p c d black
        rotate(p, dir);
        p->colour = RED;
        s->colour = BLACK;
        s = c;
        // p red, s black
        d = s->child[!dir];
        if (d != nullptr && d->colour == RED)
            goto case_d6;
        c = s->child[dir];
        if (c != nullptr && c->colour == RED)
            goto case_d5;
    case_d4: // p red, s c d black
        s->colour = RED;
        p->colour = BLACK;
        return;
    case_d5: // c red, s d black
        rotate(s, !dir);
        s->colour = RED;
        c->colour = BLACK;
        d = s;
        s = c;
    case_d6: // d red, s black
        rotate(p, dir);
        s->colour = p->colour;
        p->colour = BLACK;
        d->colour = BLACK;
    }

public:
    TreeRB() {}

    TreeRB(std::function<bool(const Node* a, const Node* b)> greater)
        : greater(greater){}

    Node* find(std::function<bool(const Node* n)> greater, std::function<bool(const Node* n)> equal) {
        for (auto n = root; n != nullptr; n = n->child[greater((Node*)n)])
            if (equal((Node*)n))
                return (Node*)n;
        return nullptr;
    }

    Node* insert(Node* n) {
        if (root == nullptr) {
            insert((Hook*)n, nullptr, false);
            return n;
        }
        auto p = root;
        bool dir;
        while (true) {
            dir = greater(n, (Node*)p);
            auto k = p->child[dir];
            if (k == nullptr) {
                insert((Hook*)n, p, dir);
                return n;
            }
            p = k;
        }
    }

    void erase(Node* n, bool destroy = false) {
        erase((Hook*)n);
        if (destroy)
            delete n;
    }

    bool empty() {
        return root == nullptr;
    }
};
