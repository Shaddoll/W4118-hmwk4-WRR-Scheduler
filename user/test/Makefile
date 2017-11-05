# Set this to the name of your program
TARGET = test

# Edit this variable to point to all
# of your sources files (make sure
# to put a complete relative path)
SOURCES = test.c

# -----------------------------------------------
#
# Don't touch things below here unless
# you know what you're doing :-)
#
OBJECTS = $(SOURCES:%.c=%.c.o)
INCLUDE = -I$(A24_INCLUDE)
LINK = -L$(A24_LINK)
SYSROOT = --sysroot $(A24_SYSROOT)
CFLAGS = -g -O2 -Wall $(INCLUDE) -static
LDFLAGS = -static $(LINK) $(SYSROOT)
CC = aarch64-linux-android-gcc
LD = aarch64-linux-android-gcc

default: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	@echo [Arm64-LD] $@
	@$(LD) $(LDFLAGS) $(OBJECTS) -o $@

%.c.o: %.c
	@echo [Arm64-CC] $<...
	@$(CC) -c $(CFLAGS) $< -o $@

clean: .PHONY
	@echo [CLEAN]
	@rm -f $(OBJECTS) $(TARGET)

.PHONY:
