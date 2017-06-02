# Colors
green            ='\033[0;32m'
red              ='\033[0;31m'
nocolor          ='\033[0m'

CFLAGS           = -std=c11 -Wall -Wextra -Wpedantic -Wshadow -I./src -I/usr/include
PP_FLAGS         = -D LT_DEBUG -D DEV_ENV
LDLIBS           = -L/usr/lib -lm -lglfw -lGL -lpthread -lX11 -lXi -lXrandr -ldl
LDFLAGS_CMOCKA   = -Wl,--wrap=write -Wl,--wrap=read
LDLIBS_CMOCKA    = -L/usr/lib -lcmocka

CC               = clang
SRC              = $(shell find src -mindepth 1 -name "*.c")
HEADERS          = $(wildcard src/**/*.h)
OBJ              = ${SRC:src/%.c=build/objects/%.o}
DEP              = $(OBJ:%.o=%.d)

BIN              = shloader
BUILD_DIR        = ./build

libraries_mocka = $(libraries) -lcmocka

# Default target named after the binary.
$(BIN): $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(BIN): $(OBJ)
	mkdir -p $(@D)
	@echo CC $^ -o $@
	@$(CC) $(CFLAGS) $^ $(LDLIBS)  -o $@

# include all .d files.
-include $(DEP)

# Build target for every single object file.
# The potentially dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/objects/%.o: src/%.c
	mkdir -p $(@D)
	@echo CC -MMD -c $< -o $@
	@$(CC) $(PP_FLAGS) $(CFLAGS) -MMD -c $< -o $@

cc_args: CC="cc_args.py clang"
cc_args: $(BIN)

.PHONY: clean
clean:
	find build/ -not -path '*/\.*' -type f -delete

run: $(BIN)
	$(BUILD_DIR)/$(BIN)

# tests/run: test
# 	./run-tests.sh
