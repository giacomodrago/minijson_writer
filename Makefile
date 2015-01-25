# You can use this (non-portable) Makefile to build and run the unit tests on a typical Linux machine.

# You don't need to compile any library to use minijson_writer in your project:
# just include the minijson_writer.hpp header anywhere you need, and you're ready to go.

CXX=g++
CPPSTD=c++11
CXXFLAGS=-Wall -Wextra -std=$(CPPSTD) -O0 -g --coverage
LDFLAGS=-lgtest -pthread
TARGET=minijson_writer_tests
HEADERS=minijson_writer.hpp
MEMDEBUG=valgrind --leak-check=full
LCOV_COMMAND=lcov --directory . --capture --output-file coverage.info
GENHTML_COMMAND=genhtml --output-directory genhtml coverage.info

$(TARGET): $(TARGET).cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)
	$(LCOV_COMMAND)
	$(GENHTML_COMMAND)

memdebug: $(TARGET)
	$(MEMDEBUG) ./$(TARGET)

clean:
	@rm -rf $(TARGET)
	@rm -rf *.gcno
	@rm -rf *.gcda
	@rm -rf coverage.info
	@rm -rf genhtml
