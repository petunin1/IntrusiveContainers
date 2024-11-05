## Intrusive containers implementing [list](https://en.wikipedia.org/wiki/Doubly_linked_list), abstract [binary trees](https://en.wikipedia.org/wiki/Binary_search_tree) (that can act as maps and sets), and their [red-black](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree) and [AVL](https://en.wikipedia.org/wiki/AVL_tree) self-balancing forms.

Intrusive containers do not copy objects passed to them and do not allocate or delete. An object can simultaniously be part of several containers, inheriting from their hooks. To differentiate between container classes of the same name, an index in the second template argument needs to be specified.

```
#include <IntrusiveContainers/List.hpp>
#include <IntrusiveContainers/TreeRB.hpp>
#include <IntrusiveContainers/TreeAVL.hpp>

struct Class : public List<Class>::Hook, public TreeRB<Class>::Hook, public TreeRB<Class, 1>::Hook {
    char letter;
    size_t index;
    Class(char letter, size_t index) : letter(letter), index(index) {}
};
```

For Tree classes, a default comparison function between nodes is set in the constructor:

```
List<Class> list;
TreeRB<Class> map1([](auto a, auto b) { return std::tolower(a->letter) > std::tolower(b->letter); });
TreeRB<Class, 1> map2([](auto a, auto b) { return a->index > b->index; });
```

An example of default insertion and iteration:

```
auto a = new Class('a', 1), b = new Class('b', 2), c = new Class('c', 3), d = new Class('d', 4), e = new Class('e', 5);
for (auto x : { a, b, c, d }) {
    map1.insert(x);
    map2.insert(x);
}
for (auto x : { a, b, d })
list.push_back(x);
list.insert(c, d);

printf("Alphabet 1: ");
for (auto it = list.begin(); it != list.end(); ++it)
    printf("letter %c, ", it->letter);
printf("\n");
```

One can search an object in the Tree and, if absent, insert it efficiently using the output of the 'find' method:
```
for (auto v : { c, e }) {
    auto search = map1.find([&](auto x) { return c->letter <=> x->letter; }); // returns pointer to the node found (nullptr if absent), pointer to the parent if absent, direction from the parent if absent
    auto found = search.node;
    if (found == nullptr) {
        printf("Letter %c not found, inserting\n", v->letter);
        list.push_back(v);
        map1.insert(v, search.parent, search.dir); // faster to insert an element into the tree if the parent and direction of insertion are already known
        map2.insert(v);
    } else {
        printf("Letter %c found\n", found->letter);
        found->letter += 'A' - 'a'; // make it a capital letter
    }
}
```

Objects need to be destroyed explicitly after being erased from the containers (or can be destroyed while being erased from the list if the second argument is set to 'true'):
```
printf("Alphabet 2: ");
for (const auto& v : list)
    printf("letter %c, ", v.letter);
printf("\n");

size_t index = 3;
auto search = map2.find([&](auto x) { return index <=> x->index; });
auto found = search.node;
if (found != nullptr)
    printf("Letter with index %llu is %c\n", index, found->letter);

for (auto x : { a, b, c, d, e }) {
    list.erase(x, false);
    map1.erase(x);
    map2.erase(x);
    delete x;
}
```
