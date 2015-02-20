all:  ../../bin/ots_gui2

../../bin/ots_gui2:  ots_gui2.c  gtk_utils.c gtk_utils.h
	gcc -O -Wall `pkg-config --cflags gtk+-2.0` ots_gui2.c  \
        `pkg-config --libs gtk+-2.0`  -o ../../bin/ots_gui2

clean:
	rm -f ../../bin/ots_gui2
