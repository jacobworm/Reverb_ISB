# Project Name
TARGET = Reverb

# Sources
CPP_SOURCES = main.cpp

# Library Locations
LIBDAISY_DIR = ./libDaisy/
DAISYSP_DIR = ./DaisySP/

# Library Flags
LDFLAGS += -u _printf_float
LDFLAGS += -Wl,--print-memory-usage


# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
