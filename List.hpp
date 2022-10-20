#pragma once

template<typename T>
class List : private noncopyable {
    class Link : private noncopyable {
    public:
        Link* prev, * next;

        Link()
            : prev(this)
            , next(this) {}

        Link(Link* prev, Link* next)
            : prev(prev)
            , next(next) {}
    };

    template<typename U>
    class _Node : public Link, public U {
    public:
        template<typename... Args>
        _Node(Link* prev, Link* next, Args... args)
            : Link(prev, next)
            , U(args...) {}
    };

    template<typename L, typename U>
    class _iterator {
    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef U value_type;
        typedef std::ptrdiff_t difference_type;
        typedef U* pointer;
        typedef U& reference;

        L* ptr;

        _iterator()
            : ptr(nullptr) {}

        _iterator(L* ptr)
            : ptr(ptr) {}

        U& operator*() {
            return *(U*)ptr;
        }

        U* operator->() {
            return (U*)ptr;
        }

        _iterator operator++() {
            ptr = ptr->next;
            return *this;
        }

        _iterator operator++(int) {
            auto ret = *this;
            ++(*this);
            return ret;
        }

        bool operator==(const _iterator<L, U>& rhs) const {
            return ptr == rhs.ptr;
        }

        bool operator!=(const _iterator<L, U>& rhs) const {
            return ptr != rhs.ptr;
        }

        operator _iterator<const L, const U>() const {
            return _iterator<const L, const U>(ptr);
        }
    };

public:
    typedef _Node<T> Node;
    typedef _iterator<Link, Node> iterator;
    typedef _iterator<const Link, const Node> const_iterator;

private:
    Link tail{};
    size_t _size = 0;

    template<typename... Args>
    Node* insert(Link* link, Args... args) {
        ++_size;
        return (Node*)(link->prev = link->prev->next = new Node(link->prev, link, args...));
    }

    void erase(Link* link) {
        --_size;
        link->prev->next = link->next;
        link->next->prev = link->prev;
        delete (Node*)link;
    }

public:
    List() {}

    ~List() {
        for (auto link = tail.next; link != &tail;) {
            auto next = link->next;
            delete (Node*)link;
            link = next;
        }
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

    template<typename... Args>
    iterator insert(iterator it, Args... args) {
        return iterator(insert(it.ptr, args...));
    }

    void erase(iterator it) {
        erase(it.ptr);
    }

    void clear() {
        _size = 0;
        for (auto link = tail.next; link != &tail;) {
            auto next = link->next;
            delete (Node*)link;
            link = next;
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
