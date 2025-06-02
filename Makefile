CXX = g++
CXXFLAGS = -std=c++20 -Wall -Werror -Wpedantic -O2 -Isrc
LDFLAGS = -lgtest -lpthread

TARGET = tests
TEST_BIN = test

TEST_SRCS = tests/test-skip-list.cpp

all: $(TARGET)

$(TARGET): SkipList.h $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_SRCS) $(LDFLAGS)

run_tests: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -f $(TARGET) $(TEST_BIN) result.csv

.PHONY: all tests clean