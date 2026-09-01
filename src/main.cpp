// Phonebook Application
// BEJ32103 Data Structures and Algorithms - Group Project
//
// Data structures: Binary Search Tree (ordering) + hash map (fast lookup)
// Language: C++17
//
// Build:  make
// Run:    ./phonebook

#include <fstream>
#include <iostream>
#include <string>

#include "PhoneBook.h"

namespace {

const std::string DATA_FILE = "data/contacts.txt";

void printMenu() {
    std::cout << "\n===============================\n"
              << "        PHONEBOOK MENU\n"
              << "===============================\n"
              << " 1. Add Contact\n"
              << " 2. Search Contact\n"
              << " 3. Edit Contact\n"
              << " 4. Remove Contact\n"
              << " 5. Display All Contacts\n"
              << " 6. Save Contacts to File\n"
              << " 7. Load Contacts from File\n"
              << " 8. Exit\n"
              << "-------------------------------\n"
              << "Enter your choice: ";
}

// Reads a whole line. Returns false when the user types "cancel",
// which every prompt accepts as a way back to the menu.
bool prompt(const std::string& label, std::string& out) {
    std::cout << label;
    if (!std::getline(std::cin, out)) return false;
    return toLower(out) != "cancel";
}

int readChoice() {
    std::string line;
    if (!std::getline(std::cin, line)) return 8;   // EOF behaves as Exit
    try {
        return std::stoi(line);
    } catch (...) {
        return -1;
    }
}

void handleAdd(PhoneBook& book) {
    std::string name, phone;
    if (!prompt("Enter name (type 'cancel' to return to menu): ", name)) return;
    if (name.empty()) {
        std::cout << "Name cannot be empty.\n";
        return;
    }

    // Keep asking until the number is valid or the user gives up.
    while (true) {
        if (!prompt("Enter phone number: ", phone)) return;
        if (isValidPhoneNumber(phone)) break;
        std::cout << "Invalid phone number. Use 7 to 15 digits "
                     "(spaces, dashes and a leading + are allowed).\n";
    }

    std::string existing;
    const bool isUpdate = book.searchContact(name, existing);

    book.addContact(name, phone);
    std::cout << (isUpdate ? "Contact updated: " : "Contact added: ")
              << name << " - " << phone << "\n";
}

void handleSearch(const PhoneBook& book) {
    std::string name;
    if (!prompt("Enter name to search (type 'cancel' to return to menu): ", name)) return;

    std::string phone;
    if (book.searchContact(name, phone)) {
        std::cout << "Found contact: " << name << " - " << phone << "\n";
        return;
    }

    // Exact hash lookup missed. Fall back to a partial name scan so a
    // typo or a half-remembered name still returns something useful.
    const auto matches = book.searchByPrefix(name);
    if (matches.empty()) {
        std::cout << "Contact not found.\n";
        return;
    }
    std::cout << "No exact match. " << matches.size() << " similar contact(s):\n";
    for (const auto& c : matches) {
        std::cout << "  " << c.name << " - " << c.phoneNumber << "\n";
    }
}

void handleEdit(PhoneBook& book) {
    std::string name, phone;
    if (!prompt("Enter name to edit (type 'cancel' to return to menu): ", name)) return;

    std::string current;
    if (!book.searchContact(name, current)) {
        std::cout << "Contact not found: " << name << "\n";
        return;
    }
    std::cout << "Current number: " << current << "\n";

    while (true) {
        if (!prompt("Enter new phone number: ", phone)) return;
        if (isValidPhoneNumber(phone)) break;
        std::cout << "Invalid phone number. Use 7 to 15 digits.\n";
    }

    book.editContact(name, phone);
    std::cout << "Contact updated: " << name << " - " << phone << "\n";
}

void handleRemove(PhoneBook& book) {
    std::string name;
    if (!prompt("Enter name to remove (type 'cancel' to return to menu): ", name)) return;

    if (book.removeContact(name)) {
        std::cout << "Contact removed: " << name << "\n";
    } else {
        std::cout << "Contact not found: " << name << "\n";
    }
}

}  // namespace

int main() {
    PhoneBook book;

    // Load any saved contacts at startup so data persists between runs.
    std::ifstream probe(DATA_FILE);
    if (probe.good()) {
        probe.close();
        book.loadFromFile(DATA_FILE);
    }

    while (true) {
        printMenu();
        const int choice = readChoice();

        switch (choice) {
            case 1: handleAdd(book);            break;
            case 2: handleSearch(book);         break;
            case 3: handleEdit(book);           break;
            case 4: handleRemove(book);         break;
            case 5: book.displayContacts();     break;

            case 6: {
                if (book.saveToFile(DATA_FILE)) {
                    std::cout << "Contacts saved to " << DATA_FILE << "\n";
                }
                break;
            }
            case 7:
                book.loadFromFile(DATA_FILE);
                break;

            case 8:
                // Autosave on exit so work is never silently lost.
                book.saveToFile(DATA_FILE);
                std::cout << "Contacts saved. Goodbye.\n";
                return 0;

            default:
                std::cout << "Invalid choice. Please enter a number from 1 to 8.\n";
        }
    }
}
