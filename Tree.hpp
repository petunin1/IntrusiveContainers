#pragma once

template<typename Node, size_t N = 0>
class Tree {
public:
    struct Hook { // n = node now, p = parent, g = grandparent, r = great-grandparent, k = kid, c = close nephew, d = distant nephew
        Node* parent, * child[2]; // child nodes: false = smaller, true = larger
    };

protected:
    Node* root = nullptr;
    
    static inline Node*& parent(Node* n) {
        return ((Hook*)n)->parent;
    }

    static inline Node*(& child(Node* n))[2] {
        return ((Hook*)n)->child;
    }

    static inline bool side(Node* n, Node* p) { // returns the side of parent on which the node is located
        assert(parent(n) == p && (n == child(p)[false] || n == child(p)[true]));
        return n == child(p)[true] ? true : false;
    }

    virtual inline void swapAuxiliary(Node* a, Node* b) = 0;

    void rotate(Node* n, Node* p, bool dir) { // n replaces p, moving it down in the direction dir
        assert(parent(n) == p && child(p)[!dir] == n);
        auto g = parent(p);
        parent(n) = g;
        if (g == nullptr)
            root = n;
        else
            child(g)[side(p, g)] = n;
        auto k = child(n)[dir];
        child(p)[!dir] = k;
        if (k != nullptr)
            parent(k) = p;
        child(n)[dir] = p;
        parent(p) = n;
    }

    void rotate2(Node* n, Node* p, Node* g, bool dir) { // double rotation: rotate(n, p, !dir) and rotate(n, g, dir)
        assert(parent(n) == p && child(p)[dir] == n && parent(p) == g && child(g)[!dir] == p);
        auto r = parent(g);
        parent(n) = r;
        if (r == nullptr)
            root = n;
        else
            child(r)[side(g, r)] = n;
        auto k1 = child(n)[dir];
        child(g)[!dir] = k1;
        if (k1 != nullptr)
            parent(k1) = g;
        auto k2 = child(n)[!dir];
        child(p)[dir] = k2;
        if (k2 != nullptr)
            parent(k2) = p;
        child(n)[dir] = g;
        child(n)[!dir] = p;
        parent(g) = parent(p) = n;
    }

    Node* extremum(bool side) { // false to return the smallest element, true to return the largest element
        auto n = root;
        if (n != nullptr)
            while (true) {
                auto x = child(n)[side];
                if (x == nullptr)
                    break;
                n = x;
            }
        return n;
    }

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
            : ptr(ptr) {}

        _Node& operator*() {
            return *(_Node*)ptr;
        }

        _Node* operator->() {
            return (_Node*)ptr;
        }

        _iterator& operator--() {
            ptr = step(ptr, false);
            return *this;
        }

        _iterator operator--(int) {
            auto ret = *this;
            --*this;
            return ret;
        }

        _iterator& operator++() {
            ptr = step(ptr, true);
            return *this;
        }

        _iterator operator++(int) {
            auto ret = *this;
            ++*this;
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

    std::function<bool(const Node* a, const Node* b)> greater;

    static Node* step(Node* n, bool dir) {
        auto x = child(n)[dir];
        if (x != nullptr) {
            n = x;
            while (true) {
                x = child(n)[!dir];
                if (x == nullptr)
                    return n;
                n = x;
            }
        }
        while (true) {
            x = n->parent;
            if (x == nullptr)
                return nullptr;
            if (child(x)[!dir] == n)
                return x;
            n = x;
        }
    }

    Tree() {}

    Tree(std::function<bool(const Node* a, const Node* b)> greater)
        : greater(greater) {}

    std::tuple<Node*, Node*, bool> find(std::function<bool(const Node* n)> greater, std::function<bool(const Node* n)> equal) { // returns: pointer to the node found (nullptr if not found), pointer to the parent if not found, direction from the parent if not found
        if (root == nullptr)
            return { nullptr, nullptr, false };
        auto n = root;
        while (true) {
            if (equal(n))
                return { n, nullptr, false };
            auto dir = greater(n);
            auto k = child(n)[dir];
            if (k == nullptr)
                return { nullptr, n, dir };
            n = k;
        }
    }

    void insert(Node* n) {
        if (root == nullptr)
            return insert(n, nullptr, false);
        auto p = root;
        while (true) {
            auto dir = greater(n, p);
            auto x = child(p)[dir];
            if (x == nullptr)
                return insert(n, p, dir);
            p = x;
        }
    }

    virtual void erase(Node* n) {
        if (child(n)[false] != nullptr && child(n)[true] != nullptr) { // n has two children
            auto x = child(n)[true];
            while (true) {
                auto x2 = child(x)[false];
                if (x2 == nullptr)
                    break;
                x = x2;
            }
            // swapping values of n and next in order node x
            auto px = parent(x);
            auto family = px == n;
            auto p = parent(n);
            if (p == nullptr)
                root = x;
            else
                child(p)[side(n, p)] = x;
            parent(child(n)[false]) = x;
            auto kx = child(x)[true];
            if (kx != nullptr)
                parent(kx) = n;
            if (!family) {
                parent(child(n)[true]) = x;
                child(px)[false] = n;
            }
            std::swap(parent(n), parent(x));
            std::swap(child(n), child(x));
            swapAuxiliary(n, x);
            if (family) {
                parent(n) = x;
                child(x)[true] = n;
            }
        } else if (n == root && child(n)[false] == nullptr && child(n)[true] == nullptr) { // n is childless root
            root = nullptr;
            return;
        }
        eraseBottom(n);
    }
    
    virtual void insert(Node* n, Node* p, bool dir) = 0;
    virtual void eraseBottom(Node* n) = 0; // not childless root, has up to one child

    void clear() {
        root = nullptr;
    }

    bool empty() {
        return root == nullptr;
    }

    iterator begin() {
        return iterator(extremum(false));
    }

    iterator end() {
        return iterator(nullptr);
    }

    const_iterator begin() const {
        return const_iterator(extremum(false));
    }

    const_iterator end() const {
        return const_iterator(nullptr);
    }

    void erase(iterator it) {
        return erase(&*it);
    }
};
