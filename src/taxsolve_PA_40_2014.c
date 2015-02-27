/************************************************************************/
/* TaxSolve_PA40_2014.c - Pennsylvania 2014 PA-40 State Tax Form.	*/
/* Copyright (C) 2015, - Aston Roberts					*/
/* 									*/
/* Compile:   gcc taxsolve_PA40_2014.c -o taxsolve_PA40_2014		*/
/* Run:	      ./taxsolve_PA40_2014  PA40_2014.txt 			*/
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
/* Aston Roberts 1-15-2015	aston_roberts@yahoo.com			*/
/************************************************************************/

#define version 12.0

#include "taxsolve_routines.c"
#include "taxsolve_PA_40_2014_forms.h"

#define SINGLE 		        1
#define MARRIED_FILLING_JOINTLY 2
#define MARRIED_FILLING_SEPARAT 3
#define WIDOW		        1
#define Yes 1
#define No  0



int main( int argc, char *argv[] )
{
 int i, j, k, status=0;
 char word[2000], outfname[1500], pa40_xfdf_outfname[1500];
 time_t now;
 double oneA, oneB;

 /* Decode any command-line arguments. */
 printf("PA40 - 2014 - v%3.1f\n", version);
 i = 1;  k=1;
 while (i < argc)
 {
  if (strcmp(argv[i],"-verbose")==0)  { verbose = 1; }
  else
  if (k==1)
   {
    infile = fopen(argv[i],"r");
    if (infile==0) {printf("ERROR: Parameter file '%s' could not be opened.\n", argv[i]); exit(1);}
    k = 2;
    /* Base name of output file on input file. */
    strcpy(outfname,argv[i]);
    strcpy(pa40_xfdf_outfname,argv[i]);
    j = strlen(outfname)-1;
    while ((j>=0) && (outfname[j]!='.')) j--;
    if (j<0) {
     strcat(outfname,"_out.txt");
     strcat(pa40_xfdf_outfname,"_pa40.xfdf");
    } else {
     strcpy(&(outfname[j]),"_out.txt");
     strcpy(&(pa40_xfdf_outfname[j]),"_pa40.xfdf");
    }
    outfile = fopen(outfname,"w");
    if (outfile==0) {printf("ERROR: Output file '%s' could not be opened.\n", outfname); exit(1);}
    printf("Writing results to file:  %s\n", outfname);
   }
  else
   {printf("Unknown command-line parameter '%s'\n", argv[i]); exit(1);}
  i = i + 1;
 }

 if (infile==0) {printf("Error: No input file on command line.\n"); exit(1);}

 /* Pre-initialize all lines to zeros. */
 for (i=0; i<MAX_LINES; i++) L[i] = 0.0;

 /* Accept parameters from input file. */
 /* Expect  PA-40 lines, something like:
	Title:  PA 40 1999 Return
	L12	34900.0  {Wages}
 */

 /* Accept Form's "Title" line, and put out with date-stamp for your records. */
 read_line( infile, word );
 now = time(0);
 fprintf(outfile,"\n%s		%s\n", word, ctime( &now ));

 /* get_parameter(infile, kind, x, emssg ) */
 get_parameter( infile, 's', word, "Status" );	/* Single, Married/joint, Married/sep, Widow(er) */
 get_parameter( infile, 'l', word, "Status?");
 if (strncasecmp(word,"Single",4)==0) { status = SINGLE; L["Single"] = 1; }else
 if (strncasecmp(word,"Married/Joint",13)==0) { status = MARRIED_FILLING_JOINTLY; L["MFJ"] = 1; }else
 if (strncasecmp(word,"Married/Sep",11)==0) { status = MARRIED_FILLING_SEPARAT; L["MFS"] = 1; } else
 if (strncasecmp(word,"Widow",4)==0) { status = WIDOW; L["Deceased"] = 1; }
 else
  { 
   printf("Error: unrecognized status '%s'. Exiting.\n", word); 
   fprintf(outfile,"Error: unrecognized status '%s'. Exiting.\n", word); 
   exit(1);
  }
 fprintf(outfile,"Status = %s (%d)\n", word, status);

 GetLineF( "L1a", &oneA );	/* Gross compensation. */
 L["1a"] = oneA;
 GetLineF( "L1b", &oneB );	/* Unreimbursed employee business expenses. */
 L["1b"] = oneB;

 L[1] = oneA - oneB;
 L["1c"] = L[1];

 fprintf(outfile,"L1c = %2.2f\n", L[1] );		/* Net compensation. */
 
 GetLineF( "L2", &L[2] );	/* Interest Income. */

 GetLineF( "L3", &L[3] );	/* Dividend Income. */

 GetLineF( "L4", &L[4] );	/* Income or loss for business operations. */
 if (L[4] < 0) L["4l"] = 1;
 GetLineF( "L5", &L[5] );	/* Net gain or loss from disposition of property. */
 if (L[5] < 0) L["5l"] = 1;

 GetLineF( "L6", &L[6] );	/* Net gain or loss rents, royalties, patents, or copyrights. */
 if (L[6] < 0) L["6l"] = 1;
 GetLineF( "L7", &L[7] );	/* Estate or Trust Income. */

 GetLineF( "L8", &L[8] );	/* Gambling or lottery winnings. */

 for (j=1; j<=8; j++) if (L[j] < 0.0) L[j] = 0.0;

 L[9] = L[1] + L[2] + L[3] + L[4] + L[5] + L[6] + L[7] + L[8];
 showline_wmsg(9,"Total PA Taxable Income");

 GetLineF( "L10", &L[10] );	/* Other Deductions. */

 L[11] = L[9] - L[10];
 showline_wmsg(11,"Adjusted PA Taxable Income"); /* Adjusted PA income. */

 L[12] = 0.0307 * L[11];
 showline_wmsg(12,"PA Tax Liability");		/* PA Tax liability. */

 GetLine( "L13", &L[13] );	/* Total PA Tax withheld. */
 showline_wmsg(13,"Total PA tax withheld");

 GetLineF( "L14", &L[14] );	/* Credit from last year's PA income tax return. */

 GetLineF( "L15", &L[15] );	/* 2014 Estimated Installment payments. */

 GetLineF( "L16", &L[16] );	/* 2014 Extension payment. */

 GetLineF( "L17", &L[17] );	/* Non-resident tax withheld. */

 L[18] = L[14] + L[15] + L[16] + L[17];
 showline_wmsg(18,"Total Estimated Payments and Credits");

 GetLine( "L21", &L[21] );	/* Tax Forgiveness Credit from Part D, Line 16, PA Schedule SP. */
 showline_wmsg(21,"Tax Back/Tax Foregiveness Credit");

 GetLineF( "L22", &L[22] );	/* Resident credit (Scheds G/RK-1). */

 GetLineF( "L23", &L[23] );	/* Other credits (Sched OC). */

 L[24] = L[13] + L[18] + L[21] + L[22] + L[23];
 showline_wmsg(24,"Total Payments and Credits");

 GetLineF( "L25", &L[25] );	/* Use Tax. */
 GetLine( "L27", &L[27] );	/* Penalties and interest. */

 if (L[12] + L[25] > L[24])
  {
   L[26] = L[12] + L[25] - L[24];
   showline_wmsg(26,"TAX DUE");
   showline(27);
   L[28] = L[26] + L[27];
   if (L[28] > 0.0) showline_wmsg( 28, "Total Payment Due" );
  }
 else
 if (L[24] > L[12] + L[25] + L[27])
  {
   showline(27);
   L[29] = L[24] - (L[12] + L[25] + L[27]);
   showline_wmsg(29,"OVERPAYMENT");
   L[30] = L[29];
   showline_wmsg(30,"REFUND");
  }
 
 fclose(infile);
 fclose(outfile);

 outfile = fopen(pa40_xfdf_outfname,"w");
 output_xfdf_form_data(outfile, pa40_2014, L); 
 fclose(outfile);

 printf("\nListing results from file: %s\n\n", outfname);
 Display_File( outfname );
 return 0;
}
