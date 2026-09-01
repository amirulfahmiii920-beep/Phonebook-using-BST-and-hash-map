#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Contact.h"

// Converts a string to lowercase. Used to build the BST comparison key
// and the hash map key, so lookups are case-insensitive.
// Time complexity: O(L), where L is the length of the string.
std::string toLower(const std::string& str);

// Returns true if the phone number is plausibly valid.
// Accepts digits, spaces and dashes; requires 7 to 15 digits overall.
bool isValidPhoneNumber(const std::string& phoneNumber);

// PhoneBook stores contacts in two structures at once:
//
//   1. A Binary Search Tree, keyed on the lowercase name. This keeps the
//      contacts permanently ordered, so displaying them alphabetically is
//      just an in-order traversal. No separate sort is ever run.
//
//   2. A hash map (std::unordered_map) from lowercase name to phone number.
//      This gives O(1) average lookup, which is what search and edit use.
//
// The two structures are kept in sync. Every mutation touches both.
class PhoneBook {
public:
    PhoneBook();
    ~PhoneBook();

    // Non-copyable: the class owns raw tree nodes.
    PhoneBook(const PhoneBook&) = delete;
    PhoneBook& operator=(const PhoneBook&) = delete;

    // Inserts a new contact, or updates the number if the name already
    // exists. updateHash is false only during a bulk file load, where the
    // map is rebuilt in one pass afterwards.
    // Average: O(L log N)   Worst: O(N * L) if the tree is skewed.
    void addContact(const std::string& name,
                    const std::string& phoneNumber,
                    bool updateHash = true);

    // Deletes a contact from both the tree and the map.
    // Average: O(L log N)   Worst: O(N * L)
    bool removeContact(const std::string& name);

    // Looks a contact up through the hash map.
    // Average: O(L)   Worst: O(N * L) if every key collides.
    bool searchContact(const std::string& name, std::string& phoneOut) const;

    // Partial, case-insensitive name search. Walks the whole tree, so this
    // is the slow path — offered because users rarely recall exact spelling.
    // O(N * L)
    std::vector<Contact> searchByPrefix(const std::string& prefix) const;

    // Changes the number stored against an existing name.
    // Average: O(L log N)
    bool editContact(const std::string& name, const std::string& newPhoneNumber);

    // Prints every contact in alphabetical order via in-order traversal.
    // O(N * L)
    void displayContacts() const;

    // Persistence. File format is one "name,phone" pair per line.
    // saveToFile:   O(N * L)
    // loadFromFile: O(N * L log N) average
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);

    std::size_t size() const { return contactMap.size(); }
    bool empty()       const { return contactMap.empty(); }

private:
    TreeNode* root;
    std::unordered_map<std::string, std::string> contactMap;

    // BST internals.
    TreeNode* insertNode(TreeNode* node, const Contact& contact, const std::string& key);
    TreeNode* deleteNode(TreeNode* node, const std::string& key, bool& deleted);
    TreeNode* findMinNode(TreeNode* node) const;
    void inorderTraversal(TreeNode* node) const;
    void collectPrefix(TreeNode* node,
                       const std::string& prefix,
                       std::vector<Contact>& out) const;
    void saveToFileHelper(TreeNode* node, std::ostream& out) const;
    void freeTree(TreeNode* node);
    TreeNode* findNode(TreeNode* node, const std::string& key) const;
};

#endif // PHONEBOOK_H
