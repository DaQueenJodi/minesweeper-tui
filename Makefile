SRCS := $(shell find src/ -name "*.c" )
OBJS := $(SRCS:.c=.o)
BIN  := bin/main
LDFLAGS := -fsanitize=address -fsanitize=undefined -lncurses -ltinfo
FLAGS := -Wextra -Wall -Wpedantic -fsanitize=address -fsanitize=undefined -ggdb -pthread
INCLUDES := -Iinclude

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJS)
	clang $(OBJS) $(LDFLAGS) -o $(BIN)

%.o: %.c
	clang $(FLAGS) $(INCLUDES) -c $< -o $@


clean:
	rm $(OBJS)

run:
	$(BIN)
