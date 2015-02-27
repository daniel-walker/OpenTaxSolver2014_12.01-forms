/************************************************************************/
/* TaxSolve_NJ_1040_2014.c - 						*/
/* Copyright (C) 2015 - Aston Roberts					*/
/* 									*/
/* Compile:   gcc taxsolve_NJ_1040_2014.c -o taxsolve_NJ_1040_2014	*/
/* Run:	      ./taxsolve_NJ_1040_2014  NJ_1040_2014.txt 		*/
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
/* Aston Roberts 1-5-2015	aston_roberts@yahoo.com			*/
/*    2-14-05   BWB							*/
/*      2-24-06 Further updates BWB					*/
/************************************************************************/

float thisversion=12.00;

#include <stdio.h>
#include <time.h>

#include "taxsolve_routines.c"
#include "taxsolve_NJ_1040_2014_forms.h"

double S[MAX_LINES], E[MAX_LINES];
Lmap A;

#define SINGLE 		        1
#define MARRIED_FILLING_JOINTLY 2
#define MARRIED_FILLING_SEPARAT 3
#define HEAD_OF_HOUSEHOLD       4
#define WIDOW		        5


double TaxRateFormula( double x, int status )
{

 if ((status==SINGLE) || (status==MARRIED_FILLING_SEPARAT))	/* Single, Married/sep */
  {
   if (x < 20000.0)   return x * 0.014;              else
   if (x < 35000.0)   return x * 0.0175  -    70.0;  else
   if (x < 40000.0)   return x * 0.035   -   682.5;  else
   if (x < 75000.0)   return x * 0.05525 -  1492.5;  else
   if (x < 500000.0)  return x * 0.0637  -  2126.25;
   else		      return x * 0.0897  - 15126.25;
  }
 else
 if ((status==MARRIED_FILLING_JOINTLY) || (status==HEAD_OF_HOUSEHOLD) || (status==WIDOW))
  {								/* Married/Joint, HouseHead, widower. */
   if (x < 20000.0)   return x * 0.014;             else
   if (x < 50000.0)   return x * 0.0175  -    70.0; else
   if (x < 70000.0)   return x * 0.0245  -   420.0; else
   if (x < 80000.0)   return x * 0.035   -  1154.5; else
   if (x < 150000.0)  return x * 0.05525 -  2775.0; else
   if (x < 500000.0)  return x * 0.0637  -  4042.5; 
   else		      return x * 0.0897  - 17042.5;
  }
 else {printf("Status not covered.\n"); exit(0);}
}


double TaxRateFunction( double income, int status )     /* Emulates table lookup or function appropriately. */
{
 double x, dx, tx;
 int k;

 if (income < 100000.0)  /* Quantize to match tax-table exactly. */
  {
   x = 50.0;
   dx = 0.5 * x;
   k = (income - 0.000001) / x;
   x = x * (double)k + dx;
   tx = (int)(TaxRateFormula( x, status ) + 0.5);
  }
 else
  tx = TaxRateFormula( income, status );
 return tx;
}



/*----------------------------------------------------------------------------*/

