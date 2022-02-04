# ----------------------------
# Makefile Options
# ----------------------------

NAME := YATISH
DESCRIPTION := "Description"
COMPRESSED := NO
ARCHIVED := YES

HAS_PRINTF := NO

CFLAGS := -Wall -Wextra -Oz
CXXFLAGS := -Wall -Wextra -Oz

# ----------------------------

ifndef CEDEV
$(error CEDEV environment path variable is not set)
endif

include $(CEDEV)/meta/makefile.mk

bin/YATISH.8xg: bin/YATISH.8xp
	gcc -include $(GFXDIR)/bgimage.c -DOUTFILE='"$(BINDIR)/bgimage.bin"' -DOUTDATA=bgimage_data $(BINDIR)/mkbin.c -o $(BINDIR)/mkbin
	$(BINDIR)/mkbin
	convbin -j bin -i $(BINDIR)/bgimage.bin --archive --uppercase -k 8xv -o $(BINDIR)/bgimage.8xv -n YATISH02
	convbin -j 8x -i $(BINDIR)/bgimage.8xv -j 8x -i $(BINDIR)/YATISH.8xp -k 8xg-auto-extract -o $(BINDIR)/YATISH.8xg

group: bin/YATISH.8xg
