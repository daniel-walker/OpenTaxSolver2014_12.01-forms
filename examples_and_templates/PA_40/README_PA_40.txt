Open Tax Solver - Pennsylvania State PA-40 Personal Income Tax Return
---------------------------------------------------------------------

Included here is a program, template, and example for
Pennsylvania State PA-40 personal income tax form.

The PA40_2014_example.txt file is included for testing.
The PA40_2014_template.txt file is a blank starting form for entering
your tax data.  For each filer, copy template to a new name, 
such as "PA40_2014.txt" or "PA40_2014_aunt_sally.txt, and 
fill-in the lines.

The program consists of two files:
  taxsolve_PA40_2014.c - main, customized for PA-40.
  taxsolve_routines.c - general purpose base routines.

Compile:
  cc taxsolve_PA40_2014.c -o taxsolve_PA40_2014

Run:
  ./taxsolve_PA40_2014  PA40_2014.txt

To download the pdf forms and fill them out run the following,
  ../../bin/fill_form_PA_40_2014 PA40_2014_out.txt 

For updates and further information, see:
        http://sourceforge.net/projects/opentaxsolver/
Documentation:
        http://opentaxsolver.sourceforge.net/


Aston Roberts (aston_roberts@yahoo.com)
