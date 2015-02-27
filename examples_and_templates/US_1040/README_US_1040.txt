Open Tax Solver - US Federal 1040 Personal Income Tax Return
------------------------------------------------------------

Included here is a program, template, and example for
US Federal 1040 personal income tax form.

The example*.txt file is included for testing.
The template*.txt file is a blank starting form for entering
your tax data.  For each filer, copy template to a new name, 
such as "fed1040_2014.txt" or "fed1040_2014_aunt_sally.txt, and 
fill-in the lines.

The program consists of two files:
  src/taxsolve_US_1040_2014.c - main, customized for US Fed-1040.
  src/taxsolve_routines.c - general purpose base routines.

Compile:
  cc src/taxsolve_US_1040_2014.c -o bin/taxsolve_US_1040_2014 

Run from this directory:
  ../../bin/taxsolve_US_1040_2014   Fed1040_???_2014.txt

... Where "Fed1040_???_2014.txt" is the name of -your- tax data file.
(Two tax-data files are included in OTS packages: an *example.txt
 and a blank *template.txt. The idea is to copy the template to a
 personally meaningful file-name and fill in the lines with your
 numbers. You can maintain returns for multiple people over
 multiple years this way.) 

To download the pdf forms and fill them out run the following,
  ../../bin/fill_form_US_1040_2014 Fed1040_???_2014.txt

For updates and further information, see:
        http://sourceforge.net/projects/opentaxsolver/
Documentation:
        http://opentaxsolver.sourceforge.net/


Aston Roberts (aston_roberts@yahoo.com)
