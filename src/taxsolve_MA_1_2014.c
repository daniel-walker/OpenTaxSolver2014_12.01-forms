/************************************************************************/
/* taxsolve_ma_1_2014.c - OpenTaxSolver for Mass Form 1 V12.0		*/
/* Copyright (C) 2014 - Robert Heller					*/
/* 									*/
/* OTS Project Home Page and Updates:  					*/
/*		http://opentaxsolver.sourceforge.com/			*/
/* 									*/
/* Compile:   cc taxsolve_ma_1_2014.c -o taxsolve_ma_1_2014		*/
/* Run:       ./taxsolve_ma_1_2014  Mass1_2006.txt			*/
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
/* Robert Heller 2-10-2004	heller@deepsoft.com			*/
/* Updated 1-26-05	Aston Roberts & Robert Heller			*/
/*  ...									*/
/* Updated 1-3-15	Aston Roberts 					*/
/************************************************************************/

#include <stdio.h>
#include <time.h>

float thisversion=12.0;

#include "taxsolve_routines.c"

#define SINGLE 		        1
#define MARRIED_FILLING_JOINTLY 2
#define MARRIED_FILLING_SEPARAT 3
#define HEAD_OF_HOUSEHOLD       4
#define WIDOW		        5
#define Yes 1
#define No  0

double SumL( Lmap &v, int start_slot, int end_slot )
{
 int j;
 double result = 0.0;
 for (j=start_slot; j <= end_slot; j++) result += v[j];
 return result;
}

double Sum( double *v, int start_slot, int end_slot )
{
 int j;
 double result = 0.0;
 for (j=start_slot; j <= end_slot; j++) result += v[j];
 return result;
}


double ComputeTax(double taxableIncome)
{
 if (taxableIncome < 24000.0)
  return (int)(0.052 * (taxableIncome + 25.0) + 0.5);
 else
  return taxableIncome * 0.052;
}



