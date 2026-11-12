# Builds and runs every program in the book.
#
#   make test     compile each chNN/*.cpp with warnings-as-errors and the
#                 address + undefined-behaviour sanitizers, then run it
#   make bench    compile chNN/bench/*.cpp with -O2 (no sanitizers)
#   make bugs     compile the deliberately broken programs in chNN/bugs/
#   make clean
#
# A program that reads standard input is fed chNN/<name>.in if it exists.

CXX      ?= g++
STD      := -std=c++20
WARN     := -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror
SAN      := -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer
BIN      := ../build/bin

SRCS     := $(wildcard ch*/*.cpp) $(wildcard app*/*.cpp)
PROGS    := $(patsubst %.cpp,$(BIN)/%,$(SRCS))
BENCH    := $(patsubst %.cpp,$(BIN)/%,$(wildcard ch*/bench/*.cpp))
BUGS     := $(patsubst %.cpp,$(BIN)/%,$(wildcard ch*/bugs/*.cpp))

.PHONY: test bench bugs clean
test: $(PROGS)
	@fail=0; for p in $(PROGS); do \
	  src=$${p#$(BIN)/}; in=/dev/null; [ -f $$src.in ] && in=$$src.in; \
	  if $$p < $$in > $$p.out 2>&1; then echo "ok    $$src"; \
	  else echo "FAIL  $$src"; tail -5 $$p.out; fail=1; fi; \
	done; [ $$fail -eq 0 ] && echo "all programs passed"

$(BIN)/%: %.cpp $(wildcard ch*/*.hpp)
	@mkdir -p $(dir $@)
	$(CXX) $(STD) $(WARN) $(SAN) -I. $< -o $@

bench: $(BENCH)
$(BIN)/ch%/bench/%: ch%/bench/%.cpp
	@true
$(BENCH): $(BIN)/%: %.cpp $(wildcard ch*/*.hpp)
	@mkdir -p $(dir $@)
	$(CXX) $(STD) -Wall -Wextra -O2 -I. $< -o $@

bugs: $(BUGS)
$(BUGS): $(BIN)/%: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(STD) -Wall -Wextra $(SAN) $< -o $@ || true

clean:
	rm -rf $(BIN)
