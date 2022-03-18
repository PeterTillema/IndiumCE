NAME        ?= IndiumCE
DESCRIPTION ?= IndiumCE - A faster BASIC interpreter
COMPRESSED  ?= YES

CXXFLAGS ?= -Wall -Wextra -Oz

include $(shell cedev-config --makefile)
