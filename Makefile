.DELETE_ON_ERROR:

TARGET		:= chessEngine

COMPILER	:= g++
FLAGS		:= -Wall -std=c++17 -MMD
LINKS		:=


# Directories, Objects, and Binary 
SRC_DIR     := src
BUILD_DIR   := obj
TARGET_DIR	:= bin

SRC_EXT     := cpp
OBJ_EXT     := o
DEP_EXT     := d

#-----------------------------------
#	 DO NOT EDIT BELOW THIS LINE
#-----------------------------------

BIN  := $(TARGET_DIR)/$(TARGET)
SRCS := $(wildcard $(SRC_DIR)/*.$(SRC_EXT))
OBJS := $(SRCS:$(SRC_DIR)/%.$(SRC_EXT)=$(BUILD_DIR)/%.$(OBJ_EXT))
DEPS := $(OBJS:.$(OBJ_EXT)=.$(DEP_EXT))

all: createDirs $(BIN)

#Make the Directories
createDirs:
	@mkdir -p $(TARGET_DIR)
	@mkdir -p $(BUILD_DIR)

# Link
$(BIN): $(OBJS)
	@$(COMPILER) $(FLAGS) $(LINKS) $^ -o $@

# Compile
$(BUILD_DIR)/%.$(OBJ_EXT): $(SRC_DIR)/%.$(SRC_EXT)
	@mkdir -p $(@D)
	@$(COMPILER) $(FLAGS) -c $< -o $@

clean:
	@$(RM) -rf $(TARGET_DIR)/* $(BUILD_DIR)/*

.PHONY: all clean

-include $(DEPS)