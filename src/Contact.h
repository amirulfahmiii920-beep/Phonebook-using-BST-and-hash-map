#ifndef CONTACT_H
#define CONTACT_H

#include <string>

// A single phonebook entry.
struct Contact {
    std::string name;
    std::string phoneNumber;

    Contact() = default;
    Contact(std::string n, std::string p)
        : name(std::move(n)), phoneNumber(std::move(p)) {}
};

// Node of the Binary Search Tree.
//
// The BST is ordered by the LOWERCASE form of the name. Storing the
// comparison key separately means "ahmad" and "Ahmad" land in the same
// position, so the tree never holds two nodes for the same person just
// because of capitalisation.
struct TreeNode {
    Contact data;
    std::string key;   // lowercase name, used for every comparison
    TreeNode* left;
    TreeNode* right;

    TreeNode(const Contact& c, std::string k)
        : data(c), key(std::move(k)), left(nullptr), right(nullptr) {}
};

#endif // CONTACT_H
