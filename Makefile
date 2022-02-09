NAME        ?= IndiumCE
DESCRIPTION ?= IndiumCE - A faster BASIC interpreter
COMPRESSED  ?= YES

CFLAGS ?= -Wall -Wextra -O3

include $(shell cedev-config --makefile)
