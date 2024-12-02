cc = cc
flags = -Wall -Wextra -Wpedantic -Wshadow -Wconversion

target = flua

sources = $(wildcard *.c)
objects = $(sources:.c=.o)

all:
	@echo "available build options for flua"
	@echo "make clean      clean compiled assets"
	@echo "make develop    address sanitized"
	@echo "make release    performance optimized"

develop: $(objects)
	@echo "linking $(target)..."
	@$(cc) $(flags) -o $(target) $(objects) -lsqlite3 -O0 -fsanitize=address

%.o: %.c
	@echo "compiling $<..."
	@$(cc) $(flags) -c $< -o $@

release: $(objects)
	@echo "linking $(target)..."
	@$(cc) $(flags) -o $(target) $(objects) -lsqlite3 -O3

%.o: %.c
	@echo "compiling $<..."
	@$(cc) $(flags) -c $< -o $@

clean:
	@echo "cleaning up..."
	@rm -f $(objects) $(target)
