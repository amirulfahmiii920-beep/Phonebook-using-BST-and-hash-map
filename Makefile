CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Isrc
TARGET   := phonebook
SRCS     := src/main.cpp src/PhoneBook.cpp
OBJS     := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

TEST_TARGET := tests/run_tests

$(TEST_TARGET): tests/test_phonebook.cpp src/PhoneBook.cpp src/PhoneBook.h src/Contact.h
	$(CXX) $(CXXFLAGS) -o $@ tests/test_phonebook.cpp src/PhoneBook.cpp

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_TARGET)

.PHONY: all run test clean
