SHELL := /bin/bash
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

SRCDIR = src
BUILDDIR = build
BINDIR = bin

SRC_FILES = $(wildcard $(SRCDIR)/*.c)
OBJ_FILES = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRC_FILES))

TARGET = $(BINDIR)/pipelinevm
ASSEMBLER = $(BINDIR)/assembler

VM_OBJ_FILES = build/main.o build/pipeline.o build/viz.o build/vm.o build/isa.o build/memory.o


.PHONY: all clean assembler

all: $(TARGET) $(ASSEMBLER)

$(ASSEMBLER): $(BUILDDIR)/assembler.o
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $< -o $(BINDIR)/assembler

$(TARGET): $(VM_OBJ_FILES)
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR) $(BINDIR)
