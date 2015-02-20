Open Tax Solver - 1040 Schedule C - Profit Loss from Business
-------------------------------------------------------------

Included here is a program, template, and example for
US Fed1040 Schedule-C  Profit Loss from Business.

The example*.txt file is included for testing.
The template*.txt file is a blank starting form for entering
your tax data.  For each filer, copy template to a new name, 
such as "SchedC_2014.txt" or "SchedC_2014_BakeryCo.txt, and 
fill-in the lines.

The program consists of two files:
  taxsolve_US1040_Sched_C.c - main, customized for Sched-C.
  taxsolve_routines.c - general purpose base routines.

Compile:
  cc taxsolve_US1040_Sched_C.c -o taxsolve_US1040_Sched_C

Run:
  ./taxsolve_US1040_Sched_C  Sched_C.txt


For updates and further information, see:
        http://sourceforge.net/projects/opentaxsolver/
Documentation:
        http://opentaxsolver.sourceforge.net/


Contributed by S. Jenkins
Minor formatting by Aston Roberts 
Updated for 2004 by Robert Heller:
Updated for 2010-2104 by Aston Roberts.
