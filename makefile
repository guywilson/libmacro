###############################################################################
#                                                                             #
# MAKEFILE for libmacro                                                       #
#                                                                             #
# (c) Guy Wilson 2022                                                         #
#                                                                             #
###############################################################################

# Directories
SOURCE = src
BUILD = build
DEP = dep
LIB = lib

# What is our target
TARGET = libmacro.so
TEST = test

# Tools
C = gcc-11
LINKER = gcc-11

# postcompile step
PRECOMPILE = @ mkdir -p $(BUILD) $(DEP)
# postcompile step
POSTCOMPILE = @ mv -f $(DEP)/$*.Td $(DEP)/$*.d

PRELINK = @ mkdir -p $(LIB)

CFLAGS = -c -O2 -Wall -pedantic
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP)/$*.Td

COMPILE.c = $(C) $(CFLAGS) $(DEPFLAGS) -o $@
LINK.o = $(LINKER) -o $@

CSRCFILES = $(wildcard $(SOURCE)/*.c)
OBJFILES = $(patsubst $(SOURCE)/%.c, $(BUILD)/%.o, $(CSRCFILES))
DEPFILES = $(patsubst $(SOURCE)/%.c, $(DEP)/%.d, $(CSRCFILES))

all: $(TARGET) $(TEST)

# Compile C/C++ source files
#
$(TARGET): $(OBJFILES)
	$(PRELINK)
	$(LINKER) -shared -o $(LIB)/$(TARGET) $^

$(TEST): $(BUILD)/test.o $(TARGET)
	$(LINKER) -L$${HOME}/lib -lmacro -o $@ $< 

$(BUILD)/test.o: test.c
	$(PRECOMPILE)
	$(C) $(CFLAGS) -I$${HOME}/include -o $@ $<

$(BUILD)/%.o: $(SOURCE)/%.c
$(BUILD)/%.o: $(SOURCE)/%.c $(DEP)/%.d
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

.PRECIOUS = $(DEP)/%.d
$(DEP)/%.d: ;

-include $(DEPFILES)

install: $(TARGET)
	cp $(LIB)/$(TARGET) ~/lib
	cp $(SOURCE)/libmacro.h ~/include

clean:
	rm -r $(BUILD)
	rm -r $(DEP)
	rm $(TARGET)
