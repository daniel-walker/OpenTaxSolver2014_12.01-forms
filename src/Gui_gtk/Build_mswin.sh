# This script must be run from an MinGW MSYS command window;  (NOT a DOS CMD window.)

 echo "_____"
 echo "Building GUI"

 gcc -O -mms-bitfields -I /include/gtk-2.0 -I /include/cairo/ -I /include/glib-2.0 -I /lib/glib-2.0/include  -I /include/pango-1.0 -I /include/gtk-2.0/atk \
	-I /lib/gtk-2.0/include -I /inc ots_gui2.c -o ../../msbin/ots_gui2.exe  -L /lib -lgtk-win32-2.0 -lgdk-win32-2.0 -latk-1.0 -lgio-2.0 \
	-lpangoft2-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lm -lcairo -lpng14 -lpango-1.0 -lfreetype -lfontconfig -lgobject-2.0 \
	-lgmodule-2.0 -lgthread-2.0 -lglib-2.0  

