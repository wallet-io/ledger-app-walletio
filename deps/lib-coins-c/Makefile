# originating https://github.com/TheNetAdmin/Makefile-Templates

# tool macros
CC := gcc
CCFLAG := -g
DBGFLAG := -g
CCOBJFLAG := $(CCFLAG) -c

# path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug
TEST_PATH := test

# compile macros
TARGET_NAME := main
ifeq ($(OS),Windows_NT)
    TARGET_NAME := $(addsuffix .exe,$(TARGET_NAME))
endif
TARGET := $(BIN_PATH)/$(TARGET_NAME)
TARGET_DEBUG := $(DBG_PATH)/$(TARGET_NAME)

# src files & obj files
DIRS := $(shell find $(SRC_PATH) -type d)
SRC := $(foreach dir,$(DIRS),$(wildcard $(dir)/*.c*))
OBJ := $(patsubst $(SRC_PATH)/%.o, $(OBJ_PATH)/%.o, $(patsubst %.c,%.o,$(SRC)))
OBJ_DEBUG := $(patsubst $(SRC_PATH)/, $(DBG_PATH)/, $(patsubst %.c,%.o,$(SRC)))

# clean files list
DISTCLEAN_LIST := $(OBJ) \
                  $(OBJ_DEBUG)
CLEAN_LIST := $(TARGET) \
              $(TARGET_DEBUG) \
              $(DISTCLEAN_LIST)

# default rule
default: all

# test
TEST_SRC := $(foreach dir,$(TEST_PATH),$(wildcard $(dir)/*.c*))
TEST_OBJ := $(patsubst $(TEST_PATH)/%.o, $(OBJ_PATH)/test/%.o, $(patsubst %.c,%.o,$(TEST_SRC)))
TEST_TARGET := $(BIN_PATH)/test

$(OBJ_PATH)/test/%.o: $(TEST_PATH)/%.c*
	@mkdir -p $(dir $@)
	$(CC) $(CCOBJFLAG) -o $@ $< -I$(SRC_PATH)

.PHONY: test
test: $(TEST_TARGET)

$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
	$(CC) $(CCFLAG) -o $@ $?
	
# non-phony targets
$(TARGET): $(OBJ)
	$(CC) $(CCFLAG) -g -o $@ $?

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	@mkdir -p $(dir $@)
	$(CC) $(CCOBJFLAG) -o $@ $<

$(DBG_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(CCOBJFLAG) $(DBGFLAG) -o $@ $<

$(TARGET_DEBUG): $(OBJ_DEBUG)
	$(CC) $(CCFLAG) $(DBGFLAG) $? -o $@

# phony rules
.PHONY: all
all: $(TARGET)

.PHONY: debug
debug: $(TARGET_DEBUG)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)

.PHONY: distclean
distclean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(DISTCLEAN_LIST)
	