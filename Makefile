CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
INCLUDES := -I includes

NC_CFLAGS := $(shell pkg-config --cflags notcurses 2>/dev/null)
NC_LIBS := $(shell pkg-config --libs notcurses 2>/dev/null)
ifeq ($(strip $(NC_LIBS)),)
# Fallback if pkg-config not available
NC_LIBS := -lnotcurses -lnotcurses-core
endif

SRC := source/main.cpp source/game.cpp source/snake.cpp source/fruit.cpp
BIN := snake

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(NC_CFLAGS) $(SRC) -o $(BIN) $(NC_LIBS)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
