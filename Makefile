NAME        ?= IndiumCE
DESCRIPTION ?= IndiumCE - A faster BASIC interpreter
COMPRESSED  ?= YES

CXXFLAGS ?= -Wall -Wextra -Oz

include $(shell cedev-config --makefile)

$(OBJDIR)/font/font.src: $(SRCDIR)/font/font.inc

$(SRCDIR)/font/font.inc: $(SRCDIR)/font/font.fnt
	convfont -o carray -f $(SRCDIR)/font/font.fnt $(SRCDIR)/font/font.inc
