CC = gcc
CFLAGS = -Wall -Wextra -I./include -fPIC
LDFLAGS = -shared
LDFLAGS_TEST =
DEFS =
MARCH = -march=native
MARCH_LD =

BIN = bin
SRC = src
INC = include
OBJ = $(BIN)/tmp

DESTDIR = /usr/
BINDIR = $(DESTDIR)/bin
LIBDIR = $(DESTDIR)/lib
INCLUDEDIR = $(DESTDIR)/include
TEST_SRC = tests
TEST_BIN = $(BIN)/tests

TARGET_LIB = $(BIN)/libbufrw.so
INSTALL_LIB = $(LIBDIR)/libbufrw.so
INSTALL_INCDIR = $(INCLUDEDIR)

SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))

TEST_SRCS = $(wildcard $(TEST_SRC)/*.c)
TEST_OBJS = $(patsubst $(TEST_SRC)/%.c,$(OBJ)/%.o,$(TEST_SRCS))
TEST_EXEC = $(TEST_BIN)/unit_tests

# EXAMPLES_SRC = examples
# EXAMPLES_BIN = $(BIN)/examples

# EXAMPLES_SRCS = $(wildcard $(EXAMPLES_SRC)/*.c)
# EXAMPLES_OBJS = $(patsubst $(EXAMPLES_SRC)/%.c,$(OBJ)/%.o,$(EXAMPLES_SRCS))
# EXAMPLES_EXEC = $(patsubst $(EXAMPLES_SRC)/%.c,$(EXAMPLES_BIN)/%,$(EXAMPLES_SRCS))

all: lib tests # examples

lib: $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	mkdir -p $(BIN)
	$(CC) $(LDFLAGS) $(MARCH) $(DEFS) -I$(INC) -o $@ $^

$(OBJ)/%.o: $(SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) $(CFLAGS) $(MARCH) $(DEFS) -I$(INC) -c $< -o $@

$(OBJ)/%.o: $(TEST_SRC)/%.c
	mkdir -p $(OBJ)
	$(CC) $(CFLAGS) $(MARCH) $(DEFS) -I$(INC) -c $< -o $@ -g

$(TEST_EXEC): $(TEST_OBJS) $(TARGET_LIB)
	mkdir -p $(TEST_BIN)
	$(CC) $(LDFLAGS_TEST) $(MARCH_LD) -I$(INC) -o $@ $^ -L$(BIN) -lbufrw -g

# examples: $(EXAMPLES_EXEC)

# $(EXAMPLES_BIN)/%: $(EXAMPLES_SRC)/%.c $(TARGET_LIB)
# 	mkdir -p $(EXAMPLES_BIN)
# 	$(CC) $(CFLAGS) $(MARCH) $(DEFS) -I$(INC) -o $@ $< -L$(BIN) -lbufrw $(EXAMPLES_l) -g

clean-garbage:
	rm -rf $(OBJ)

clean:
	rm -rf $(BIN)

clean-all: clean

install-lib: $(TARGET_LIB)
	# Install the shared library
	install -d $(LIBDIR)
	install -m 755 $(TARGET_LIB) $(INSTALL_LIB)

	# Install the header file
	# install -d $(INCLUDEDIR)

	mkdir -p $(INSTALL_INCDIR)
	install -m 644 $(wildcard $(INC)/*.h) $(INSTALL_INCDIR)

	@echo "libbufrw installed"

# Uninstall the library and header file
uninstall-lib:
	# Remove the shared library
	rm -f $(INSTALL_LIB)

	# Remove the header file
	rm -rf $(INSTALL_INCDIR)

	@echo "libbufrw uninstalled"

# install-execs: $(EXAMPLES_EXEC)
# 	# Install example executables to the binary directory
# 	install -d $(BINDIR)
# 	for exec in $(EXAMPLES_EXEC); do \
# 		install -m 755 $$exec $(BINDIR); \
# 	done

# 	@echo "bufrw utils installed"

# uninstall-execs:
# 	# Remove installed example executables
# 	for exec in $(notdir $(EXAMPLES_EXEC)); do \
# 		rm -f $(BINDIR)/$$exec; \
# 	done

# 	@echo "bufrw utils uninstalled"

install: install-lib # install-execs
uninstall: uninstall-lib # uninstall-execs

tests: $(TEST_EXEC)

check: $(TEST_EXEC)
	$(TEST_EXEC)

.PHONY: all clean install uninstall check