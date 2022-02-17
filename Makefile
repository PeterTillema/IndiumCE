NAME        ?= IndiumCE
DESCRIPTION ?= IndiumCE - A faster BASIC interpreter
COMPRESSED  ?= YES

CFLAGS ?= -Wall -Wextra -O3

include $(shell cedev-config --makefile)

$(OBJDIR)/font/font.src: $(SRCDIR)/font/font.inc

$(SRCDIR)/font/ont.inc: $(SRCDIR)/font/font.fnt
	convfont -o carray -f $(SRCDIR)/font/font.fnt $(SRCDIR)/font/font.inc
