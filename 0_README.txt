Open Tax Solver - Sources Package
---------------------------------

 Feb 6, 2015 v12.01 - For 2014 tax-year.
    Contains:
	- US-1040 - which also does the Schedules A, B, D, and forms 8949. 
	- Scehdule C for US-1040.
	- State Income Taxes for Virginia, Ohio, California,
	  New York, Massachusetts, New Jersey, North Carolina, 
	  and Pennsylvania.
	All updated for 2014 tax-year.
  History:
    * v12.01 (2/6/15) - GUI - Deactived button for PDF Auto-Fillout.
		      - US 1040 - Fixed printout of last line number (76).
		      - Updated years in old Gui (for what it is worth).
    * v12.00 (1/29/15) - Initial Release for Tax Year 2014.

--- This package contains executables for 64-bit Linux. ---

Usage:
 To run the GUI:
    ./Run_taxsolve_GUI
  (Located in the top directory.)

General:
Example tax-data files and starting templates are included under
the examples_and_templates directory.  For each filer, save filled-out 
template to a new name, such as "fed1040_2014.txt".  After filling-in the
lines, then run the tax solver on it.  From the GUI, this is done by
pressing "Compute Tax" button.
Or solvers can be run from the command-line, for example, as:
  bin/taxsolve_usfed1040_2014  Fed1040_2014.txt
Where "Fed1040_2014.txt" is the name of -your- tax-data file, which
you can edit with your favorite text-editor to fill it in or print
it out.  Output results are saved to "..._out.txt" files
(eg. Fed1040_2014_out.txt), and can be printed out directly too.

For updates and further information, see:
        http://sourceforge.net/projects/opentaxsolver/
Documentation:
        http://opentaxsolver.sourceforge.net/

Re-compiling:
 Unix/Linux/Mac:
  Pre-compiled executables for Unix/Linux are normally in bin directory.
  To build the binaries in the bin/ directory:
     rm ./bin/*                          ( Clears bin/ directory. )
     ./Build_taxsolve_packages_LINUX.sh   ( This command creates the bin/ directory)

 MS-Windows:
   Pre-compiled executables are normally in msbin\ directory.
    However to build (MinGW w/Msys is recommended), run src\Make_MinGW.sh.

Directory Structure:
OpenTaxSolver2014       ......................................       25.246-KB
   |-- src   .................................................      217.555-KB
   |   |-- Gui_gtk   ........................................      141.807-KB
   |
   |-- examples_and_templates   ..............................       49.152-KB
   |   |-- VA_760   ..........................................       11.038-KB
   |   |-- US_1040_Sched_C   .................................       12.803-KB
   |   |-- US_1040   .........................................       21.351-KB
   |   |-- PA_40   ...........................................       11.562-KB
   |   |-- OH_1040   .........................................       14.860-KB
   |   |-- NY_IT201   ........................................       14.686-KB
   |   |-- NJ_1040   .........................................       14.124-KB
   |   |-- NC_400   ..........................................       12.251-KB
   |   |-- MA_1   ............................................       13.844-KB
   |   |-- CA_540   ..........................................       13.166-KB
   |
   |-- bin   .................................................      351.033-KB
   |-- msbin   ...............................................      536.690-KB
16 Directories.

---------------------------------------------------------------------------------
Aston Roberts (aston_roberts@yahoo.com)
File Organization and Makefiles by: Krish Krothapalli, David Masterson, & Jesse Becker
Programs contain contributions by many others.  See OTS credits webpage.
