// Unit tests for PhoneBook. No framework: each check prints PASS or FAIL
// and the program exits with 1 if anything failed.
//
// Build and run:  make test

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "PhoneBook.h"

namespace {

int failures = 0;
int checks = 0;

void check(bool ok, const std::string& what) {
    ++checks;
    if (!ok) ++failures;
    std::cout << (ok ? "PASS  " : "FAIL  ") << what << "\n";
}

// Captures what displayContacts() prints, so the order can be checked.
std::string captureDisplay(const PhoneBook& book) {
    std::ostringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
    book.displayContacts();
    std::cout.rdbuf(old);
    return buffer.str();
}

std::string readFile(const std::string& path) {
    std::ifstream in(path);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

const std::string TMP_FILE = "tests/tmp_contacts.txt";

void testPhoneValidation() {
    check(isValidPhoneNumber("012-000 0001"), "accepts a Malaysian mobile with dash and space");
    check(isValidPhoneNumber("+60 12-000 0010"), "accepts a leading +");
    check(isValidPhoneNumber("1234567"), "accepts exactly 7 digits");
    check(isValidPhoneNumber("123456789012345"), "accepts exactly 15 digits");
    check(!isValidPhoneNumber("123456"), "rejects 6 digits");
    check(!isValidPhoneNumber("1234567890123456"), "rejects 16 digits");
    check(!isValidPhoneNumber("012-ABC 4567"), "rejects letters");
    check(!isValidPhoneNumber("12+34-5678"), "rejects a + that is not first");
    check(!isValidPhoneNumber("++60123456789"), "rejects a second +");
}

void testAddSearchCaseInsensitive() {
    PhoneBook book;
    book.addContact("Ahmad Faizal", "012-000 0001");
    std::string phone;
    check(book.searchContact("ahmad faizal", phone) && phone == "012-000 0001",
          "search ignores case");
    check(book.searchContact("  AHMAD FAIZAL  ", phone), "search ignores surrounding spaces");

    book.addContact("ahmad faizal", "012-000 0006");
    check(book.size() == 1, "adding the same name in another case updates, not duplicates");
    check(book.searchContact("Ahmad Faizal", phone) && phone == "012-000 0006",
          "the update keeps the new number");
}

void testOrderAndRemove() {
    PhoneBook book;
    // Inserted out of order; display must still be alphabetical.
    book.addContact("Nurul Aina", "012-000 0002");
    book.addContact("Zamil Hakim", "012-000 0004");
    book.addContact("Ahmad Faizal", "012-000 0001");
    book.addContact("Siti Zubaidah", "012-000 0003");
    book.addContact("Mei Ling", "012-000 0005");

    const std::string out = captureDisplay(book);
    const auto a = out.find("Ahmad"), m = out.find("Mei"), n = out.find("Nurul"),
               s = out.find("Siti"), z = out.find("Zamil");
    check(a < m && m < n && n < s && s < z, "display is alphabetical (in-order traversal)");

    // Nurul is the root with two children: exercises the successor case.
    check(book.removeContact("nurul aina"), "removes a node with two children");
    std::string phone;
    check(!book.searchContact("Nurul Aina", phone), "removed contact is gone from the map");
    check(book.size() == 4, "size drops to 4");
    const std::string after = captureDisplay(book);
    check(after.find("Nurul") == std::string::npos, "removed contact is gone from the tree");
    check(after.find("Ahmad") < after.find("Mei") && after.find("Mei") < after.find("Siti"),
          "tree stays in order after removing the root");

    check(book.removeContact("Zamil Hakim"), "removes a leaf");
    check(book.removeContact("Ahmad Faizal"), "removes a node with one or no child");
    check(!book.removeContact("Nobody Here"), "removing an unknown name returns false");
    check(book.size() == 2, "size is 2 after three removals");
}

void testEditAndPrefix() {
    PhoneBook book;
    book.addContact("Siti Zubaidah", "012-000 0003");
    book.addContact("Siti Aminah", "012-000 0007");
    book.addContact("Ahmad Faizal", "012-000 0001");

    check(book.editContact("siti aminah", "012-000 0008"), "edits an existing contact");
    std::string phone;
    check(book.searchContact("Siti Aminah", phone) && phone == "012-000 0008",
          "edit updates the map");
    check(captureDisplay(book).find("012-000 0008") != std::string::npos,
          "edit updates the tree");
    check(!book.editContact("Nobody", "012-000 0011"), "editing an unknown name returns false");

    const auto matches = book.searchByPrefix("sITi");
    check(matches.size() == 2 && matches[0].name == "Siti Aminah" &&
              matches[1].name == "Siti Zubaidah",
          "prefix search is case-insensitive and alphabetical");
    check(book.searchByPrefix("Zubaidah").empty(),
          "prefix search does not match the middle of a name");
}

void testSaveLoadRoundTrip() {
    {
        PhoneBook book;
        book.addContact("Tan, Ali", "012-000 0009");
        book.addContact("Ahmad Faizal", "012-000 0001");
        check(book.saveToFile(TMP_FILE), "saves to file");
    }
    check(readFile(TMP_FILE) == "Ahmad Faizal,012-000 0001\nTan, Ali,012-000 0009\n",
          "file is written alphabetically, one name,phone per line");

    PhoneBook loaded;
    std::ostringstream sink;
    std::streambuf* old = std::cout.rdbuf(sink.rdbuf());
    loaded.loadFromFile(TMP_FILE);
    std::cout.rdbuf(old);

    std::string phone;
    check(loaded.size() == 2, "loads both contacts back");
    check(loaded.searchContact("Tan, Ali", phone) && phone == "012-000 0009",
          "a name with a comma survives save and load");
    std::remove(TMP_FILE.c_str());
}

void testLoadSkipsMalformedLines() {
    {
        std::ofstream f(TMP_FILE);
        f << "Ahmad Faizal,012-000 0001\n"
          << "no comma on this line\n"
          << ",012-000 0009\n"
          << "Bad Number,not a phone\n"
          << "\n"
          << "Nurul Aina,012-000 0002\n";
    }
    PhoneBook book;
    book.addContact("Old Contact", "012-000 0000");

    std::ostringstream sink;
    std::streambuf* old = std::cout.rdbuf(sink.rdbuf());
    const bool ok = book.loadFromFile(TMP_FILE);
    std::cout.rdbuf(old);

    std::string phone;
    check(ok && book.size() == 2, "keeps the 2 good lines and replaces what was in memory");
    check(!book.searchContact("Old Contact", phone), "load clears the old contacts first");
    check(sink.str().find("3 malformed line(s) skipped") != std::string::npos,
          "reports 3 malformed lines (blank lines are not counted)");
    std::remove(TMP_FILE.c_str());
}

void testManyContactsInOrder() {
    // Alphabetical inserts make the tree a linked list (the worst case).
    // It must still work, just slowly.
    PhoneBook book;
    std::vector<std::string> names;
    for (int i = 0; i < 2000; ++i) {
        char buf[16];
        std::snprintf(buf, sizeof buf, "Contact %04d", i);
        names.push_back(buf);
        book.addContact(buf, "012-000 0000");
    }
    check(book.size() == 2000, "2000 contacts added in alphabetical order");
    for (int i = 0; i < 2000; i += 2) book.removeContact(names[i]);
    std::string phone;
    check(book.size() == 1000 && book.searchContact("Contact 1999", phone) &&
              !book.searchContact("Contact 0000", phone),
          "every other one removed, the rest still found");
}

}  // namespace

int main() {
    testPhoneValidation();
    testAddSearchCaseInsensitive();
    testOrderAndRemove();
    testEditAndPrefix();
    testSaveLoadRoundTrip();
    testLoadSkipsMalformedLines();
    testManyContactsInOrder();

    std::cout << "\n" << (checks - failures) << " / " << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}
