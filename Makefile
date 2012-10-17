
CFLAGS 			:= -Wall -Werror -std=gnu99

LDLIBS 			+= -lmad
LDLIBS 			+= -lao

BINNAME 		:= trans

SOURCE 			:= ${wildcard *.c}
OBJECT 			:= ${patsubst %.c, %.o, $(SOURCE)}

.PHONY: 		all clean

all: 			$(OBJECT) main

main: 			$(OBJECT)
	$(CC) $(LDLIBS) $(CFLAGS) $(OBJECT) -o $(BINNAME)

clean: 
	-rm *.o
	-rm $(BINNAME)
