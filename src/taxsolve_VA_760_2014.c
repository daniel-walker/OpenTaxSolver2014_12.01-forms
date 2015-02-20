/************************************************************************/
/* TaxSolve_VA760_2014.c - 						*/
/* Copyright (C) 2015 - Aston Roberts					*/
/* 									*/
/* Compile:   gcc taxsolve_VA760_2014.c -o taxsolve_VA760_2014		*/
/* Run:	      ./taxsolve_VA760_2014  VA_760_2014.txt 			*/
/* 									*/
/* GNU Public License - GPL:						*/
/* This program is free software; you can redistribute it and/or	*/
/* modify it under the terms of the GNU General Public License as	*/
/* published by the Free Software Foundation; either version 2 of the	*/
/* License, or (at your option) any later version.			*/
/* 									*/
/* This program is distributed in the hope that it will be useful,	*/
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU	*/
/* General Public License for more details.				*/
/* 									*/
/* You should have received a copy of the GNU General Public License	*/
/* along with this program; if not, write to the Free Software		*/
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA		*/
/* 02111-1307 USA							*/
/* 									*/
/* Aston Roberts 1-9-2015	aston_roberts@yahoo.com			*/
/************************************************************************/

#include <stdio.h>
#include <time.h>

#include "taxsolve_routines.c"

float thisversion=12.0;

#define SINGLE 		        1
#define MARRIED_FILLING_JOINTLY 2
#define MARRIED_FILLING_SEPARAT 3
#define HEAD_OF_HOUSEHOLD       4
#define WIDOW		        5

double TaxRateFunction( double x, int status )
{
 if (x < 3000.0) return x * 0.02; else
 if (x < 5000.0) return  60.0 + (x - 3000.0) * 0.03; else
 if (x < 17000.0) return 120.0 + (x - 5000.0) * 0.05; else
 return 720.0 + (x - 17000.0) * 0.0575; 
}



/*----------------------------------------------------------------------------*/

