REM  This build script is intended to be run from a Microsoft DOS CMD window
REM   and will use the MinGW C-compiler.
REM  It will compile all the tax-form programs.  But it will only build the 
REM   old Otk-based GUI; not the newer Gtk-based GUI.

gcc -O taxsolve_US_1040_2014.c -o ..\msbin\taxsolve_US_1040_2014
gcc -O taxsolve_US_1040_Sched_C_2014.c -o ..\msbin\taxsolve_US_1040_Sched_C_2014

gcc -O taxsolve_CA_540_2014.c   -o ..\msbin\taxsolve_CA_540_2014
gcc -O taxsolve_NC_D400_2014.c  -o ..\msbin\taxsolve_NC_D400_2014
gcc -O taxsolve_NJ_1040_2014.c  -o ..\msbin\taxsolve_NJ_1040_2014
gcc -O taxsolve_PA_40_2014.c    -o ..\msbin\taxsolve_PA_40_2014
gcc -O taxsolve_OH_IT1040_2014.c -o ..\msbin\taxsolve_OH_IT1040_2014
gcc -O taxsolve_VA_760_2014.c   -o ..\msbin\taxsolve_VA_760_2014
gcc -O taxsolve_NY_IT201_2014.c -o ..\msbin\taxsolve_NY_IT201_2014
gcc -O taxsolve_MA_1_2014.c     -o ..\msbin\taxsolve_MA_1_2014
gcc -O Run_taxsolve_GUI.c 	  -o ..\Run_taxsolve_GUI_mswin
