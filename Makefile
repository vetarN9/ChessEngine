CC = g++
CFLAGS = -Wall -std=c++17
LINKS =
SRC = src
OBJ = obj
SRCS = $(wildcard $(SRC)/*.cpp)
OBJS = $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
BIN = chessEngine

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LINKS)  -o $(BIN)

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(RM) $(BIN) $(OBJ)/*
