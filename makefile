cc = cc
flags = -Wall -Wextra -Wpedantic -Wshadow -Wconversion

target = flua

sources = $(wildcard *.c)
objects = $(sources:.c=.o)

all: $(target)

$(target): $(objects)
	@echo "linking $(target)..."
	@$(cc) $(flags) -o $(target) $(objects)

%.o: %.c
	@echo "compiling $<..."
	@$(cc) $(flags) -c $< -o $@

clean:
	@echo "cleaning up..."
	@rm -f $(objects) $(target)
