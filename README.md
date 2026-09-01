# Phonebook — BST and Hash Map in C++

A menu-driven contact manager that stores every contact in two data
structures at once: a Binary Search Tree for ordering, and a hash map for
lookup. Each structure does the job the other is bad at.

> **Group project** — BEJ32103 Data Structures and Algorithms,
> Universiti Tun Hussein Onn Malaysia, Semester 2 2024/2025.

---

## The design decision

The obvious implementation is an array or a vector of contacts. That makes
display easy but every search is a linear scan, and keeping the list
alphabetical means re-sorting after each insert.

This implementation uses two structures instead:

**Binary Search Tree**, keyed on the lowercase name. Contacts land in
alphabetical position as they are inserted, so displaying them in order is an
in-order traversal — no sorting algorithm is ever run. Insert and delete cost
O(log N) on average.

**Hash map** (`std::unordered_map<std::string, std::string>`) from lowercase
name to phone number. Search and edit go through this, giving O(1) average
lookup instead of the tree's O(log N).

They are kept in sync on every mutation. The cost is roughly double the
memory; the benefit is that both the common operations are fast rather than
one of them being a compromise.

Names are lowercased into a separate comparison key, so `ahmad` and `Ahmad`
resolve to the same entry rather than creating a duplicate.

## Complexity

N = number of contacts, L = length of a name.

| Operation | Average | Worst |
|---|---|---|
| `toLower` | O(L) | O(L) |
| `addContact` | O(L log N) | O(N·L) |
| `removeContact` | O(L log N) | O(N·L) |
| `searchContact` | O(L) | O(N·L) |
| `searchByPrefix` | O(N·L) | O(N·L) |
| `displayContacts` | O(N·L) | O(N·L) |
| `saveToFile` | O(N·L) | O(N·L) |
| `loadFromFile` | O(N·L log N) | O(N²·L) |

The worst cases assume a degenerate tree. Inserting contacts in alphabetical
order produces exactly that — the BST collapses into a linked list and every
operation becomes linear. A self-balancing tree (AVL or red-black) would cap
this at O(log N), and is the first thing to change if this were more than a
coursework exercise.

## Features

- Add, search, edit, remove, and display contacts
- Partial name search when the exact name is not found
- Phone number validation (7–15 digits; spaces, dashes and a leading `+` allowed)
- `cancel` at any prompt returns to the menu
- File persistence to `data/contacts.txt`, loaded at startup and saved on exit
- Malformed lines in the data file are skipped and reported, not crashed on

## Build and run

```bash
make
./phonebook
```

Requires a C++17 compiler. Tested with g++ 13.

```bash
make clean    # remove build artefacts
make run      # build and run in one step
```

## Structure

```
├── src/
│   ├── Contact.h       Contact record and BST node
│   ├── PhoneBook.h     class interface with complexity notes
│   ├── PhoneBook.cpp   BST operations, hash map sync, file I/O
│   └── main.cpp        menu loop and input handling
├── data/
│   └── contacts.txt    sample data (name,phone per line)
└── Makefile
```

## Possible improvements

- Self-balancing tree so the worst case cannot occur
- Support multiple numbers per contact
- A trie for prefix search, replacing the current O(N) tree walk
- Smart pointers instead of raw `new`/`delete`

## License

MIT