int main( int argc, char *argv[] )
{
 int i, j, k;
 char word[1000], outfname[4000];
 int status=0, exemptionsA=0, exemptionsB=0;
 time_t now;
 double L20b=0.0, std_ded=0.0, min2file;

 /* Intercept any command-line arguments. */
 printf("VA-760 2014 - v%3.1f\n", thisversion);
 i = 1;  k=1;
 while (i < argc)
 {
  if (strcmp(argv[i],"-verbose")==0)  verbose = 1;
  else
  if (k==1)
   {
    infile = fopen(argv[i],"r");
    if (infile==0) {printf("ERROR: Parameter file '%s' could not be opened.\n", argv[i]); exit(1);}
    k = 2;
    /* Base name of output file on input file. */
    strcpy(outfname,argv[i]);
    j = strlen(outfname)-1;
    while ((j>=0) && (outfname[j]!='.')) j--;
    if (j<0) strcat(outfname,"_out.txt"); else strcpy(&(outfname[j]),"_out.txt");
    outfile = fopen(outfname,"w");
    if (outfile==0) {printf("ERROR: Output file '%s' could not be opened.\n", outfname); exit(1);}
    printf("Writing results to file:  %s\n", outfname);
   }
  else {printf("Unknown command-line parameter '%s'\n", argv[i]); exit(1);}
  i = i + 1;
 }

 if (infile==0) {printf("Error: No input file on command line.\n"); exit(1);}

 /* Pre-initialize all lines to zeros. */
 for (i=0; i<MAX_LINES; i++) 
  {
   L[i] = 0.0;
  }

 /* Accept parameters from input file. */
 /* Expect  VA-760 lines, something like:
	Title:  VA-760 1999 Return
	L1		{Wages}
*/


 /* Accept Form's "Title" line, and put out with date-stamp for records. */
 read_line( infile, word );
 now = time(0);
 fprintf(outfile,"\n%s		%s\n", word, ctime( &now ));

 /* get_parameter(infile, kind, x, emssg ) */
 get_parameter( infile, 's', word, "Status" );
 get_parameter( infile, 'l', word, "Status ?");
 if (strncasecmp(word,"Single",4)==0) status = SINGLE; else
 if (strncasecmp(word,"Married/Joint",13)==0) status = MARRIED_FILLING_JOINTLY; else
 if (strncasecmp(word,"Married/Sep",11)==0) status = MARRIED_FILLING_SEPARAT; else
 if (strncasecmp(word,"Head_of_House",4)==0) status = HEAD_OF_HOUSEHOLD;
 else
  { 
   printf("Error: unrecognized status '%s'. Must be: Single, Married/joint, Married/sep, Head_of_house, Widow(er)\nExiting.\n", word); 
   fprintf(outfile,"Error: unrecognized status '%s'. Must be: Single, Married/joint, Married/sep, Head_of_house, Widow(er)\nExiting.\n", word); 
   exit(1); 
  }
 fprintf(outfile,"Status = %s (%d)\n", word, status);

 get_parameter( infile, 's', word, "Exemptions_A" );	/* Exemptions_A: self/spouse, dependents. */
 get_parameters( infile, 'i', &exemptionsA, "Exemptions_A"); 
 fprintf(outfile,"Exemptions = %d\n", exemptionsA );

 get_parameter( infile, 's', word, "Exemptions_B" );	/* Exemptions_B: Over 65, Blind for self/spouse  */
 get_parameters( infile, 'i', &exemptionsB, "Exemptions_B"); 
 fprintf(outfile,"Exemptions = %d\n", exemptionsB );

 GetLineF( "L1", &L[1] );	/* Federal Adjusted Gross Income */

 GetLineF( "L2", &L[2] );	/* Additions from attached Schedule ADJ, line 3 */

 L[3] = L[1] + L[2];
 showline(3);

 GetLineF( "L4", &L[4] );	/* Deduction for age on Jan 1, 2015. */

 GetLineF( "L5", &L[5] );	/* Social Security Act, Tier 1 Railroad Retirement Act benef. */

 GetLineF( "L6", &L[6] );	/* State Income Tax refund or overpayment credit */

 GetLineF( "L7", &L[7] );	/* Subtractions from Schedule ADJ, line 7 */

 L[8] = L[4] + L[5] + L[6] + L[7];
 showline(8);

 L[9] = L[3] - L[8]; 
 showline(9);			/* Virginia Adjusted Gross Income (VAGI) */

 GetLineF( "L10", &L[10] );	/* Deductions - Std or Itemized minus income taxes */

 GetLineF( "L11", &L[11] );	/* State and Local Income Taxes claimed on federal Schedule A. */

 switch (status)
  {
   case SINGLE:  		  std_ded = 3000.0;  min2file = 11950.0;  break;
   case MARRIED_FILLING_JOINTLY:  std_ded = 6000.0;  min2file = 23900.0;  break;
   case MARRIED_FILLING_SEPARAT:  std_ded = 3000.0;  min2file = 11950.0;  break;
   default:  printf("Unexpected status.\n");
	     fprintf(outfile,"Unexpected status.\n");
	     exit(1);  
	break;
  }

 L[12] = L[10] - L[11];
 if (L[12] < std_ded) L[12] = std_ded;
 showline(12);

 L[13] = 930.0 * exemptionsA + 800.0 * exemptionsB;
 showline(13);
  
 GetLineF( "L14", &L[14] );	/* Deductions from Virginia Adjusted Gross Income Schedule ADJ, Line 9. */

 L[15] = L[12] + L[13] + L[14];
 showline(15);

 L[16] = L[9] - L[15];
 showline(16);

 L[17] = TaxRateFunction( L[16], status );
 showline(17);

 GetLine( "L18", &L[18] );	/* Spouse Tax Adjustment. */
 showline(18);

 L[19] = L[17] - L[18];
 showline_wmsg( 19, "Net Amount of Tax" );	

 GetLineF( "L20a", &L[20] );	/* Virginia tax withheld for 2014. */
 GetLineF( "L20b", &L20b );	/* Spouse's Virginia tax withheld. */

 GetLineF( "L21", &L[21] );	/* Estimated tax paid for 2014. (form 760ES) */

 GetLineF( "L22", &L[22] );	/* Amount of 2013 overpayment applied toward 2014 estimated tax. */

 GetLineF( "L23", &L[23] );	/* Extension payments (form 760E). */

 GetLine( "L24", &L[24] );	/* Tax Credit, Low Income Individuals (Sch. ADJ, line 17) */

 if (L[24] > L[19]) L[24] = L[19];	/* Low-Income Credit cannot exceed tax liability. */

 if ((L[24] > 0.0) && (exemptionsB > 0.0))
  {
   fprintf(outfile," Cannot claim both Low-Income Credit and Age or Blind Examptions.\n");
   L[24] = 0.0;	/* Cannot claim both low-income credit and exemptions. */
  }
 showline(24);

 GetLineF( "L25", &L[25] );	/* Credit, Tax Paid to other State (Sched OSC, line 21 ...) */
 GetLineF( "L26", &L[26] );	/* Credit for Political Contributions */
 GetLineF( "L27", &L[27] );	/* Credits from enclosed Schedule CR, Section 5, Part 1, Line 1A */

 L[28] = L[20] + L20b + L[21] + L[22] + L[23] + L[24] + L[25] + L[26] + L[27];
 showline(28);

 if (L[28] < L[19])
  {
   L[29] = L[19] - L[28];
   showline_wmsg( 29, "Tax You Owe" );
  }
 else
  {
   L[30] = L[28] - L[19];
   showline_wmsg( 30, "Your Tax OverPayment" );
  }

 GetLineF( "L31", &L[31] );	/* Amount of overpayment you want credited to next year's estimated tax. */
 GetLineF( "L32", &L[32] );	/* Virginia College Savings Plan Contributions from Schedule VAC, Section I, Line 6. */
 GetLineF( "L33", &L[33] );	/* Other voluntary contribitions. */
 GetLineF( "L34", &L[34] );	/* Addition to Tax, Penalty and Interest from attached Schedule ADJ, Line 21 */
 GetLineF( "L35", &L[35] );	/* Consumer's Use Tax. */

 for (j=31; j < 35; j++)
   L[36] = L[36] + L[j];
 showline(36);

 if (L[29] > 0.0)
  {
   L[37] = L[29] + L[36];
   showline_wmsg( 37, "AMOUNT YOU OWE" );
  }
 else
 if (L[30] < L[36])
  {
   L[37] = L[36] - L[30];
   showline_wmsg( 37, "AMOUNT YOU OWE" );
  }
 else
 if (L[30] > L[36])
  {
   L[38] = L[30] - L[36];
   showline_wmsg( 38, "YOUR REFUND" );
  }

 if (L[9] < min2file)
  {
   fprintf(outfile,"\nYour VAGI is less than the minimum required to file a return.\n");
   if (L[20] + L20b + L[21] > 0.0)
    fprintf(outfile," But you need to file return to receive refund of withheld taxes.\n");
   else
    fprintf(outfile,"You do not need to file return.  Your VA Tax is zero.\n");
  }

 fclose(infile);
 fclose(outfile);
 Display_File( outfname );
 printf("\nResults written to file:  %s\n", outfname);
 return 0;
}
