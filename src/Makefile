CFLAGS  = -DLINUX 
LIBS    = -lm -lc -lz
CC      = gcc -Wall -Wno-char-subscripts -g

OUTFILE = ../bin/lola

OBJECTS01 =   reset.o        edit.o         act_comm.o     act_info.o	  \
              act_move.o     act_obj.o      act_wiz.o      comm.o         \
              const.o        db.o           fight.o        handler.o      \
              interp.o       magic.o        save.o         special.o      \
              update.o       mob_prog.o     mob_commands.o castle.o       \
              clans.o        str_hash.o     race.o         drunkify.o     \
              debug.o        mob_handler.o  games.o        skills.o       \
              data.o         mount.o        map.o          telopt.o       \
              purger.o       bounty.o       dynamic.o      pathfind.o     \
              math.o         vt100.o        language.o     economy.o      \
              socials.o      olc.o          msdp.o                \
              color.o        mth.o

SRC_FILES =   reset.c        edit.c         act_comm.c     act_info.c	  \
              act_move.c     act_obj.c      act_wiz.c      comm.c         \
              const.c        db.c           fight.c        handler.c      \
              interp.c       magic.c        save.c         special.c      \
              update.c       mob_prog.c     mob_commands.c castle.c       \
              clans.c        str_hash.c     race.c         drunkify.c     \
              debug.c        mob_handler.c  games.c        skills.c       \
              data.c         mount.c        map.c          telopt.c       \
              purger.c       bounty.c       dynamic.c      pathfind.c     \
              math.c         vt100.c        language.c     economy.c      \
              socials.c      olc.c          msdp.c                \
              color.c        mth.o

$(OUTFILE): $(OBJECTS01) 
	$(CC) $(CFLAGS) $(OBJECTS01) -o $(OUTFILE) $(LIBS)

TEMP = $(patsubst %, '\#include "%"', $(SRC_FILES))

clean:
	rm -f $(OBJECTS01) 

full:
	rm -f full.c;
	for i in $(TEMP); do echo $$i >> full.c; done
#	$(OUTFILE): full.o 
	$(CC) $(CFLAGS) full.c -o $(OUTFILE) $(LIBS)
	
port:
	rm $(OBJECTS01);
	make 
