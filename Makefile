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

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean
