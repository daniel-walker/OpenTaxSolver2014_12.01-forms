All:  otk_lib.o gadget_lib.o

otk_lib.o:  otk_lib.c  otk_lib.h
	cc -c -O -I/usr/X11R6/include otk_lib.c -o otk_lib.o

gadget_lib.o:  gadget_lib.c gadget_lib.h
	cc -c -O gadget_lib.c -o gadget_lib.o

clean:
	rm -f otk_lib.o gadget_lib.o
