CC = gcc
CFLAGS = -Wall -pthread -I../include
LDFLAGS = -pthread
OBJDIR = ../obj
BINDIR = ../bin

COMMON_SRC = ../src/server.c ../src/user.c ../src/encryption.c ../src/logger.c
COMMON_OBJ = $(patsubst ../src/%.c,$(OBJDIR)/%.o,$(COMMON_SRC))
COMMON_TARGET = common

MPCC_SRC = ../src/main.c ../src/client.c
MPCC_OBJ = $(patsubst ../src/%.c,$(OBJDIR)/%.o,$(MPCC_SRC)) $(COMMON_OBJ)
MPCC_TARGET = $(BINDIR)/clientServer_exe

.PHONY: all clean

all: $(MPCC_TARGET)

$(MPCC_TARGET): $(MPCC_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: ../src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)/* $(BINDIR)/*
