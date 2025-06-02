CXX = g++
CXXFLAGS = -std=c++20 -Wall -Werror -Wpedantic -O2 -Isrc
LDFLAGS = -lgtest -lpthread

TARGET = test
TEST_SRCS = tests/test-skip-list.cpp

all: $(TARGET)

$(TARGET): SkipList.h $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_SRCS) $(LDFLAGS)

run_tests: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all tests clean