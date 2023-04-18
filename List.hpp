#pragma once

template<typename Node, size_t N = 0>
class List {
public:
    struct Hook {
        Hook* prev, * next;
    };

protected:
    static inline Hook*& prev(Hook* n) {
        return n->prev;
    }

    static inline Hook*& next(Hook* n) {
        return n->next;
    }

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

        _iterator& operator--() {
            ptr = ptr->prev;
            return *this;
        }

        _iterator operator--(int) {
            auto ret = *this;
            --*this;
            return ret;
        }

        _iterator& operator++() {
            ptr = ptr->next;
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

    List() {
        tail.prev = tail.next = &tail;
    }

    Node& front() const {
        return *(Node*)tail.next;
    }

    Node& back() const {
        return *(Node*)tail.prev;
    }

    Node* insert(Node* node, std::function<bool(const Node* n)> f) {
        return insert(node, &*find_first_of(f));
    }

    Node* insert(Node* node, Hook* dest) {
        ++_size;
        if (dest == nullptr)
            dest = &tail;
        prev(node) = prev(dest);
        next(node) = dest;
        return (Node*)(prev(dest) = next(prev(dest)) = node);
    }

    Node* push_back(Node* node) {
        return insert(node, nullptr);
    }

    void erase(Node* node, bool destroy) {
        --_size;
        next(prev(node)) = next(node);
        prev(next(node)) = prev(node);
        if (destroy)
            delete node;
    }

    void clear(bool destroy) {
        _size = 0;
        if (destroy) {
            for (auto node = tail.next; node != &tail;) {
                auto x = next(node);
                delete (Node*)node;
                node = x;
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

    iterator find_first_of(std::function<bool(const Node* n)> f) {
        auto it = begin();
        for (; it != end() && !f(&*it); ++it) {}
        return it;
    }

    void erase(iterator it, bool destroy) {
        return erase(&*it, destroy);
    }
};
