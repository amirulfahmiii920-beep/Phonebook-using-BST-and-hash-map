#include "PhoneBook.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

// ---------------------------------------------------------------------------
// Free helpers
// ---------------------------------------------------------------------------

std::string toLower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lowerStr;
}

// Trims leading and trailing whitespace.
static std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

bool isValidPhoneNumber(const std::string& phoneNumber) {
    int digits = 0;
    for (unsigned char c : phoneNumber) {
        if (std::isdigit(c)) {
            ++digits;
        } else if (c != ' ' && c != '-' && c != '+') {
            return false;   // reject letters and other symbols
        }
    }
    return digits >= 7 && digits <= 15;
}

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

PhoneBook::PhoneBook() : root(nullptr) {}

PhoneBook::~PhoneBook() {
    freeTree(root);
    root = nullptr;
}

void PhoneBook::freeTree(TreeNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

// ---------------------------------------------------------------------------
// Insertion
// ---------------------------------------------------------------------------

TreeNode* PhoneBook::insertNode(TreeNode* node,
                                const Contact& contact,
                                const std::string& key) {
    // Empty slot reached: this is where the contact belongs.
    if (!node) {
        return new TreeNode(contact, key);
    }

    if (key < node->key) {
        node->left = insertNode(node->left, contact, key);
    } else if (key > node->key) {
        node->right = insertNode(node->right, contact, key);
    } else {
        // Key already present. Update in place rather than duplicating,
        // and keep the newly supplied spelling of the name.
        node->data = contact;
    }
    return node;
}

void PhoneBook::addContact(const std::string& name,
                           const std::string& phoneNumber,
                           bool updateHash) {
    const std::string cleanName  = trim(name);
    const std::string cleanPhone = trim(phoneNumber);
    if (cleanName.empty()) return;

    const std::string key = toLower(cleanName);

    root = insertNode(root, Contact(cleanName, cleanPhone), key);

    if (updateHash) {
        contactMap[key] = cleanPhone;
    }
}

// ---------------------------------------------------------------------------
// Deletion
// ---------------------------------------------------------------------------

TreeNode* PhoneBook::findMinNode(TreeNode* node) const {
    while (node && node->left) node = node->left;
    return node;
}

TreeNode* PhoneBook::deleteNode(TreeNode* node,
                                const std::string& key,
                                bool& deleted) {
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = deleteNode(node->left, key, deleted);
    } else if (key > node->key) {
        node->right = deleteNode(node->right, key, deleted);
    } else {
        deleted = true;

        // Case 1 and 2: zero or one child. Splice the node out.
        if (!node->left) {
            TreeNode* rightChild = node->right;
            delete node;
            return rightChild;
        }
        if (!node->right) {
            TreeNode* leftChild = node->left;
            delete node;
            return leftChild;
        }

        // Case 3: two children. Replace this node's payload with its
        // in-order successor (smallest key in the right subtree), then
        // delete that successor from the right subtree. This preserves
        // the BST ordering property.
        TreeNode* successor = findMinNode(node->right);
        node->data = successor->data;
        node->key  = successor->key;

        bool dummy = false;
        node->right = deleteNode(node->right, successor->key, dummy);
    }
    return node;
}

bool PhoneBook::removeContact(const std::string& name) {
    const std::string key = toLower(trim(name));

    // Hash map first: O(1) rejection when the contact does not exist,
    // so we avoid walking the tree for nothing.
    if (contactMap.find(key) == contactMap.end()) {
        return false;
    }

    bool deleted = false;
    root = deleteNode(root, key, deleted);
    if (deleted) {
        contactMap.erase(key);
    }
    return deleted;
}

// ---------------------------------------------------------------------------
// Search
// ---------------------------------------------------------------------------

bool PhoneBook::searchContact(const std::string& name, std::string& phoneOut) const {
    const std::string key = toLower(trim(name));
    auto it = contactMap.find(key);
    if (it == contactMap.end()) {
        return false;
    }
    phoneOut = it->second;
    return true;
}

TreeNode* PhoneBook::findNode(TreeNode* node, const std::string& key) const {
    while (node) {
        if (key < node->key)      node = node->left;
        else if (key > node->key) node = node->right;
        else                      return node;
    }
    return nullptr;
}

void PhoneBook::collectPrefix(TreeNode* node,
                              const std::string& prefix,
                              std::vector<Contact>& out) const {
    if (!node) return;
    collectPrefix(node->left, prefix, out);
    if (node->key.rfind(prefix, 0) == 0) {   // key starts with prefix
        out.push_back(node->data);
    }
    collectPrefix(node->right, prefix, out);
}

std::vector<Contact> PhoneBook::searchByPrefix(const std::string& prefix) const {
    std::vector<Contact> matches;
    collectPrefix(root, toLower(trim(prefix)), matches);
    return matches;   // already alphabetical, courtesy of the in-order walk
}

bool PhoneBook::editContact(const std::string& name, const std::string& newPhoneNumber) {
    const std::string key = toLower(trim(name));
    if (contactMap.find(key) == contactMap.end()) {
        return false;
    }

    const std::string cleanPhone = trim(newPhoneNumber);

    TreeNode* node = findNode(root, key);
    if (node) node->data.phoneNumber = cleanPhone;
    contactMap[key] = cleanPhone;
    return true;
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

void PhoneBook::inorderTraversal(TreeNode* node) const {
    if (!node) return;
    inorderTraversal(node->left);
    std::cout << "  " << std::left << std::setw(28) << node->data.name
              << node->data.phoneNumber << "\n";
    inorderTraversal(node->right);
}

void PhoneBook::displayContacts() const {
    if (!root) {
        std::cout << "No contacts available.\n";
        return;
    }
    std::cout << "\nContacts (" << contactMap.size() << "):\n";
    std::cout << "  " << std::left << std::setw(28) << "NAME" << "PHONE\n";
    std::cout << "  " << std::string(44, '-') << "\n";
    inorderTraversal(root);
}

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------

void PhoneBook::saveToFileHelper(TreeNode* node, std::ostream& out) const {
    if (!node) return;
    saveToFileHelper(node->left, out);
    out << node->data.name << ',' << node->data.phoneNumber << '\n';
    saveToFileHelper(node->right, out);
}

bool PhoneBook::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) {
        std::cout << "Error opening file for writing.\n";
        return false;
    }
    // In-order walk means the file is written alphabetically sorted.
    saveToFileHelper(root, file);
    file.close();
    return true;
}

bool PhoneBook::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cout << "Error opening file for reading.\n";
        return false;
    }

    // Discard whatever is currently in memory before loading.
    freeTree(root);
    root = nullptr;
    contactMap.clear();

    std::string line;
    std::size_t loaded = 0, skipped = 0;

    while (std::getline(file, line)) {
        if (trim(line).empty()) continue;

        const auto comma = line.find(',');
        if (comma == std::string::npos) { ++skipped; continue; }

        const std::string name  = trim(line.substr(0, comma));
        const std::string phone = trim(line.substr(comma + 1));
        if (name.empty()) { ++skipped; continue; }

        // updateHash = false: the tree is built first, then the map is
        // rebuilt in one pass below. Same result, one less map write per row.
        addContact(name, phone, false);
        contactMap[toLower(name)] = phone;
        ++loaded;
    }
    file.close();

    std::cout << "Loaded " << loaded << " contact(s) from " << filename;
    if (skipped) std::cout << " (" << skipped << " malformed line(s) skipped)";
    std::cout << ".\n";
    return true;
}