int main( int argc, char *argv[] )
{
 int i, j, k;
 char word[1000], outfname[4000], nj1040_outfname[4000], nj1040_sched_a_outfname[4000];
 int status=0;
 time_t now;
 int L12a=0, L12b=0;
 double L27a=0.0, L27b=0.0, L29a=0.0, L29b=0.0, L37a=0.0;
 double Ab[10], A9a=0.0, proptxcredit;
 double F[10], Fb[10];	/* Schedule F, added by BWB. */
 double I[10], Ib[10];	/* Schedule I. */

 /* Intercept any command-line arguments. */
 printf("NJ 1040 2014 - v%3.1f\n", thisversion);
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
    strcpy(nj1040_outfname,argv[i]);
    strcpy(nj1040_sched_a_outfname,argv[i]);
    j = strlen(outfname)-1;
    while ((j>=0) && (outfname[j]!='.')) j--;
    if (j<0) {
     strcat(outfname,"_out.txt");
     strcat(nj1040_outfname,"_nj1040.xfdf");
     strcat(nj1040_sched_a_outfname,"nj1040abc.xfdf");
    } else {
     strcpy(&(outfname[j]),"_out.txt");
     strcpy(&(nj1040_outfname[j]),"_nj1040.xfdf");
     strcpy(&(nj1040_sched_a_outfname[j]),"_nj1040abc.xfdf");
    }
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
   A[i] = 0.0;
   S[i] = 0.0;
   E[i] = 0.0;
  }

 /* Accept parameters from input file. */
 /* Expect  NJ-1040 lines, something like:
	Title:  NJ 1040 1999 Return
	L14		{Wages}
	L15a		{Interest}
	L16		{Dividends}
	L18		{Capital Gains}
	S1		{Property Tax}
	L42		{Witheld tax, from W-2}
*/


 /* Accept Form's "Title" line, and put out with date-stamp for records. */
 read_line( infile, word );
 now = time(0);
 fprintf(outfile,"\n%s		%s\n", word, ctime( &now ));

 /* get_parameter(infile, kind, x, mesage ) */
 get_parameter( infile, 's', word, "Status" );
 get_parameter( infile, 'l', word, "Status ?");
 if (strncasecmp(word,"Single",4)==0) { status = SINGLE; L["Single"] = 1; } else
 if (strncasecmp(word,"Married/Joint",13)==0) { status = MARRIED_FILLING_JOINTLY; L["MFJ"] = 1; } else
 if (strncasecmp(word,"Married/Sep",11)==0) { status = MARRIED_FILLING_SEPARAT; L["MFS"] = 1; } else
 if (strncasecmp(word,"Head_of_House",4)==0) { status = HEAD_OF_HOUSEHOLD; L["HOH"] = 1; } else
 if (strncasecmp(word,"Widow",4)==0) { status = WIDOW; L["QW"] = 1; }
 else
  { 
   printf("Error: unrecognized status '%s'. Must be: Single, Married/joint, Married/sep, Head_of_house, Widow(er)\nExiting.\n", word); 
   fprintf(outfile,"Error: unrecognized status '%s'. Must be: Single, Married/joint, Married/sep, Head_of_house, Widow(er)\nExiting.\n", word); 
   exit(1); 
  }
 switch (status)
 {
  case SINGLE: 			fprintf(outfile,"Status = Single (%d)\n", status); break;
  case MARRIED_FILLING_JOINTLY: fprintf(outfile,"Status = Married/Joint (%d)\n", status); break;
  case MARRIED_FILLING_SEPARAT: fprintf(outfile,"Status = Married/Sep (%d)\n", status); break;
  case HEAD_OF_HOUSEHOLD: 	fprintf(outfile,"Status = Head_of_Household (%d)\n", status); break;
  case WIDOW: 		  	fprintf(outfile,"Status = Widow(er) (%d)\n", status); break;
 }

 get_parameter( infile, 's', word, "L6" );	/* Exemptions, self/spouse. */
 get_parameter( infile, 'i', &j, "L6"); 
 L[6] = j;
 shownum(6); 

 get_parameter( infile, 's', word, "L7" );	/* Exemptions, Over 65. */
 get_parameter( infile, 'i', &j, "L7"); 
 L[7] = j;
 shownum(7); 

 get_parameter( infile, 's', word, "L8" );	/* Exemptions, Blind/disabled. */
 get_parameter( infile, 'i', &j, "L8"); 
 L[8] = j;
 shownum(8); 

 get_parameter( infile, 's', word, "L9" );	/* Exemptions, children. */
 get_parameter( infile, 'i', &j, "L9"); 
 L[9] = j;
 shownum(9); 

 get_parameter( infile, 's', word, "L10" );	/* Exemptions, other dependents. */
 get_parameter( infile, 'i', &j, "L10"); 
 L[10] = j;
 shownum(10); 

 get_parameter( infile, 's', word, "L11" );	/* Exemptions, college kids. */
 get_parameter( infile, 'i', &j, "L11"); 
 L[11] = j;
 shownum(11); 

 L12a = L[6] + L[7] + L[8] + L[11];
 L["12a"] = L12a;
 fprintf(outfile,"L12a = %d\n", L12a);
 L12b = L[9] + L[10];
 L["12b"] = L12b;
 fprintf(outfile,"L12b = %d\n", L12b);

 GetLineF( "L14", &L[14] );	/* Wages. */

 GetLineF( "L15a", &L[15] );	/* Taxable Interest. */

 /* Form asks for tax-exempt income, but does not use it. */

 GetLineF( "L16", &L[16] );	/* Dividends. */

 GetLine( "L17", &L[17] );	/* Business profits, Fed Sched C. */
 if (L[17] < 0.0) L[17] = 0.0;
 showline(17);

 GetLine( "L18", &L[18] );	/* Capital Gains . */
 if (L[18] < 0.0) L[18] = 0.0;
 showline(18);

 GetLineF( "L19", &L[19] );	/* Pensions, Annuities, and IRA Withdrawals (pg 20). */

 GetLineF( "L20", &L[20] );	/* Partnership income. (See pg 24.) */

 GetLineF( "L21", &L[21] );	/* S Corporation income. (See pg 24.) */

 GetLineF( "L22", &L[22] );	/* Rent, royalty, patents income. (Sched NJ-BUS-1, Part IV, Line 4.) */

 GetLineF( "L23", &L[23] );	/* Net gambling winnings. */

 GetLineF( "L24", &L[24] );	/* Alimony and maintenance payments RECEIVED. */

 GetLineF( "L25", &L[25] );	/* Other (See pg 24). */

 L[26] = L[14] + L[15] + L[16] + L[17] + L[18] + L[19] + L[20] + L[21] + L[22] + L[23] + L[24] + L[25];
 showline_wmsg(26,"Total Income");	/* Total Income. */

 GetLineF( "L27a", &L27a );	/* Pension Exclusion (See pg 26). */
 GetLineF( "L27b", &L27b );	/* Other Retirement Income Exclusion (See worksheet pg 26). */

 L["27a"] = L27a;
 L["27b"] = L27b;
 L[27] = L27a + L27b;
 L["27c"] = L[27];
 showline(27);

 L[28] = L[26] - L[27];
 showline_wmsg(28,"NJ Gross Income");

 if ((status == SINGLE) || (status == MARRIED_FILLING_SEPARAT))
  { if (L[28] < 10000.0)
     fprintf(outfile," --- You do not need to file, (except to get refund).  Income < $10,000. ---\n");
  }
 else
  { if (L[28] < 20000.0) 
     fprintf(outfile," --- You do not need to file, (except to get refund).  Income < $20,000. ---\n");
  }

 L29a = L12a * 1000.0;
 fprintf(outfile," L29a = %6.2f\n", L29a );
 L29b = L12b * 1500.0;
 fprintf(outfile," L29b = %6.2f\n", L29b );
 L[29] = L29a + L29b;
 fprintf(outfile,"L29c = %6.2f	Total Exemption Amount\n", L[29] );

 fprintf(outfile,"\n");
 GetLine( "E1", &E[1] );	/* Medical Expenses (See pg 27). */
 showline_wrksht('E',1,E);
 E[2] = 0.02 * L[28];
 showline_wrksht('E',2,E);
 E[3] = NotLessThanZero( E[1] - E[2] );
 showline_wrksht('E',3,E);
 GetLine( "E4", &E[4] );      /* Qualified Archer MSA contributions from Federal Form 8853 */
 showline_wrksht('E',4,E);
 GetLine( "E5", &E[5] );      /* Amount of self-employed health insurance deduction */
 showline_wrksht('E',5,E);
 E[6] = NotLessThanZero( E[3] + E[4] + E[5] );
 showline_wrksht('E',6,E);
 fprintf(outfile,"\n");
 L[30] = E[6];
 if (L[30] != 0.0)
  showline_wmsg(30," Medical Expenses Worksheet E (See pg 27)");
 /* end of Worksheet E */

 GetLineF( "L31", &L[31] );	/* Alimony and maintenance payments PAYED. */

 GetLineF( "L32", &L[32] );	/* Qualified Conservation Contribution. */

 GetLineF( "L33", &L[33] );	/* Health Enterprise Zone Deduction. */

 GetLineF( "L34", &L[34] );	/* Alternative Business Calc Adj (Sched NJ-BUS-2, Line 11). */

 L[35] = L[29] + L[30] + L[31] + L[32] + L[33] + L[34];
 showline_wmsg(35,"Total Exemptions and Deductions");

 /* Taxable income. */
 L[36] = L[28] - L[35];
 if (L[36] > 0.0)
  showline_wmsg(36, "(Taxable Income)");

 GetLineF( "L37a", &L37a );	/* Property Tax Paid. */
 L[37] = L37a;

 GetLine( "A1", &A[1] );	/* Income taxed by other jurisdictions, if any. */
 GetLine( "A9a", &A9a );	/* Tax paid to other jurisdictions on that income, if any. */

 fprintf(outfile,"\n");  /* Tax deduction worksheet F (pg 34). */
 F[1] = L37a;
 showline_wrksht('F',1,F);
 if (status != MARRIED_FILLING_SEPARAT)
  F[2] = smallerof( F[1], 10000.0 );
 else
  F[2] = smallerof( F[1],  5000.0 );
 showline_wrksht('F',2,F);

 if (status != MARRIED_FILLING_SEPARAT)
  proptxcredit = 50.0;
 else
  proptxcredit = 25.0;

 if (A9a == 0.0)
  { /*Worksheet-F*/
    F[3] = L[36];	 Fb[3] = L[36];
    fprintf(outfile," F3a = %6.2f	F3b = %6.2f\n", F[3], Fb[3]);
    F[4] = F[2];	 Fb[4] = 0.0;
    fprintf(outfile," F4a = %6.2f	F4b = %6.2f\n", F[4], Fb[4]);
    F[5] = F[3] - F[4];  Fb[5] = Fb[3] - Fb[4];
    fprintf(outfile," F5a = %6.2f	F5b = %6.2f\n", F[5], Fb[5]);
    F[6] = TaxRateFunction( F[5], status );
    Fb[6] = TaxRateFunction( Fb[5], status );
    fprintf(outfile," F6a = %6.2f	F6b = %6.2f\n", F[6], Fb[6]);
    F[7] = Fb[6] - F[6];
    showline_wrksht('F',7,F);
    if (F[7] >= proptxcredit)
     { /*yes*/
       fprintf(outfile," F8. Yes. (Take Property Tax Deduction.)\n");
       L[38] = F[4];
       L[39] = F[5];
       L[40] = F[6];
       L[49] = 0.0;
     } /*yes*/
    else
     { /*no*/
       fprintf(outfile," F8. No. (Take Property Tax Credit.)\n");
       L[38] = 0.0;
       L[39] = Fb[5];
       L[40] = Fb[6];
       L[49] = proptxcredit;
     } /*no*/
  } /*Worksheet-F*/
 else
  { /*SchedA+Worksheet-I*/
    fprintf(outfile,"\nSchedule A:\n");
    fprintf(outfile," %c%d = %6.2f\n", 'A', 1, A[1]);

    A[2] = L[28];
    fprintf(outfile," %c%d = %6.2f\n", 'A', 2, A[2]);

    A[3] = smallerof( 1.0, (A[1] / A[2]) );
    fprintf(outfile," A3 = %6.2f %%\n", 100.0 * A[3] );
    A["3p"] = 100.0*A[3];
    A[4] = L[36];
    A["4a"] = L[36];
    A["4b"] = L[36];
    fprintf(outfile," A4a = %6.2f	A4b = %6.2f\n", A[4], A[4] );
    fprintf(outfile," (5a = %6.2f)\n", F[1] );
    A["5a"] = F[1];
    A[5] = F[2];
    A["5aa"] = F[2];
    fprintf(outfile," A5a = %6.2f	A5b = %6.2f\n", A[5], 0.0);
    A[6]  = A[4] - A[5];
    A["6a"] = A[6];
    Ab[6] = A[4] - 0.0;
    A["6b"] = Ab[6];
    fprintf(outfile," A6a = %6.2f	A6b = %6.2f\n", A[6], Ab[6]);
    A[7]  = TaxRateFunction( A[6], status );
    A["7a"] = A[7];
    Ab[7] = TaxRateFunction( Ab[6], status );
    A["7b"] = Ab[7];
    fprintf(outfile," A7a = %6.2f	A7b = %6.2f\n", A[7], Ab[7] );
    A[8]  = A[3] * A[7];
    A["8a"] = A[8];
    Ab[8] = A[3] * Ab[7];
    A["8b"] = Ab[8];
    fprintf(outfile," A8a = %6.2f	A8b = %6.2f\n", A[8], Ab[8] );
    fprintf(outfile,"  (9a = %6.2f)\n", A9a );
    A["9a"] = A9a;
    A[9] = smallerof( smallerof( A9a, A[8] ), A[7] );
    A["9aa"] = A[8];
    Ab[9] = smallerof( smallerof( A9a, Ab[8] ), Ab[7] );
    A["9ab"] = Ab[9];
    fprintf(outfile," A9a = %6.2f	A9b = %6.2f\n", A[9], Ab[9] );

    fprintf(outfile,"\nWorksheet I:\n");
    I[1] = A[7];	Ib[1] = Ab[7];
    fprintf(outfile," I1a = %6.2f	I1b = %6.2f\n", I[1], Ib[1] );
    I[2] = A[9];	Ib[2] = Ab[9];
    fprintf(outfile," I2a = %6.2f	I2b = %6.2f\n", I[2], Ib[2] );

    I[3]  = I[1] - I[2];
    Ib[3] = Ib[1] - Ib[2];
    fprintf(outfile," I3a = %6.2f	I3b = %6.2f\n", I[3], Ib[3] );

    Ib[4] = Ib[3] - I[3];
    showline_wrksht('I', 4, Ib);

    if (Ib[4] >= proptxcredit)
     {
      fprintf(outfile," Sched-I, Yes:  Take PropTax Deduction\n\n");
      L[38] = A[5];	// fprintf(outfile,"L36c = %6.2f\n", L[36]);
      L[39] = A[6];
      L[40] = A[7];
      L[41] = I[2];
      L[49] = 0.0;
     }
    else
     {
      fprintf(outfile," Sched-I, No:  Take PropTax Credit\n\n");
      L[38] = 0.0;
      L[39] = Ab[6];
      L[40] = Ab[7];
      L[41] = Ib[2];
      L[49] = proptxcredit;
     }
  } /*SchedA+Worksheet-I*/


 /* If no property tax was paid, ensure prop.tax credit is set to 0 */
 if ( L37a == 0.0 ) L[49] = 0.0;

 if (L[37] > 0.0)
  fprintf(outfile, "L37c = %6.2f\n", L[37]);

 showline(39);

 fprintf(outfile,"\n");  /* NJ Taxable Income.*/
 // L[39] = L[36] - L[38];  /* Handled above in Sched-1. */
 if (L[39] > 0.0)
  showline_wmsg(39, "NJ Taxable Income");

 // L[40] = TaxRateFunction( L[39], status );  /* Handled above in Schedules+Worksheets, A, F, H. */
 showline_wmsg(40, "TAX");

 if (A[1] > 0.0)
  showline_wmsg(41, "( Credit for Taxes paid to other jurisdictions. )\n");

 L[42] = L[40] - L[41];
 showline_wmsg(42, "( Balance of Tax )");

 GetLineF( "L43", &L[43] );	/* Sheltered Workshop Tax Credit. */
 L[44] = L[42] - L[43];
 showline(44);

 GetLineF( "L45", &L[45] );	/* Use Tax Due on Out-of-State Purchases (pg 35). */
 GetLineF( "L46", &L[46] );	/* Penalty for underpayment of estimated tax. */

 L[47] = L[44] + L[45] + L[46];
 showline(47);			/* Total Tax + Penalty. */

 GetLine( "L48", &L[48] );
 showline_wmsg(48, "Total NJ Income Tax Withheld");

 showline_wmsg(49, "Property tax Credit");

 GetLineF( "L50", &L[50] );	/* NJ Estimated Tax Payments/Credit from last year's return. */

 GetLineF( "L51", &L[51] );	/* NJ Earned Income Tax Credit. (See Sched pg 38.) */

 GetLineF( "L52", &L[52] );	/* EXCESS NJ UI/HC/WD Withheld, (See pg 38.) */

 GetLineF( "L53", &L[53] );	/* EXCESS NJ Disability Insurance Withheld, (See pg 38.) */

 GetLineF( "L54", &L[54] );	/* EXCESS NJ Family Leave Insurance Withheld, (See pg 38.) */

 L[55] = L[48] + L[49] + L[50] + L[51] + L[52] + L[53] + L[54];
 showline_wmsg(55,"Total Payments/Credits");
 
 if (L[55] < L[47])
  {
   L[56] = L[47] - L[55];
   fprintf(outfile, "L56 = %6.2f	YOU OWE !!!\n", L[56] );
  }
 else
  {
   L[57] = L[55] - L[47];
   fprintf(outfile, "L57 = %6.2f	Overpayment\n", L[57] );

   L[65] = 0.0;
   showline_wmsg(65, "( Total Contributions from overpayment )");
   L[66] = L[57] - L[65];
   showline_wmsg(66, "Refund !!!");
  }
 
 fclose(infile);
 fclose(outfile);

 outfile = fopen(nj1040_outfname,"w");
 output_xfdf_form_data(outfile, nj1040_2014, L); 
 fclose(outfile);

 outfile = fopen(nj1040_sched_a_outfname,"w");
 output_xfdf_form_data(outfile, nj_schedule_a_2014, A); 
 fclose(outfile);

 Display_File( outfname );
 printf("\nResults written to file:  %s\n", outfname);
 return 0;
}
