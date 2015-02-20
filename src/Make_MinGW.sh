# This build script is intended to be run from an MSYS command window in Microsoft,
# which interprets Unix-shell-style syntax;  Do NOT run this script from a DOS CMD window.
# This script will compile all the tax-form programs as well as the newer Gtk-based GUI.

gcc -O taxsolve_US_1040_2014.c -o ../msbin/taxsolve_US_1040_2014
gcc -O taxsolve_US_1040_Sched_C_2014.c -o ../msbin/taxsolve_US_1040_Sched_C_2014

gcc -O taxsolve_CA_540_2014.c   -o ../msbin/taxsolve_CA_540_2014
gcc -O taxsolve_NC_D400_2014.c  -o ../msbin/taxsolve_NC_D400_2014
gcc -O taxsolve_NJ_1040_2014.c  -o ../msbin/taxsolve_NJ_1040_2014
gcc -O taxsolve_PA_40_2014.c    -o ../msbin/taxsolve_PA_40_2014
gcc -O taxsolve_OH_IT1040_2014.c -o ../msbin/taxsolve_OH_IT1040_2014
gcc -O taxsolve_VA_760_2014.c   -o ../msbin/taxsolve_VA_760_2014
gcc -O taxsolve_NY_IT201_2014.c -o ../msbin/taxsolve_NY_IT201_2014
gcc -O taxsolve_MA_1_2014.c     -o ../msbin/taxsolve_MA_1_2014
gcc -O Run_taxsolve_GUI.c 	-o ../Run_taxsolve_GUI
cd Gui_gtk
  source ./Build_mswin.sh
cd ..