/*----------------------------------------------------------------------------*/
/* ---				Main					  --- */
/*----------------------------------------------------------------------------*/
int main( int argc, char *argv[] )
{
 int i, j, k, status=0, i65, iblind, ndep, dep_deduct;
 int flag, notaxstatus=0;
 char word[2000], outfname[2000];
 time_t now;
 double Exemptions[5], Exempt_MD, Exempt_Adoption;
 double MassBankInterest, Iexempt, AGI;
 double Unemployment, Lottery;
 double MassRetirement[2];
 double L23a=0.0, L32[6], L34a=0.0, L34b=0.0;
 double L40b=0.0;
 
 printf("Massachusetts Form-1 2014 - v%3.2f\n", thisversion);
 
 /* Decode any command-line arguments. */
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
    j = strlen(outfname)-1;
    while ((j>=0) && (outfname[j]!='.')) j--;
    if (j<0) strcat(outfname,"_out.txt"); else strcpy(&(outfname[j]),"_out.txt");
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
 for (i=0; i<MAX_LINES; i++) { L[i] = 0.0; }

 /* Accept parameters from input file. */
 /* Expect  Mass. Form-1 lines, something like:
	Title:  Mass Form 1 Return
	L2		{Exemptions}
	L3		{Wages}
 */

 /* Accept Form's "Title" line, and put out with date-stamp for your records. */
 read_line( infile, word );
 now = time(0);
 fprintf(outfile,"\n%s 	%s\n", word, ctime( &now ));

 /* Get status as:  Single, Married/joint, Head house, Married/sep. */
 get_parameter( infile, 's', word, "Status" );
 get_parameter( infile, 'l', word, "Status?");
 if (strncasecmp(word,"Single",4)==0) status = SINGLE; else
 if (strncasecmp(word,"Married/Joint",13)==0) status = MARRIED_FILLING_JOINTLY; else
 if (strncasecmp(word,"Married/Sep",11)==0) status = MARRIED_FILLING_SEPARAT; else
 if (strncasecmp(word,"Head_of_House",4)==0) status = HEAD_OF_HOUSEHOLD; else
 if (strncasecmp(word,"Widow",4)==0) status = WIDOW;
 else 
  { 
   printf("Error: unrecognized status '%s'. Exiting.\n", word); 
   fprintf(outfile,"Error: unrecognized status '%s'. Exiting.\n", word); 
   exit(1);
  }
 fprintf(outfile,"Status = %s (%d)\n", word, status);

 for (i=0; i<4; i++) {Exemptions[i] = 0.0;}

 switch (status)
  {
   case SINGLE: 
   case MARRIED_FILLING_SEPARAT:
	Exemptions[0] = 4400.0;
   	break;
   case HEAD_OF_HOUSEHOLD:
	Exemptions[0] = 6800.0;
	break;
   case MARRIED_FILLING_JOINTLY: 
	Exemptions[0] = 8800.0;
	break;
  }   
 
 fprintf(outfile,"L2. Exemptions: \n");
 fprintf(outfile,"  2a. Personal exemptions = %6.2f\n", Exemptions[0]);

 get_parameter( infile, 's', word, "Dependents" );
 get_parameter( infile, 'i', &ndep, "Dependents"); 
 Exemptions[1] = (double)ndep * 1000.0;
 if (Exemptions[1] > 0.0)
  fprintf(outfile,"  2b. Number of dependents: %d x 1,000 = %6.2f\n", ndep, Exemptions[1]);

 i65 = 0; iblind = 0;
 Exempt_MD = 0.0; Exempt_Adoption=0.0;
 get_parameter( infile, 's', word, "Age65You");
 get_parameter( infile, 'b', &flag, "Your age over 65?");
 if (flag) i65++;
 get_parameter( infile, 's', word, "Age65Spouse");
 get_parameter( infile, 'b', &flag, "Spouse age over 65?");
 if (flag) i65++;
 Exemptions[2] = (double)i65 * 700.0;
 if (Exemptions[2] > 0)
  fprintf(outfile,"  2c. Age 65 or over: %d x 700        = %6.2f\n", i65, Exemptions[2]);

 get_parameter( infile, 's', word, "BlindYou");
 get_parameter( infile, 'b', &flag, "Your Blindness?");
 if (flag) iblind++;
 get_parameter( infile, 's', word, "BlindSpouse");
 get_parameter( infile, 'b', &flag, "Spouse Blindness?");
 if (flag) iblind++;
 Exemptions[3] = (double)iblind * 2200.0;
 if (Exemptions[3] > 0)
  fprintf(outfile,"  2d. Blindness: %d x 2,200        = %6.2f\n", iblind, Exemptions[3]);

 /* get_parameter( infile, 's', word, "Med/Dental");
 get_parameter( infile, 'f', &Exempt_MD, "Med/Dental?"); */
 GetLine( "Med/Dental", &Exempt_MD );
 /* get_parameter( infile, 's', word, "Adoption");
 get_parameter( infile, 'f', &Exempt_Adoption, "Adoption?"); */
 GetLine( "Adoption", &Exempt_Adoption );
 Exemptions[4] = Exempt_MD + Exempt_Adoption;
 if (Exemptions[4] > 0) fprintf(outfile,"  2e. Other: 1. Medical/Dental > %6.2f + 2. Adoption > %6.2f = %6.2f\n", Exempt_MD, Exempt_Adoption, Exemptions[4]);

 L[2] = Sum( Exemptions, 0, 4 );
 fprintf(outfile,"  2f. Total Exemptions = %6.2f\n", L[2]);

 GetLine( "L3", &L[3] );	/* Wages, salery, tips (W-2). */
 showline(3);

 GetLine( "L4", &L[4] );	/* Taxable pensions. */
 ShowLineNonZero(4);

 GetLine( "L5", &MassBankInterest );
 if (status == MARRIED_FILLING_JOINTLY)
  Iexempt = 200;
 else
  Iexempt = 100;
 L[5] = MassBankInterest - Iexempt;
 if (L[5] < 0.0) L[5] = 0.0;
 if (L[5] > 0.0)
  {
   sprintf(word,"Mass. Bank Interest: a. %6.2f - b. exemption %6.2f",
	MassBankInterest,Iexempt);
   showline_wmsg( 5, word );
  }

 GetLine( "L6", &L[6] );	/* Business income/loss. */
 ShowLineNonZero(6);

 GetLine( "L7", &L[7] );	/* Rental, royality, REMIC. */
 ShowLineNonZero(7);

 GetLine( "L8a", &Unemployment );	/* Unemployment */
 GetLine( "L8b", &Lottery );	/* Lottery */
 L[8] = Unemployment + Lottery;
 if (L[8] > 0)
  {
   sprintf(word,"a. %6.2f + b. %6.2f",Unemployment,Lottery);
   showline_wmsg( 8, word );
  }

 GetLine( "L9", &L[9] );	/* Other Income, Alimony received. */
 ShowLineNonZero(9);

 L[10] = SumL( L, 3, 9 );
 showline_wmsg( 10, "TOTAL 5.2% INCOME" );
 
 /* Amount paid to SS, Medicare, RR, US, or Mass retirement */
 GetLine( "L11a", &MassRetirement[0] ); /* You */
 if (MassRetirement[0] > 2000) MassRetirement[0] = 2000;
 GetLine( "L11b", &MassRetirement[1] ); /* Spouse */
 if (MassRetirement[1] > 2000) MassRetirement[1] = 2000;
 L[11] = Sum(MassRetirement, 0, 1);
 if (L[11] > 0)
  {
   sprintf(word,"you %6.2f + spouse %6.2f", MassRetirement[0], MassRetirement[1]);
   showline_wmsg(11,word);
  }

 GetLine( "L12", &L[12] );	/* Child under 13... */
 ShowLineNonZero(12);

 get_parameter( infile, 's', word, "L13"); /* Dependent under 12. */
 get_parameter( infile, 'i', &dep_deduct, "L13");
 if (dep_deduct > 2) dep_deduct = 2;
 if ((L[12] == 0) && ((status == MARRIED_FILLING_JOINTLY) || (status == HEAD_OF_HOUSEHOLD))
     && (dep_deduct > 0))
  {
   L[13] = dep_deduct * 3600.0;
   sprintf(word,"a. %d x 3,600 ", dep_deduct);
   showline_wmsg(13, word);
  }

 GetLine( "L14a", &L[14] );	/* Rental Paid */
 L[14] = L[14] / 2.0;
 if (status == MARRIED_FILLING_SEPARAT)
  L[14] = smallerof( L[14] , 1500.0 );
 else
  L[14] = smallerof( L[14] , 3000.0 );
 ShowLineNonZero(14);

 GetLine( "L15", &L[15] );	/* Other Deductions (sched Y, L17) */
 ShowLineNonZero(15);

 L[16] = SumL( L, 11, 15 );
 showline_wmsg(16,"Total Deductions");

 L[17] = NotLessThanZero( L[10] - L[16] );
 showline(17);

 L[18] = L[2];
 showline(18);

 L[19] = NotLessThanZero( L[17] - L[18] );
 showline(19);

 GetLine( "L20", &L[20] );	/* Interest and Dividends -- Sched B */
 L[20] = NotLessThanZero( L[20] );
 showline(20);

 L[21] = L[19] + L[20];
 showline_wmsg(21, "Total 5.2% Taxable Income");

 L[22] = ComputeTax( L[21] );
 showline_wmsg(22,"5.2% Tax");

 GetLine( "L23a", &L23a ); 	/* 12% income */
 L[23] = NotLessThanZero( L23a * 0.12 );
 if (L23a > 0.0)
  {
   sprintf(word,"12%% Income tax: a. %6.2f x 0.12", L23a);
   showline_wmsg(23, word);
  }

 GetLine( "L24", &L[24] ); 	/* Tax on long-term capital gains, sched D */
 ShowLineNonZero(24);

 GetLine( "L25", &L[25] ); 	/* Credit Capture amount Sch. H-2 */
 ShowLineNonZero(25);

 GetLine( "L26", &L[26] ); 	/* Additional tax on installment sale */
 ShowLineNonZero(26);

 L[28] = SumL( L, 22, 26 );

 { /* AGI Worksheet pg 12+13. */
   double ws[20], threshA, threshB;
   for (j=0; j<20; j++) ws[j] = 0.0;
   ws[1] = NotLessThanZero( L[10] );
   ws[2] = 0.0;		/* Sched Y lines 1-10.  Assumed zero, adjust otherwise. */
   ws[3] = NotLessThanZero( ws[1] - ws[2] );
   ws[4] = smallerof( MassBankInterest, Iexempt );
   if (L[10] < 0.0) ws[4] = NotLessThanZero( L[10] + ws[4] );
   ws[5] = L[20];	/* Or Schedule B, line 35. */
   ws[6] = 0.0;		/* Sched D, line 19.  Assumed zero, adjust otherwise. */
   ws[7] = Sum( ws, 1, 6 );
   AGI = ws[7];
   if (status != MARRIED_FILLING_SEPARAT)
    { /*not_sep*/
     switch (status)
      {
       case SINGLE:  
		threshA = 8000.0;
		threshB = 14000.0;
		break;
       case HEAD_OF_HOUSEHOLD:
		threshA = 14400.0 + 1000.0 * ndep;
		threshB = 25200.0 + 1750.0 * ndep;
		break;
       case MARRIED_FILLING_JOINTLY: 
		threshA = 16400.0 + 1000.0 * ndep;
		threshB = 28700.0 + 1750.0 * ndep;
		break;
	default: printf("Bad filing status.\n");  exit(1); break;
      }
     if (AGI <= threshA)
      {
	notaxstatus = 1;
	fprintf(outfile,"You qualify for No Tax Status.\n");
      } else
     if (AGI <= threshB)
      fprintf(outfile,"See Form 1 Line 29 special instructions for Limited Income.\n");
    } /*not_sep*/
}

 if (notaxstatus) L[28] = 0.0;
 showline_wmsg(28, "Total Tax");

 GetLine1( "L29", &L[29] ); 	/* Limited Income Credit */
 GetLine1( "L30", &L[30] ); 	/* Other credits from Sch Z, line 14 */
 if (notaxstatus) { L[29] = 0.0;  L[3] = 0.0; }
 ShowLineNonZero(29);
 ShowLineNonZero(30);

 L[31] = NotLessThanZero( L[28] - (L[29] + L[30]) );
 showline_wmsg(31,"Income Tax After Credits");

 GetLine1( "L32a", &L32[0] ); /* Endangered Wildlife */
 if (L32[0] != 0) showline_wlabel( "L32a", L32[0] );
 GetLine1( "L32b", &L32[1] ); /* Organ Transplant */
 if (L32[1] != 0) showline_wlabel( "L32b", L32[1] );
 GetLine1( "L32c", &L32[2] ); /* Mass AIDS */
 if (L32[2] != 0) showline_wlabel( "L32c", L32[2] );
 GetLine1( "L32d", &L32[3] ); /* Mass US Olympic */
 if (L32[3] != 0) showline_wlabel( "L32d", L32[3] );
 GetLine1( "L32e", &L32[4] ); /* Mass Military Family Relief */
 if (L32[4] != 0) showline_wlabel( "L32e", L32[4] );
 GetLine1( "L32f", &L32[5] ); /* Homeless Animal Prevention And Care */
 if (L32[5] != 0) showline_wlabel( "L32f", L32[5] );
 L[32] = Sum( L32, 0, 5 );
 ShowLineNonZero( 32 );

 GetLine1( "L33", &L[33] ); 	/* Use tax due on out-of-state purchases */
 showline(33);

 GetLine1( "L34a", &L34a ); 	/* Health Care Penalty (you) */
 GetLine1( "L34b", &L34b ); 	/* Health Care Penalty (spouse) */
 L[34] = L34a + L34b;
 if (L[34] != 0)
  fprintf(outfile,"L34. Health Care penalty:  a. You: %6.2f,  b. Spouse: %6.2f,  L34 = %6.2f\n",
	L34a, L34b, L[34] );

 L[35] = SumL( L, 31, 34 );
 showline_wmsg(35,"Income Tax After Credits Contributions, Use Tax + HC Penalty");

 
 /* Payments section. */

 GetLine( "L36", &L[36] );	/* Mass income tax withheld, Forms W-2, 1099 */
 ShowLineNonZero(36);

 GetLine( "L37", &L[37] );	/* Last year's overpayment you want applied to 2014 estimated tax */
 ShowLineNonZero(37);

 GetLine( "L38", &L[38] );	/* 2014 estimated tax payments */
 ShowLineNonZero(38);

 GetLine( "L39", &L[39] );	/* Payments made with extension */
 ShowLineNonZero(39);

 GetLine( "L40b", &L40b );	/* Earned income credit (EIC) */
 if (L40b != 0.0) fprintf(outfile, " L40b = %6.2f  x 0.15 = .....  ", L40b );
 L[40] = L40b * 0.15;
 ShowLineNonZero(40);

 GetLine( "L41", &L[41] );	/* Senior Circuit Breaker Credit, sched CB */
 ShowLineNonZero(41);

 GetLine( "L42", &L[42] );	/* Refundable credits, Sched RF, line 4. */
 ShowLineNonZero(42);

 L[43] = SumL( L, 36, 42 );
 showline_wmsg(43,"total payments");

 /* Refund or Owe section. */
 if (L[35] < L[43]) 
  {
   L[44] = L[43] - L[35];
   fprintf(outfile,"L44 = %6.2f  Overpayment!\n", L[44] );
   GetLine1( "L45", &L[45] );	/* Overpayment to be applied to next year's estimated tax */
   if (L[45] > L[44])
    L[45] = L[44];
   showline_wmsg(45,"Overpayment to be applied to next year's estimated tax");
   L[46] = L[44] - L[45];
   fprintf(outfile,"L46 = %6.2f  THIS IS YOUR REFUND\n", L[46] );
  }
 else 
  {
   L[47] = L[35] - L[43];
   fprintf(outfile,"L47 = %6.2f  TAX DUE !!!\n", L[47] );
   if ((L[47] > 400.0) && (L[43] < 0.80 * L[35]))
    fprintf(outfile," You may owe Underpayment of Estimated Tax penalty.\n");
  }

 fclose(infile);
 fclose(outfile);

 printf("\nListing results from file: %s\n\n", outfname);
 Display_File( outfname );
 printf("\nResults writen to file: %s\n", outfname);

 return 0;
}
