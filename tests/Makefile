# Test build for Lab 4. Run from the repo root: make -C tests
CXX := g++
CXXFLAGS := -std=c++17 -O0 -g --coverage -Wall -Wextra
INCLUDES := -I ../includes
LIBS := -lgtest -lgtest_main -pthread

# Part B: only the sources that link without notcurses.
PARTB_SRC := ../source/snake.cpp ../source/fruit.cpp partb_test.cpp
# Part D: the rules seam, still without notcurses.
ADVANCE_SRC := ../source/snake.cpp ../source/rules.cpp advance_test.cpp

all: partb advance

partb: $(PARTB_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(PARTB_SRC) -o $@ $(LIBS)

advance: $(ADVANCE_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ADVANCE_SRC) -o $@ $(LIBS)

clean:
	rm -f partb partb.exe advance advance.exe *.gcda *.gcno *.gcov
