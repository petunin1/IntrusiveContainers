#pragma once

template<typename Node, size_t N = 0>
class List {
public:
    struct Hook {
        Hook* link[2];
    };

protected:
    static inline Hook*& step(Hook* n, bool dir) {
        return n->link[dir];
    }

    Hook tail;
    size_t _size = 0;

    template<typename _Hook, typename _Node, bool left>
    class _iterator {
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef _Node value_type;
        typedef std::ptrdiff_t difference_type;
        typedef _Node* pointer;
        typedef _Node& reference;

        _Hook* ptr;

        _iterator()
            : ptr(nullptr) {}

        _iterator(_Hook* ptr)
            : ptr(ptr) {}

        _Node& operator*() {
            return *(_Node*)ptr;
        }

        _Node* operator->() {
            return (_Node*)ptr;
        }

        bool operator==(const _iterator<_Hook, _Node, left>& rhs) const {
            return ptr == rhs.ptr;
        }

        bool operator!=(const _iterator<_Hook, _Node, left>& rhs) const {
            return ptr != rhs.ptr;
        }

        _iterator& operator--() {
            ptr = ptr->link[!left];
            return *this;
        }

        _iterator operator--(int) {
            auto ret = *this;
            --*this;
            return ret;
        }

        _iterator& operator++() {
            ptr = ptr->link[left];
            return *this;
        }

        _iterator operator++(int) {
            auto ret = *this;
            ++*this;
            return ret;
        }
    };

public:
    typedef _iterator<Hook, Node, true> iterator;
    typedef _iterator<const Hook, const Node, true> const_iterator;
    typedef _iterator<Hook, Node, false> reverse_iterator;
    typedef _iterator<const Hook, const Node, false> const_reverse_iterator;

    List() {
        tail.link[false] = tail.link[true] = &tail;
    }

    Node& front() const {
        return *(Node*)tail.link[true];
    }

    Node& back() const {
        return *(Node*)tail.link[false];
    }

    Node* insert(Node* node, std::function<bool(const Node* n)> f) {
        return insert(node, &*find_first_of(f));
    }

    Node* insert(Node* node, Hook* dest) {
        ++_size;
        if (dest == nullptr)
            dest = &tail;
        step(node, false) = step(dest, false);
        step(node, true) = dest;
        return (Node*)(step(dest, false) = step(step(dest, false), true) = node);
    }

    Node* push_back(Node* node) {
        return insert(node, nullptr);
    }

    void erase(Node* node, bool destroy) {
        --_size;
        step(step(node, false), true) = step(node, true);
        step(step(node, true), false) = step(node, false);
        if (destroy)
            delete node;
    }

    void clear(bool destroy) {
        _size = 0;
        if (destroy) {
            for (auto node = step(&tail, true); node != &tail;) {
                auto x = step(node, true);
                delete (Node*)node;
                node = x;
            }
        }
        tail.link[false] = tail.link[true] = &tail;
    }

    size_t size() const {
        return _size;
    }

    bool empty() const {
        return _size == 0;
    }

    iterator begin() {
        return iterator(tail.link[true]);
    }

    iterator end() {
        return iterator(&tail);
    }

    const_iterator begin() const {
        return const_iterator(tail.link[true]);
    }

    const_iterator end() const {
        return const_iterator(&tail);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(tail.link[false]);
    }

    reverse_iterator rend() {
        return reverse_iterator(&tail);
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(tail.link[false]);
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(&tail);
    }

    iterator find_first_of(std::function<bool(const Node* n)> f) {
        auto it = begin();
        for (; it != end() && !f(&*it); ++it) {}
        return it;
    }

    reverse_iterator find_last_of(std::function<bool(const Node* n)> f) {
        auto it = rbegin();
        for (; it != rend() && !f(&*it); ++it) {}
        return it;
    }

    void erase(iterator it, bool destroy) {
        return erase(&*it, destroy);
    }

    void erase(reverse_iterator it, bool destroy) {
        return erase(&*it, destroy);
    }
};
