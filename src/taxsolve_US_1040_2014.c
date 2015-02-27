/************************************************************************/
/* TaxSolve_usa_fed1040_2014.c - OpenTaxSolver for USFed1040 v11.01	*/
/* Copyright (C) 2015 - Aston Roberts					*/
/* 									*/
/* Tax Solver for US Fedral 1040 Income Tax return for 2014 Tax Year.	*/
/* 									*/
/* OTS Project Home Page and Updates:  					*/
/*		http://opentaxsolver.sourceforge.com/			*/
/* 									*/
/* Compile:   cc taxsolve_US_1040_2014.c -o taxsolve_US_1040_2014       */
/* Run:       ./taxsolve_US_1040_2014  Fed1040_2014.txt                 */
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

float thisversion=12.01;

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "taxsolve_routines.c"
#include "taxsolve_US_1040_2014_form.h"

#define SINGLE 		        1
#define MARRIED_FILLING_JOINTLY 2
#define MARRIED_FILLING_SEPARAT 3
#define HEAD_OF_HOUSEHOLD       4
#define WIDOW		        5
#define Yes 1
#define No  0

Lmap SchedA, SchedD;
double L9b=0.0;			 /* Qualified dividends. */
double qcgws6=0.0, qcgws7=0.0;	 /* Support for AMT calculation. (qual.div+cap.gain wrksht vals.)*/
int Do_SchedD=No, Do_QDCGTW=No, Do_SDTW=No, output_SchedD=No;
int status, under65=Yes, over65=No, dependent=No;
double  L60b=0.0, collectibles_gains=0.0, ws_sched_D[MAX_LINES], idws_thresh=0.0, idws[MAX_LINES];


double TaxRateFormula( double x, int status )  /* Returns tax due. */
{			/* Values taken from 1040 Instructions. */			/* Updated for 2014. */
  double brkpt[4][9]={
		  { 0.0,  9075.0, 36900.0,  89350.0, 186350.0, 405100.0, 406750.0, 9e9 }, /* Single */
		  { 0.0, 18150.0, 73800.0, 148850.0, 226850.0, 405100.0, 457600.0, 9e9 }, /* Married, filing jointly. */
		  { 0.0,  9075.0, 36900.0,  74425.0, 113425.0, 202550.0, 228800.0, 9e9 }, /* Married, filing separate. */
		  { 0.0, 12950.0, 49400.0, 127550.0, 206600.0, 405100.0, 432200.0, 9e9 }  /* Head of Household. */
		     };
  double txrt[4][9] ={
		  { 0.1,   0.15,   0.25,    0.28,    0.33,    0.35,    39.6 },	/* Single */
		  { 0.1,   0.15,   0.25,    0.28,    0.33,    0.35,    39.6 },	/* Married, filing jointly. */
		  { 0.1,   0.15,   0.25,    0.28,    0.33,    0.35,    39.6 },	/* Married, filing separate. */
		  { 0.1,   0.15,   0.25,    0.28,    0.33,    0.35,    39.6 }	/* Head of Household. */
		     };
  double sum=0.0;
  int   bracket=0;
  
  if (status==WIDOW) status = MARRIED_FILLING_JOINTLY;  /* Handle case of widow(er). */
  status = status - 1;  /* Arrays start at zero; not one. */
  while (brkpt[status][bracket+1] < x)
   {
    sum = sum + (brkpt[status][bracket+1] - brkpt[status][bracket]) * txrt[status][bracket];
    bracket = bracket + 1;
   }

  return (x - brkpt[status][bracket]) * txrt[status][bracket] + sum;
}



double TaxRateFunction( double income, int status )     /* Emulates table lookup or function appropriately. */
{
 double x, dx, tx;
 int k;

 if (income < 100000.0)	/* Quantize to match tax-table exactly. */
  {
   if (income <= 25.0) x = 5.0; else
   if (income <= 3000.0) x = 25.0; else x = 50.0;
   dx = 0.5 * x;
   k = (income - 0.000001) / x;
   x = x * (double)k + dx;
   tx = (int)(TaxRateFormula( x, status ) + 0.5);
  }
 else
  tx = TaxRateFormula( income, status );
 return tx;
}



void showschedA( int linenum )
{ if (SchedA[linenum] > 0.0) fprintf(outfile," A%d = %6.2f\n", linenum, SchedA[linenum] ); }

void showschedA_wMsg( int linenum, char *msg )
{ if (SchedA[linenum] > 0.0) fprintf(outfile," A%d = %6.2f	%s\n", linenum, SchedA[linenum], msg ); }



/*-----------------------------------------------------------------*/
/* Qualified Dividends and Capital Gain Tax Worksheet for Line 44. */
/*  From page 43 of instructions.				   */
/*-----------------------------------------------------------------*/
void capgains_qualdividends_worksheets( int status, double L9b )		/* Updated for 2014. */
{
 double ws[50];
 int j;

 for (j=0; j<50; j++) ws[j] = 0.0;
 ws[1] = L[43];
 ws[2] = L9b;
 if (Do_SchedD)
  ws[3] = NotLessThanZero(smallerof( SchedD[15], SchedD[16] ));
 else
  ws[3] = L[13];
 ws[4] = ws[2] + ws[3];
 ws[5] = 0.0;  /* Investment interest expense, form 4952, usually 0. */
 ws[6] = NotLessThanZero( ws[4] - ws[5] );  	qcgws6 = ws[6];
 ws[7] = NotLessThanZero( ws[1] - ws[6] );	qcgws7 = ws[7];
 switch (status)
  {
   case SINGLE: case MARRIED_FILLING_SEPARAT: ws[8] = smallerof( ws[1], 36900.0 ); break;
   case MARRIED_FILLING_JOINTLY: case WIDOW:  ws[8] = smallerof( ws[1], 73800.0 ); break;
   case HEAD_OF_HOUSEHOLD: 		      ws[8] = smallerof( ws[1], 49400.0 ); break;
  }
 ws[9]  = smallerof( ws[1], ws[8] );
 ws[10] = smallerof( ws[7], ws[9] );
 ws[11] = ws[9] - ws[10];		/* This amount is taxed at 0%. */
 ws[12] = smallerof( ws[1], ws[6] );
 ws[13] = ws[11];
 ws[14] = ws[12] - ws[13];
 switch (status)
  {
   case SINGLE:  			      ws[15] = 406750.0;  break;
   case MARRIED_FILLING_SEPARAT:	      ws[15] = 228800.0;  break;
   case MARRIED_FILLING_JOINTLY: case WIDOW:  ws[15] = 457600.0;  break;
   case HEAD_OF_HOUSEHOLD: 		      ws[15] = 432200.0;  break;
  }
 ws[16] = smallerof( ws[1], ws[15] );
 ws[17] = ws[7] + ws[11];
 ws[18] = NotLessThanZero( ws[16] - ws[17] );
 ws[19] = smallerof( ws[14], ws[18] );
 ws[20] = 0.15 * ws[19];
 ws[21] = ws[11] + ws[19];
 ws[22] = ws[12] - ws[21];
 ws[23] = 0.20 * ws[22];
 ws[24] = TaxRateFunction( ws[7], status );
 ws[25] = ws[20] + ws[23] + ws[24];
 ws[26] = TaxRateFunction( ws[1], status );
 ws[27] = smallerof( ws[25], ws[26] );
 for (j = 1; j <= 27; j++)
  {
   printf("	Qual. Div & Gains WorkSheet %d:  %8.2f\n", j, ws[j] );
   if (j == 3) { if (Do_SchedD) fprintf(outfile,"\t\t3: Check Yes.\n"); else fprintf(outfile,"\t\t3: Check No.\n"); }
   fprintf(outfile,"	Qual. Div & Gains WorkSheet %d:  %8.2f\n", j, ws[j] );
  }
 L[44] = ws[27];
}




/*----------------------------------------------------------------------------------------------*/
/* Form-6251 - Alternative Minimum Tax (AMT) form detailed calculations. 			*/
/* This routine establishes the framework for the 6251 form, for the limited few who need it. 	*/
/* Form 6251 asks many highly specialized questions, which are assumed zero for most filers. 	*/
/* Those who should make the additional entries will no-doubt know who they are, and can 	*/
/* simply add them to this section.  The balance of the routine will be helpful in either case. */
/* --- Anyone indicated to fill-out Form 6251 should review the 6251 instruction booklet. ---	*/ 
/*----------------------------------------------------------------------------------------------*/
double form6251_AlternativeMinimumTax( int itemized )						/* Updated for 2014. */
{
 double amtws[100], ws[100], thresholdA=0, thresholdB=0, thresholdC=182500.0, amtexmption;
 double offsetA=3650.0, sum8_27=0.0;
 int j;

 printf("Review AMT form6251 routine for your situation.\n");
 fprintf(outfile,"Review AMT form6251 routine for your situation.\n");
 for (j=0; j<100; j++) { amtws[j] = 0.0;  ws[j] = 0.0; }
 /* Part I - Alternatiev Minimum Taxable Income (AMTI) */
 if (itemized)  
  { 
   amtws[1] = L[41];  
   amtws[2] = NotLessThanZero( smallerof( SchedA[4], ( 0.025 * L[38] ) ) );
   amtws[3] = SchedA[9];
   amtws[4] = 0.0;  /* Home mortgage interest adjustment. (Assuming zero, adjustotherwise.) */
   amtws[5] = SchedA[27];
   if (L[38] <= idws_thresh)
    amtws[6] = 0.0;
   else
    amtws[6] = -absolutev(idws[9]);  /* From Itemized Deductions WorkSheet. */
  }
 else
  amtws[1] = L[38];
 amtws[7] = -(L[10] + L[21]);		/* (As negaitive amount.) */
 amtws[8] = 0.0;	/* Investment interest expense. (Diff between regular tax and AMT). */
 amtws[9] = 0.0;	/* Depletion (Diff between regular tax and AMT). */
 amtws[10] = absolutev( L[21] );
	/* Following amounts assumed normally zero, but review and adjust if needed. */
 amtws[11] = -0.0; 	/* Alternative tax net operating loss deduction, as negative amount. */
 amtws[12] = 0.0;	/* Interest from specified private activity bonds exempt from the regular tax */
 amtws[13] = 0.0;	/* Qualified small business stock (7% of gain excluded under section 1202) */
 amtws[14] = 0.0;	/* Exercise incentive stock options (excess of AMT income over reg tax income) */
 amtws[15] = 0.0;	/* Estates and trusts (amount from Schedule K-1 (Form 1041), box 12, code A) */
 amtws[16] = 0.0;	/* Electing large partnerships (amount from Schedule K-1 (Form 1065-B), box 6) */
 amtws[17] = 0.0;	/* Disposition of property (difference between AMT and regular tax gain or loss) */
 amtws[18] = 0.0;	/* Deprec assets placed in service after 1986 (diff between regular tax and AMT) */
 amtws[19] = 0.0;	/* Passive activities (difference between AMT and regular tax income or loss) */
 amtws[20] = 0.0;	/* Loss limitations (difference between AMT and regular tax income or loss) */
 amtws[21] = 0.0;	/* Circulation costs (difference between regular tax and AMT) */
 amtws[22] = 0.0;	/* Long-term contracts (difference between AMT and regular tax income) */
 amtws[23] = 0.0;	/* Mining costs (difference between regular tax and AMT) */
 amtws[24] = 0.0;	/* Research and experimental costs (difference between regular tax and AMT) */
 amtws[25] = -0.0;	/* Income from certain installment sales before 1/1/87 (As negaitive amount.) */
 amtws[26] = 0.0;	/* Intangible drilling costs preference */
 amtws[27] = 0.0;	/* Other adjustments, including income-based related adjustments */
 for (j = 1; j <= 27; j++)
  amtws[28] = amtws[28] + amtws[j];
 if (status == MARRIED_FILLING_SEPARAT)
  {
   if (amtws[28] > 406650.0) amtws[28] = amtws[28] + 41050.0;
   else
   if (amtws[28] > 242450.0) amtws[28] = amtws[28] + 0.25 * (amtws[28] - 242450.0);
  }

 /* Part II */
 switch (status)
  {
     case SINGLE: case HEAD_OF_HOUSEHOLD:
	thresholdA = 117300.0;
	thresholdB = 328500.0;
	amtexmption = 52800.0;
	break;
     case MARRIED_FILLING_JOINTLY: case WIDOW: 
	thresholdA = 156500.0;
	thresholdB = 484900.0;
	amtexmption = 82100.0;
	break;
     case MARRIED_FILLING_SEPARAT: 
	thresholdA = 78250.0;
	thresholdB = 242450.0;
	thresholdC = 91250.0;
	offsetA = 1825.0;
        amtexmption = 41050.0;
	if (amtws[29] > thresholdB)
	 {
	  if (amtws[29] > 358800.0)
	   amtws[29] = amtws[29] + 35475.0;
	  else 
	   amtws[29] = amtws[29] + 0.25 * (amtws[29] - thresholdB);
	 }
	break;
     default:  printf("Status %d not handled.\n", status);  exit(1); 
  }

 if (amtws[28] <= thresholdA)
  amtws[29] = amtexmption;
 else
 if (amtws[28] >= thresholdB)
   amtws[29] = 0.0;
 else
  { /* Page 9 AMT-Instructions Exemptions Worksheet. */
    ws[1] = amtexmption;
    ws[2] = amtws[28];
    ws[3] = thresholdA;
    ws[4] = NotLessThanZero( ws[2] - ws[3] );
    ws[5] = 0.25 * ws[4];
    ws[6] = NotLessThanZero( ws[1] - ws[5] );
    for (j=0; j<10; j++)
     if (ws[j] != 0.0)
      {
         printf("\t\tAMT pg 9 WrkSht %d: %8.2f\n", j, ws[j] );
         fprintf(outfile,"\t\tAMT pg 8 WrkSht %d: %8.2f\n", j, ws[j] );
      }
    amtws[29] = ws[6];
    /* Certain Children Under Age 24 - not handled here. */
  }

 amtws[30] = NotLessThanZero( amtws[28] - amtws[29] );

 if (amtws[30] > 0.0)
  { /*Lines31-33*/

    if ((L[13] == 0.0) && (L9b == 0.0) && ((SchedD[15] <= 0.0) || (SchedD[16] <= 0.0)))
     {
      if (amtws[30] <= thresholdC)
       amtws[31] = 0.26 * amtws[30];
      else
       amtws[31] = 0.28 * amtws[30] - offsetA;
     }
    else
     { /*Part-III*/ /* Part-III must be done here, due to dependency on line 31. */
   
       /* Part III - Tax Computation Using Maximum Capital Gains Rates */
       amtws[36] = amtws[30];
       amtws[37] = largerof( qcgws6, ws_sched_D[13] );
       amtws[38] = SchedD[19];
       if (Do_SDTW)
        amtws[39] = smallerof( amtws[37] + amtws[38], ws_sched_D[10] );
       else
        amtws[39] = amtws[37];
       amtws[40] = smallerof( amtws[36], amtws[39] );
       amtws[41] = amtws[36] - amtws[40];

       if (amtws[41] <= thresholdC)
        amtws[42] = 0.26 * amtws[41];
       else
        amtws[42] = 0.28 * amtws[41] - offsetA;

       switch (status)
        {
           case MARRIED_FILLING_JOINTLY:  case WIDOW: 
	     amtws[43] = 73800.0;
	   break;
           case SINGLE:  case MARRIED_FILLING_SEPARAT: 
   	     amtws[43] = 36900.0;
   	   break;
           case HEAD_OF_HOUSEHOLD:
   	     amtws[43] = 49400.0;
        }
       amtws[44] = largerof( qcgws7, ws_sched_D[14] );;
       amtws[45] = NotLessThanZero( amtws[43] - amtws[44] );
       amtws[46] = smallerof( amtws[36], amtws[37] );
       amtws[47] = smallerof( amtws[45], amtws[46] );
       amtws[48] = amtws[46] - amtws[47];  
       switch (status)
	{
	   case SINGLE:  			      amtws[49] = 406750.0;  break;
	   case MARRIED_FILLING_SEPARAT:	      amtws[49] = 228800.0;  break;
	   case MARRIED_FILLING_JOINTLY: case WIDOW:  amtws[49] = 457600.0;  break;
	   case HEAD_OF_HOUSEHOLD: 		      amtws[49] = 432200.0;  break;
	   default:  printf("Status %d not handled.\n", status);  exit(1); 
	}
       amtws[50] = amtws[45];
       if ((qcgws7 != 0.0) || (ws_sched_D[19] != 0.0))
	amtws[51] = NotLessThanZero( largerof( qcgws7, ws_sched_D[19] ) );
       else
	amtws[51] = NotLessThanZero( L[43] );
       amtws[52] = amtws[50] + amtws[51];
       amtws[53] = amtws[49] - amtws[52];
       amtws[54] = smallerof( amtws[48], amtws[53] );
       amtws[55] = 0.15 * amtws[54];
       amtws[56] = amtws[47] + amtws[54];
       if (absolutev( amtws[36] - amtws[56] ) > 0.005)
	{ /*lines 57-61*/
	  amtws[57] = amtws[46] - amtws[56];
	  amtws[58] = 0.20 * amtws[57];
	  if (amtws[38] != 0.0)
	   { /*lines 59-61*/
	    amtws[59] = amtws[41] + amtws[56] + amtws[57];
	    amtws[60] = amtws[36] - amtws[59];
	    amtws[61] = 0.25 * amtws[60];
	   } /*lines 59-61*/
	} /*lines 57-61*/
       amtws[62] = amtws[42] + amtws[55] + amtws[58] + amtws[61];
       if (amtws[36] <= thresholdC)
        amtws[63] = 0.26 * amtws[36];
       else
        amtws[63] = 0.28 * amtws[36] - offsetA;
       amtws[64] = smallerof( amtws[62], amtws[63] );
       amtws[31] = amtws[64];
     } /*Part-III*/

    amtws[32] = L[48];			/* Alt min tax foreign tax credit. */
    amtws[33] = amtws[31] - amtws[32];	/* Tentative minimum tax. */
  } /*Lines31-33*/

 amtws[34] = L[44] + L[46] - L[48];
 amtws[35] = NotLessThanZero( amtws[33] - amtws[34] );
 // L[45] = amtws[35];  /* Redundant.  Is assigned by return value below. */

 /* These rules are stated on Form-6251 Instructions page-1. */
 if (amtws[31] > amtws[34])
  fprintf(outfile,"You MUST file AMT form 6251. (%g > %g)\n", amtws[31], amtws[34] );
 else
  {
   for (j=8; j<27; j++)
    sum8_27 = sum8_27 + amtws[j];
   fprintf(outfile," Sum of Form 6251 Lines 8 through 27 = %8.2f\n", sum8_27 );
   fprintf(outfile,"You may need to file AMT form 6251.  (AMTws[31]=%g which is NOT more than AMTws[34]=%g)\n", amtws[31], amtws[34] );
   fprintf(outfile," (See \"Who Must File\" on page-1 of Instructions for Form-6251.)\n");
  }
 for (j=0; j<100; j++) 
  if ((j==35) || (amtws[j] != 0.0))
   {
    printf(" 		AMT Form 6251, L%d = %8.2f\n", j, amtws[j] );
    fprintf(outfile," 		AMT Form 6251, L%d = %8.2f\n", j, amtws[j] );
   }
 fprintf(outfile,"Your Alternative Minimum Tax = %8.2f\n", amtws[35] ); 
 printf("Your Alternative Minimum Tax = %8.2f\n", amtws[35] ); 
 return amtws[35];
}





struct FedReturnData
 {
  double fedline[MAX_LINES], schedD[MAX_LINES];
  int Exception, Itemized;
 } LastYearsReturn;


void convert_slashes( char *fname )
{ /* Convert slashes in file name based on machine type. */
  char *ptr;
 #ifdef __MINGW32__
  char slash_sreach='/', slash_replace='\\';
 #else
  char slash_sreach='\\', slash_replace='/';
 #endif

  ptr = strchr( fname, slash_sreach );
  while (ptr)
   {
    ptr[0] = slash_replace;
    ptr = strchr( fname, slash_sreach );
   }
}


void ImportFederalReturnData( char *fedlogfile, struct FedReturnData *fed_data )
{
 FILE *infile;
 char fline[1000], word[1000];
 int linenum;

 for (linenum=0; linenum<MAX_LINES; linenum++) 
  { fed_data->fedline[linenum] = 0.0;  fed_data->schedD[linenum] = 0.0; }
 convert_slashes( fedlogfile );
 infile = fopen(fedlogfile, "r");
 if (infile==0)
  {
   printf("Error: Could not open federal return '%s'\n", fedlogfile);
   fprintf(outfile,"Error: Could not open federal return '%s'\n", fedlogfile);
   exit(1);
  }
 fed_data->Itemized = 1; /* Set initial default values. */
 read_line(infile,fline);  linenum = 0;
 while (!feof(infile))
  {
   if (strstr(fline,"Use standard deduction.")!=0) fed_data->Itemized = 0;
   next_word(fline, word, " \t=");
   if ((strstr(word,"L")==word) && (strstr(fline," = ")!=0))
    {
     if (sscanf(&word[1],"%d",&linenum)!=1) printf("Error: Reading fed line number '%s%s'\n",word,fline);
     next_word(fline, word, " \t=");
     if (sscanf(word,"%lf", &fed_data->fedline[linenum])!=1)
	printf("Error: Reading fed line %d '%s%s'\n",linenum,word,fline);
     if (verbose) printf("FedLin[%d] = %2.2f\n", linenum, fed_data->fedline[linenum]);
    }
   if ((strstr(word,"D")==word) && (strstr(fline," = ")!=0)) 
    {
     if (sscanf(&word[1],"%d",&linenum)!=1) printf("Error: Reading fed line number '%s%s'\n",word,fline);
     next_word(fline, word, " \t=");
     if (sscanf(word,"%lf", &fed_data->schedD[linenum])!=1) 
      {
       if (strcasecmp(word,"yes") == 0) fed_data->schedD[linenum] = 1;
       else
       if (strcasecmp(word,"no") == 0) fed_data->schedD[linenum] = 0;
       else
	printf("Error: Reading fed schedD %d '%s%s'\n",linenum,word,fline);
      }
     if (verbose) printf("FedLin[%d] = %2.2f\n", linenum, fed_data->schedD[linenum]);
    }
   read_line(infile,fline);
  }
 fclose(infile);
}


void CapitalLossCarryOverWorksheet( char *fedlogfile, struct FedReturnData *LastYearsReturn )	/* Updated for 2014. */
{ /* From instructions page D-11. */
 double ws[50];
 int k;

 ImportFederalReturnData( fedlogfile, LastYearsReturn );
 if (LastYearsReturn->schedD[21] == 0.0) return;  /* Use this worksheet only if last year's D[21] was a loss. */
 if ((absolutev(LastYearsReturn->schedD[21]) >= absolutev(LastYearsReturn->schedD[16])) && (LastYearsReturn->fedline[41] >= 0.0))
  return;

 for (k=0; k<50; k++) ws[k] = 0.0;
 ws[1] = LastYearsReturn->fedline[41];
 ws[2] = absolutev( LastYearsReturn->schedD[21] );	/* Loss from last year's Sched-D21 as positive amount. */
 ws[3] = NotLessThanZero( ws[1] + ws[2] );
 ws[4] = smallerof( ws[2], ws[3] );
 for (k=1; k<=4; k++)
  {
   printf("\tCarryOverWs%d = %2.2f\n", k, ws[k] );
   fprintf(outfile,"\tCarryOverWs%d = %2.2f\n", k, ws[k] );
  }
 if (LastYearsReturn->schedD[7] < 0.0)
  { /*lines5-8*/
    ws[5] = -LastYearsReturn->schedD[7];
    ws[6] = NotLessThanZero( LastYearsReturn->schedD[15] );
    ws[7] = ws[4] + ws[6];
    ws[8] = NotLessThanZero( ws[5] - ws[7] );
    if (ws[8] > 0.0) SchedD[6] = ws[8];
    for (k=5; k<=8; k++)
     {
	printf("\tCarryOverWs%d = %2.2f\n", k, ws[k] );
	fprintf(outfile,"\tCarryOverWs%d = %2.2f\n", k, ws[k] );
     }
  } /*lines5-8*/
 else
  printf("\t(Skip CarryOverWs lines 5-8.)\n");

 if (LastYearsReturn->schedD[15] < 0.0)
  { /*lines9-13*/
    ws[9] = absolutev( LastYearsReturn->schedD[15] );
    ws[10] = NotLessThanZero( LastYearsReturn->schedD[7] );
    ws[11] = NotLessThanZero( ws[4] - ws[5] );
    ws[12] = ws[10] + ws[11];
    ws[13] = NotLessThanZero( ws[9] - ws[12] );
    if (ws[13] > 0.0) SchedD[14] = ws[13];
    for (k=9; k<=13; k++)
     {
	printf("\tCarryOverWs%d = %2.2f\n", k, ws[k] );
	fprintf(outfile,"\tCarryOverWs%d = %2.2f\n", k, ws[k] );
     }
  } /*lines9-13*/
 else
  printf("\t(Skip CarryOverWs lines 9-13.)\n");
}



struct capgain_record
 {
  char *comment, *buy_date, *sell_date;
  double buy_amnt, sell_amnt;
  struct capgain_record *nxt;
 } *short_trades=0, *long_trades=0;

double total_sales, total_costs=0.0;


void new_capgain( struct capgain_record **list, char *comment, double buy_amnt, 
					char *buy_date, double sell_amnt, char *sell_date )
{ /* Add a new entry to a list. */
  struct capgain_record *new_item, *prev;

  new_item = (struct capgain_record *)malloc( sizeof(struct capgain_record) );
  new_item->comment = strdup( comment );	/* Make new list item and fill-in its fields. */
  new_item->buy_amnt = buy_amnt;
  new_item->buy_date = strdup( buy_date );
  new_item->sell_amnt = sell_amnt;
  new_item->sell_date = strdup( sell_date );
  new_item->nxt = 0;
  prev = *list;		/* Insert onto end of list. */
  if (prev == 0)
   *list = new_item;
  else
   {
    while (prev->nxt != 0) prev = prev->nxt;
    prev->nxt = new_item;
   }
}


void printf_capgain_list( struct capgain_record *list, int section, char const *message )
{
 struct capgain_record *item;
 char word[4096];

 total_sales = 0.0;
 total_costs = 0.0;
 fprintf(outfile,"\n%s\n", message );
 fprintf(outfile," %d. (a Description)         (b Buy Date) (c Date Sold) (d Sold Price) (e Cost) (h Gain)\n", section );
 fprintf(outfile," ---------------------------------------------------------------------------------------\n");
 item = list;
 while (item != 0)
  {
   strcpy( word, item->comment );
   if (strlen( word ) > 27) word[30] = '\0';
   if ((strlen(word) > 0) && (word[ strlen(word) - 1 ] == '}')) word[ strlen(word) - 1 ] = '\0';
   while (strlen( word ) < 27) strcat( word, " " ); 	/* Fields become formatted right-justified. */
   fprintf(outfile," %s %10s %10s %14.2f %14.2f %14.2f\n", word, item->buy_date, item->sell_date, item->sell_amnt, 
	item->buy_amnt, item->sell_amnt + item->buy_amnt );
   total_sales = total_sales + item->sell_amnt;
   total_costs = total_costs + item->buy_amnt;
   item = item->nxt;
  }
 fprintf(outfile," ---------------------------------------------------------------------------------------\n");
 fprintf(outfile," %d. Totals:                                        %14.2f %14.2f %14.2f\n\n", 
	section + 1, total_sales, total_costs, total_sales + total_costs );
}


void free_capgain_list( struct capgain_record **list )
{
 struct capgain_record *olditem;

 while (*list != 0)
  {
   olditem = *list;
   *list = (*list)->nxt;
   free( olditem->comment );
   free( olditem );
  }
}


void get_gain_and_losses( char const *label )
{
 char comment[4096], date_str1[512], date_str2[512], word[4096];
 double amnt1, amnt2;
 int toggle=0, date1=0, date2;

 get_parameter( infile, 's', word, label );     /* Capital gains. */
 get_word(infile, word);
 while (word[0]!=';')
 { /*while_not_end*/
  if (feof(infile))
   {printf("ERROR: Unexpected EOF on '%s'\n", label ); fprintf(outfile,"ERROR: Unexpected EOF on '%s'\n", label ); exit(1);}
  if (!Do_SchedD) {fprintf(outfile,"\nForm(s) 8949:\n"); Do_SchedD = Yes;}

  switch (toggle)
   { /*switch_toggle*/
    case 0:	toggle++;
	if (sscanf(word,"%lf",&amnt1)!=1)
	 {printf("ERROR: Bad float '%s', reading %s.\n", word, label ); fprintf(outfile,"ERROR: Bad float '%s', reading %s.\n", word, label ); exit(1); }
	if (amnt1 > 0.0) amnt1 = -amnt1;  /* Buy amounts must be negative. (It is a cost.) */
	break;
    case 1:	toggle++;
	strcpy( date_str1, word );
	date1 = get_date( word, label );
	/* Expect stock name in comment after first date (buy-date). */
	get_comment( infile, comment );
	break;
    case 2:	toggle++;
	if (sscanf(word,"%lf",&amnt2)!=1)
	 {printf("ERROR: Bad float '%s', reading %s.\n", word, label ); fprintf(outfile,"ERROR: Bad float '%s', reading %s.\n", word, label ); exit(1); }
	break;
    case 3:	toggle = 0;
	strcpy( date_str2, word );
	date2 = get_date( word, label );
	if ((date2-date1) < 0)
	 {printf("ERROR: Buy-date after sell-date.\n"); fprintf(outfile,"ERROR: Buy-date after sell-date.\n");  exit(1);}
	if ((date2-date1) > 365) 
	 { /*long-gain/loss*/
	  new_capgain( &long_trades, comment, amnt1, date_str1, amnt2, date_str2 );
	 } /*long-gain/loss*/
	else 
	 { /*short-gain/loss*/
	  new_capgain( &short_trades, comment, amnt1, date_str1, amnt2, date_str2 );
	 } /*short-gain/loss*/
	break;
   } /*switch_toggle*/
  get_word(infile, word);
 } /*while_not_end*/
 if (toggle!=0)
  {printf("ERROR: Imbalanced cap-gains entry (toggle=%d).\n", toggle);  fprintf(outfile,"ERROR: Imbalanced cap-gains entry (toggle=%d).\n", toggle); exit(1); }
}



/************************************************************************/
/* Get_Cap_Gains - Get and calculate gains.  Forms 8949 + Sched-D.	*/
/* Like "get_params", but must get transaction dates.			*/
/* Expect entries in double pairs. 					*/
/*   buy_amnt   date 							*/
/*   sell_amnt  date 							*/
/*									*/
/************************************************************************/
void get_cap_gains( char *emssg	)							/* Updated for 2014. */
{
 char word[1024], *LastYearsOutFile=0;
 int j, doline22=0;
 double stcg=0.0, ltcg=0.0;      /* Variables for short and long term gains. */
 double SchedDd[20], SchedDe[20];

 for (j=0; j<20; j++) { SchedDd[j] = 0.0;  SchedDe[j] = 0.0; }
 /* Form 8849 - Adjunct form to Schedule-D. */
 get_gain_and_losses( "Cap-Gains-A" );	/* (A) Basis Reported to IRS. */
 if (short_trades)
  {
   printf_capgain_list( short_trades, 1, "Form 8949 Part-I, Short-Term Cap Gains+Losses, CHECK (A) Basis Reported to IRS:" );
   SchedDd[1] = total_sales;
   SchedD["1d"] = total_sales;
   SchedDe[1] = total_costs;
   SchedD["1e"] = total_costs;
   SchedD[1] = SchedDd[1] + SchedDe[1];
   free_capgain_list( &short_trades );
   output_SchedD=Yes;
  }
 if (long_trades)
  {
   printf_capgain_list( long_trades, 3, "Form 8949 Part-II, Long-Term Cap Gains+Losses, CHECK (A) Basis Reported to IRS:"  );
   SchedDd[8] = total_sales;
   SchedD["8d"] = total_sales;
   SchedDe[8] = total_costs;
   SchedD["8e"] = total_costs;
   SchedD[8] = SchedDd[8] + SchedDe[8];
   free_capgain_list( &long_trades );
   output_SchedD=Yes;
  }

 get_gain_and_losses( "Cap-Gains-B" );	/* (B) Basis NOT Reported to IRS. */
 if (short_trades)
  {
   printf_capgain_list( short_trades, 1, "Form 8949 Part-I, Short-Term Cap Gains+Losses, CHECK (B) Basis NOT Reported to IRS:" );
   SchedDd[2] = total_sales;
   SchedD["2d"] = total_sales;
   SchedDe[2] = total_costs;
   SchedD["2e"] = total_costs;
   SchedD[2] = SchedDd[2] + SchedDe[2];
   free_capgain_list( &short_trades );
   output_SchedD=Yes;
  }
 if (long_trades)
  {
   printf_capgain_list( long_trades, 3, "Form 8949 Part-II, Long-Term Cap Gains+Losses, CHECK (B) Basis NOT Reported to IRS:"  );
   SchedDd[9] = total_sales;
   SchedD["9d"] = total_sales;
   SchedDe[9] = total_costs;
   SchedD["9e"] = total_costs;
   SchedD[9] = SchedDd[9] + SchedDe[9];
   free_capgain_list( &long_trades );
   output_SchedD=Yes;
  }

 get_gain_and_losses( "Cap-Gains-C" );	/* (C) Cannot check (A) or (B). */
 if (short_trades)
  {
   printf_capgain_list( short_trades, 1, "Form 8949 Part-I, Short-Term Cap Gains+Losses, CHECK (C) Not reported on Form 1099-B.\n" );
   SchedDd[3] = total_sales;
   SchedD["3d"] = total_sales;
   SchedDe[3] = total_costs;
   SchedD["3e"] = total_costs;
   SchedD[3] = SchedDd[3] + SchedDe[3];
   free_capgain_list( &short_trades );
   output_SchedD=Yes;
  }
 if (long_trades)
  {
   printf_capgain_list( long_trades, 3, "Form 8949 Part-II, Long-Term Cap Gains+Losses, CHECK (C) Not reported on Form 1099-B.\n"  );
   SchedDd[10] = total_sales;
   SchedD["10d"] = total_sales;
   SchedDe[10] = total_costs;
   SchedD["10e"] = total_costs;
   SchedD[10] = SchedDd[10] + SchedDe[10];
   free_capgain_list( &long_trades );
   output_SchedD=Yes;
  }

 stcg = SchedD[1] + SchedD[2] + SchedD[3];
 ltcg = SchedD[8] + SchedD[9] + SchedD[10];

 GetLine( "D4", &SchedD[4] );       /* Short term gain from 6252 and short-term gain or loss from Forms 4684, 6781, 8824. */
 GetLine( "D5", &SchedD[5] );       /* Net short-term gain or loss from partnerships, S corps, estates, trusts from K-1. */

 get_parameter( infile, 's', word, "D6" );	/* Carryover short-term loss from last year.  Or, LastYear's Return Output File-name. */
 get_word(infile,word);
 if (strcmp(word,";") != 0)
  {
   if (sscanf(word,"%lf",&SchedD[6]) != 1) LastYearsOutFile = strdup(word);
   do
    { get_word(infile,word); 
      if ((strlen(word) > 0) && (strcmp(word,";") != 0))
       fprintf(outfile,"Warning: Unexpected multiple values on line D6.  '%s' ignored.\n If multi-part filename, then surround it in quotes (\").", word );
    } while (strcmp(word,";") != 0);
  }

 GetLine( "D11", &SchedD[11] );	    /* Gain from Form 4797. */
 GetLine( "D12", &SchedD[12] );	    /* Partnership net long-term gain or loss. */
 GetLine( "D13", &SchedD[13] );	    /* Cap Gains Distributions - 1099-DIV col. 2a. */
 GetLine( "D14", &SchedD[14] );     /* Carryover long-term loss from last year. Or, leave blank if last year's file entered in line D6. */

 if (LastYearsOutFile != 0)
  CapitalLossCarryOverWorksheet( LastYearsOutFile, &LastYearsReturn );

 if (SchedD[6] > 0.0)
  { 
   /* fprintf(outfile,"Warning: D6 Carryover Loss must be NEGATIVE.\n"); */
   SchedD[6] = -SchedD[6];
  }
 if (SchedD[14] > 0.0)
  { 
   /* fprintf(outfile,"Warning: D14 Carryover Loss must be NEGATIVE.\n"); */
   SchedD[14] = -SchedD[14];
  }

 if ((SchedD[4] != 0.0) || (SchedD[5] != 0.0) || (SchedD[6] != 0.0) || (SchedD[11] != 0.0) || 
     (SchedD[12] != 0.0) || (SchedD[13] != 0.0) || (SchedD[14] != 0.0))
  { Do_SchedD = Yes; }	/* Set Do_SchedD in case it was not already set by Cap-Gain/Loss in rows 1-3, or 8-10. */

 if (Do_SchedD)
  { /*Sched-D*/
   fprintf(outfile," Cap Gains/Losses Schedule-D\n");
   // Do_QDCGTW = Yes;	/* Tentatively set to do: Qualified Dividends and Captial Gain tax Worksheet. */
   fprintf(outfile,"\tNet Forms-8949 Short-term Gains = %10.2f\n", stcg );
   fprintf(outfile,"\tNet Forms-8949 Long-term Gains  = %10.2f\n", ltcg);
   fprintf(outfile," D1b: d = %10.2f   e = %10.2f    h = %10.2f\n", SchedDd[1], SchedDe[1], SchedD[1] );
   fprintf(outfile," D2:  d = %10.2f   e = %10.2f    h = %10.2f\n", SchedDd[2], SchedDe[2], SchedD[2] );
   fprintf(outfile," D3:  d = %10.2f   e = %10.2f    h = %10.2f\n", SchedDd[3], SchedDe[3], SchedD[3] );
   fprintf(outfile," D4 = %6.2f\n", SchedD[4] );
   fprintf(outfile," D5 = %6.2f\n", SchedD[5] );
   fprintf(outfile," D6 = %6.2f		(Carry-over Loss)\n", SchedD[6] );
   SchedD[7] = SchedD[1] + SchedD[2] + SchedD[3] + SchedD[4] + SchedD[5] + SchedD[6];
   fprintf(outfile," D7 = %6.2f		{ Net short-term capital gain or loss }\n", SchedD[7] );
   fprintf(outfile," D8b: d = %10.2f   e = %10.2f   h = %10.2f\n", SchedDd[8], SchedDe[8], SchedD[8] );
   fprintf(outfile," D9:  d = %10.2f   e = %10.2f   h = %10.2f\n", SchedDd[9], SchedDe[9], SchedD[9] );
   fprintf(outfile," D10: d = %10.2f   e = %10.2f   h = %10.2f\n", SchedDd[10], SchedDe[10], SchedD[10] );
   fprintf(outfile," D11 = %6.2f\n", SchedD[11] );
   fprintf(outfile," D12 = %6.2f\n", SchedD[12] );
   fprintf(outfile," D13 = %6.2f\n", SchedD[13] );
   fprintf(outfile," D14 = %6.2f	(Carry-over Loss)\n", SchedD[14] );
   SchedD[15] = SchedD[8] + SchedD[9] + SchedD[10] + SchedD[11] + SchedD[12] + SchedD[13] + SchedD[14];
   fprintf(outfile," D15 = %6.2f		{ Net long-term capital gain or loss }\n", SchedD[15] );

   /* Part ||| */
   SchedD[16] = SchedD[7] + SchedD[15];
   fprintf(outfile," D16 = %6.2f\n", SchedD[16]);
   if (SchedD[16] > 0.0) 
    { /*gain*/
     L[13] = SchedD[16];
     if ((SchedD[15] > 0.0) && (SchedD[16] > 0.0))
      { /* Lines 17-21 */
	double wsd[50];

	SchedD["17y"] = 1;

	fprintf(outfile," D17 = yes\n");

	/* '28% Rate Gain Worksheet' on instructions page D-12. */
	wsd[1] = collectibles_gains;	/* Gain or losses from "Collectibles" only.  Usually zero. */
	wsd[2] = 0.0;	/* Any 1202 exclusions, usually 0.0. */
	wsd[3] = 0.0;	/* Total collectibles on forms 4684, 6245, 6781, 8824. Usually no. */
	wsd[4] = 0.0;	/* Total collectibles 1099-Div box 2d, 2439 box 1d, or K-1's. Usually no. */
	wsd[5] = SchedD[14];
	if (SchedD[7] < 0.0)  wsd[6] = SchedD[7];  else  wsd[6] = 0.0;
	wsd[7] = NotLessThanZero( wsd[1] + wsd[2] + wsd[3] + wsd[4] + wsd[5] + wsd[6] );
	SchedD[18] = wsd[7];
	fprintf(outfile," D18 = %6.2f\n", SchedD[18]);

	/* 'Unrecaptured Section 1250 Gain Worksheet' on page D13, usually 0. */
	fprintf(outfile," D19 = %6.2f\n", SchedD[19]);

        if ((SchedD[18] == 0.0) && (SchedD[19] == 0.0))
	 { /*yes*/
	  fprintf(outfile," D20 = Yes\n");
	  SchedD["20y"] = 1;
	  // printf("Complete 'Qualified Dividends and Captial Gain tax Worksheet', instructions page 43.\n");
	  Do_QDCGTW = Yes;
	 } /*yes*/
	else
	 { /*no*/
	  fprintf(outfile," D20 = No\n");
	  SchedD["20n"] = 1;
	  // printf("Complete 'Schedule D Tax Worksheet', instructions page D-15.\n");
	  Do_SDTW = Yes;
	  Do_QDCGTW = No;
	 } /*no*/
       doline22 = 0;
      } /* Lines 17-21 */
     else 
      {
       printf(" D17 = no\n");
       SchedD["17n"] = 1;
       doline22 = Yes;
      }
    } /*gain*/  
   else
   if (SchedD[16] < 0.0) 
    { /*loss*/	/* Schedule-D line 21. Skip to here from line 16 if a loss. */
     double maxloss;

     if (status == MARRIED_FILLING_SEPARAT) maxloss = -1500.0; else maxloss = -3000.0;
     if (SchedD[16] < maxloss) SchedD[21] = maxloss; else SchedD[21] = SchedD[16];
     fprintf(outfile," D21 = %6.2f\n", SchedD[21]);
     L[13] = SchedD[21];
     doline22 = Yes;
    }
   else
    { /*Zero gain/loss.*/
     L[13] = 0.0;
     doline22 = Yes;
    }

   if (doline22)
    {
     if (L9b > 0.0)
      { /*yes*/
       fprintf(outfile," D22 = Yes\n");
       SchedD["22y"] = 1;
       // printf("Complete 'Qualified Dividends and Captial Gain tax Worksheet', instructions page 34.\n");
       Do_QDCGTW = Yes;	
      } /*yes*/
     else
      { /*no*/
       fprintf(outfile," D22 = No\n");
       SchedD["22n"] = 1;
       // Do_QDCGTW = No;	
      } /*no*/
    }

  } /*Sched-D*/
}


/*------------------------------------------------------*/
/* 'Schedule D Tax Worksheet', instructions page D14.	*/
/*------------------------------------------------------*/
void sched_D_tax_worksheet( int status, double L9b )				/* Updated for 2014. */
{
 double ws[100];
 int k;

 for (k = 0; k < 100; k++) ws[k] = 0.0;
 ws[1] = L[43];
 ws[2] = L9b;
 ws[3] = 0.0;	/* Form 4952, line 4g. Usually 0.0. */
 ws[4] = 0.0;	/* Form 4952, line 4e. Usually 0.0. */
 ws[5] = NotLessThanZero( ws[3] - ws[4] );
 ws[6] = NotLessThanZero( ws[2] - ws[5] );
 ws[7] = smallerof( SchedD[15], SchedD[16] );
 ws[8] = smallerof( ws[3], ws[4] );
 ws[9] = NotLessThanZero( ws[7] - ws[8] );
 ws[10] = ws[6] + ws[9];
 fprintf(outfile,"  Sched-D tax Worksheet line 10 = %6.2f\n", ws[10]);
 ws[11] = SchedD[18] + SchedD[19];
 ws[12] = smallerof( ws[9], ws[11] );
 ws[13] = ws[10] - ws[12];
 ws[14] = NotLessThanZero( ws[1] - ws[13] );
 fprintf(outfile,"  Sched-D tax Worksheet line 13 = %6.2f\n", ws[13]);
 fprintf(outfile,"  Sched-D tax Worksheet line 14 = %6.2f\n", ws[14]);
 switch (status) 
  { case SINGLE: case MARRIED_FILLING_SEPARAT: ws[15] = 36900.0; break;
    case MARRIED_FILLING_JOINTLY: case WIDOW:  ws[15] = 73800.0; break;
    case HEAD_OF_HOUSEHOLD:      	       ws[15] = 49400.0; break;
  }
 ws[16] = smallerof( ws[1], ws[15] );
 ws[17] = smallerof( ws[14], ws[16] );
 ws[18] = NotLessThanZero( ws[1] - ws[10] );
 ws[19] = largerof( ws[17], ws[18] );
 ws[20] = ws[16] - ws[17];	/* This amount is taxed at 0%. */
 if (ws[1] != ws[16])
  { /*lines21-41*/
   ws[21] = smallerof( ws[1], ws[13] );
   ws[22] = ws[20];
   ws[23] = NotLessThanZero( ws[21] - ws[22] );
   switch (status) 
    { case SINGLE: 			ws[24] = 406750.0;  break;
      case MARRIED_FILLING_SEPARAT: 	ws[24] = 228800.0;  break;
      case MARRIED_FILLING_JOINTLY: 
      case WIDOW:  			ws[24] = 457600.0;  break;
      case HEAD_OF_HOUSEHOLD:		ws[24] = 432200.0;  break;
    }
   ws[25] = smallerof( ws[1], ws[24] );
   ws[26] = ws[19] + ws[20];
   ws[27] = NotLessThanZero( ws[25] - ws[26] );
   ws[28] = smallerof( ws[23], ws[27] );
   ws[29] = 0.15 * ws[28];
   ws[30] = ws[22] + ws[28];
   if (ws[1] != ws[30])
    { /*lines31-41*/
      ws[31] = ws[21] - ws[30];
      ws[32] = 0.20 * ws[31];
      if (SchedD[19] != 0.0)
       { /*lines33-38*/
	 ws[33] = smallerof( ws[9], SchedD[19] );
	 ws[34] = ws[10] + ws[19];
	 ws[35] = ws[1];
	 ws[36] = NotLessThanZero( ws[34] - ws[35] );
	 ws[37] = NotLessThanZero( ws[33] - ws[36] );
	 ws[38] = 0.25 * ws[37];
       } /*lines33-38*/
      if (SchedD[18] != 0.0)
       { /*lines39-41*/
	 ws[39] = ws[19] + ws[20] + ws[28] + ws[31] + ws[37];
	 ws[40] = ws[1] - ws[39];
	 ws[41] = 0.28 * ws[40];
       } /*lines39-41*/
    } /*lines31-41*/
  } /*lines21-41*/
 ws[42] = TaxRateFunction( ws[19], status );
 ws[43] = ws[29] + ws[32] + ws[38] + ws[41] + ws[42];
 ws[44] = TaxRateFunction( ws[1], status );
 ws[45] = smallerof( ws[43], ws[44] );
 L[44] = ws[45];
 for (k = 0; k < 100; k++) ws_sched_D[k] = ws[k];	/* Save worksheet values for AMT, if needed. */
}



/*----------------------------------------------------------------------*/
/* Main									*/
/*----------------------------------------------------------------------*/
int main( int argc, char *argv[] )						/* Not Yet Updated for 2014. */
{
 int argk, j, k, itemize=0;
 char word[2000], outfname[1000], f1040_xfdf_outfname[1000], f1040sa_xfdf_outfname[1000], f1040sd_xfdf_outfname[1000];
 time_t now;
 double exemption_threshold=0.0, dedexws[20];
 double S_STD_DEDUC, MFS_STD_DEDUC, MFJ_STD_DEDUC, HH_STD_DEDUC;

 /* Decode any command-line arguments. */
 printf("US 1040 2014 - v%3.2f\n", thisversion);
 argk = 1;  k=1;
 while (argk < argc)
 {
  if (strcmp(argv[argk],"-verbose")==0)  { verbose = Yes; }
  else
  if (k==1)
   {
    infile = fopen(argv[argk],"r");
    if (infile==0) {printf("ERROR: Parameter file '%s' could not be opened.\n", argv[argk]); exit(1);}
    k = 2;
    /* Base name of output file on input file. */
    strcpy(outfname,argv[argk]);
    strcpy(f1040_xfdf_outfname,argv[argk]);
    strcpy(f1040sa_xfdf_outfname,argv[argk]);
    strcpy(f1040sd_xfdf_outfname,argv[argk]);
    j = strlen(outfname)-1;
    while ((j>=0) && (outfname[j]!='.')) j--;
    if (j<0) {
	strcat(outfname,"_out.txt");
	strcat(f1040_xfdf_outfname,"_f1040.xfdf");
	strcat(f1040sa_xfdf_outfname,"_f1040sa.xfdf");
	strcat(f1040sd_xfdf_outfname,"_f1040sd.xfdf");
	} else {
	strcpy(&(outfname[j]),"_out.txt");
	strcpy(&(f1040_xfdf_outfname[j]),"_f1040.xfdf");
	strcpy(&(f1040sa_xfdf_outfname[j]),"_f1040sa.xfdf");
	strcpy(&(f1040sd_xfdf_outfname[j]),"_f1040sd.xfdf");
	}
    outfile = fopen(outfname,"w");
    if (outfile==0) {printf("ERROR: Output file '%s' could not be opened.\n", outfname); exit(1);}
    printf("Writing results to file:  %s\n", outfname);
   }
  else
   {printf("Unknown command-line parameter '%s'\n", argv[argk]); exit(1);}
  argk = argk + 1;
 }

 if (infile==0) {printf("Error: No input file on command line.\n"); exit(1);}

 /* Pre-initialize all lines to zeros. */
 for (j=0; j<MAX_LINES; j++)
  { 
   L[j] = 0.0;
   SchedA[j] = 0.0; 
   SchedD[j] = 0.0;
   idws[j] = 0.0;
   ws_sched_D[j] = 0.0;
  }

 /* Accept parameters from input file. */
 /* Expect  US-Fed-1040 lines, something like:
	Title:  Federal 1040 1999 Return
	L7		{Wages}
	L8a		{Interest}
	L9		{Dividends}
	L13		{Capital Gains}
	L38		{Itemized deductions from Schedule A or Std Deduction}
	L58		{Witheld tax, from W-2}
 */

 /* Accept Form's "Title" line, and put out with date-stamp for your records. */
 read_line( infile, word );
 now = time(0);
 fprintf(outfile,"\n%s 	%s\n", word, ctime( &now ));

 /* get_parameter(infile, kind, x, emssg ) */
 get_parameter( infile, 's', word, "Status" );	/* Single, Married/joint, Married/sep, Head house, Widow(er) */
 get_parameter( infile, 'l', word, "Status?");
 if (strncasecmp(word,"Single",4)==0) { status = SINGLE; L["S"] = 1; }else
 if (strncasecmp(word,"Married/Joint",13)==0) {status = MARRIED_FILLING_JOINTLY; L["MJ"] = 1;} else
 if (strncasecmp(word,"Married/Sep",11)==0) {status = MARRIED_FILLING_SEPARAT; L["MS"] = 1;} else
 if (strncasecmp(word,"Head_of_House",4)==0) {status = HEAD_OF_HOUSEHOLD; L["HoH"] = 1;} else
 if (strncasecmp(word,"Widow",4)==0) {status = WIDOW; L["QW"] = 1;}
 else
  { 
   printf("Error: unrecognized status '%s'. Exiting.\n", word); 
   fprintf(outfile,"Error: unrecognized status '%s'. Exiting.\n", word); 
   exit(1);
  }
 fprintf(outfile,"Status = %s (%d)\n", word, status);

 get_parameter( infile, 's', word, "Under65?" );	/* Age at end of 2014. (Y/N) If Married/Joint, Y=both-under-65, N=both-over-65, 1=One-over-65 */
 get_parameter( infile, 'l', word, "Under65?" );
 if (strncasecmp(word,"Yes",1) == 0)
  { if (status == MARRIED_FILLING_JOINTLY) under65 = 2;  else  under65 = 1; }
 else
 if (strncasecmp(word,"No",1) == 0)
  under65 = 0;
 else
  {
   if ((sscanf(word, "%d", &under65) != 1) || (under65 < 0) || (under65 > 2))
    { printf("Error: unrecognized answer to 'Under65?' ('%s'). Exiting.\n", word); fprintf(outfile,"Error: unrecognized answer to 'Under65?' ('%s'). Exiting.\n", word); exit(1); }
  }
 fprintf(outfile," (Under65 = %d)\n", under65 );

 GetLine1( "Dependents", &L[6] );
 shownum(6);
 GetLineF( "L7", &L[7] );	/* Wages, salery, tips (W-2). */
 GetLineF( "L8a", &L[8] );	/* Taxable interest. (Sched-B) */
 GetLineF( "L9", &L[9] );	/* Ordinary Dividends. (Sched-B) */
 GetLineF( "L9b", &L9b );	/* Qualified Dividends. (Sched-B) */
 L["9b"] = L9b;
 if (L9b > 0.0) Do_QDCGTW = Yes;	
 if ((L[8] != 0.0) || (L[9] != 0.0))
  {
   fprintf(outfile," Schedule-B:\n");
   fprintf(outfile,"  B2 = %6.2f\n", L[8] );
   fprintf(outfile,"  B4 = %6.2f\n", L[8] );
   fprintf(outfile,"  B6 = %6.2f\n", L[9] );
  }
 GetLineF( "L10", &L[10] );	/* Taxable refunds. */
 GetLineF( "L11", &L[11] );	/* Alimony received. */
 GetLineF( "L12", &L[12] );	/* Business income/loss. */

 get_cap_gains( "L13" );	 /* Capital gains. (Schedule-D) */
 showline(13);

 GetLine( "L14", &L[14] );	/* Other gains or losses. Form 4794. */
 ShowLineNonZero(14);	

 GetLine( "L15b", &L[15] );	/* Taxable IRA distributions (pg 24). */
 ShowLineNonZero(15);

 GetLine( "L16b", &L[16] );	/* Taxable Pensions/annuities (pg 25) */
 ShowLineNonZero(16);

 GetLine( "L17", &L[17] );	/* Rent realestate, royalties, partnerships, S corp. (Sched E)*/
 ShowLineNonZero(17);

 GetLine( "L18", &L[18] );	/* Farm income/loss. (Sched F) */
 ShowLineNonZero(18);

 GetLine( "L19", &L[19] );	/* Unemployment compensation */
 ShowLineNonZero(19);

 GetLine( "L20b", &L[20] );	/* Taxable Social security benefits */
 ShowLineNonZero(20);

 GetLine( "L21", &L[21] );	/* Other income. (pg 28) */
 ShowLineNonZero(21);

 for (j=7; j<=21; j++) L[22] = L[22] + L[j];
 showline_wmsg(22,"total income");

 if (under65 == 0) over65 = 1; 
 switch (status)	/* Check for minimum income to file. */				/* Updated for 2014. */
  {
   case SINGLE:  		  if (under65) exemption_threshold = 10150.0;
				  else  exemption_threshold = 11700.0;
	break;
   case MARRIED_FILLING_JOINTLY:  if (under65==2) exemption_threshold = 20300.0;  else 
				  if (under65==1) exemption_threshold = 21500.0;  
				  else  exemption_threshold = 22700.0;
				  if (under65 != 2) over65 = 1;
	break;
   case MARRIED_FILLING_SEPARAT:  exemption_threshold = 3950.0;
	break;
   case HEAD_OF_HOUSEHOLD: 	  if (under65) exemption_threshold = 13050.0;  
				  else  exemption_threshold = 14600.0;
	break;
   case WIDOW:  		  if (under65) exemption_threshold = 16350.0;  
				  else  exemption_threshold = 17550.0;
  }
 if (L[22] < exemption_threshold)
  {
   printf(" (L22 = %3.2f < Threshold = %3.2f)\n", L[22], exemption_threshold );
   printf("You may not need to file a return, due to your income level.\n"); 
   fprintf(outfile,"You may not need to file a return, due to your income level.\n");
  }


 /* Adjusted Gross Income section. */
 GetLine( "L23", &L[23] );	/* Educator expenses */
 ShowLineNonZero(23);
 GetLine( "L24", &L[24] );	/* Bus. exp.: reservists, artists, ... Attach Form 2106 */
 ShowLineNonZero(24);

 GetLine( "L25", &L[25] );	/* Health savings account deduction. Attach Form 8889 */
 GetLine( "L26", &L[26] );	/* Moving expenses. Attach Form 3903*/
 GetLine( "L27", &L[27] );	/* One-half of self-employment tax. Attach Schedule SE*/
 GetLine( "L28", &L[28] );	/* Self-employed SEP, SIMPLE, and qualified plans */
 GetLine( "L29", &L[29] );	/* Self-employed health insurance deduction */
 GetLine( "L30", &L[30] );	/* Penalty on early withdrawal of savings*/
 GetLine( "L31a", &L[31] );	/* Alimony paid*/
 GetLine( "L32", &L[32] );	/* IRA deduction */
 GetLine( "L33", &L[33] );	/* Student loan interest deduction */
 GetLine( "L34", &L[34] );	/* Tuition and fees. Form 8917. */
 GetLine( "L35", &L[35] );	/* Domestic production activities deduction, Form 8903 */

 if (L[33] != 0.0)
 { /* Student loan interest calculation pg 36. */
  double ws[20];
  ws[1] = smallerof( L[33], 2500.0 );
  ws[2] = L[22];
  ws[3] = L[23] + L[24] + L[25] + L[26] + L[27] + L[29] + L[30] + L[31] + L[32];
  ws[4] = ws[2] - ws[3];
  if (status == MARRIED_FILLING_JOINTLY) ws[5] = 130000.0; else ws[5] = 65000.0;
  if (ws[4] > ws[5])
   {
    ws[6] = ws[4] - ws[5];
    if (status == MARRIED_FILLING_JOINTLY) ws[7] = ws[6] / 30000.0; 
    else ws[7] = ws[6] / 15000.0;
    if (ws[7] >= 1.0) ws[7] = 1.0;
    ws[8] = ws[1] * ws[7];
   }
  else ws[8] = 0.0;
  ws[9] = ws[1] - ws[8];
  L[33] = ws[9];
 }

 ShowLineNonZero(25);
 ShowLineNonZero(26);
 ShowLineNonZero(27);
 ShowLineNonZero(28);
 ShowLineNonZero(29);
 ShowLineNonZero(30);
 ShowLineNonZero(31);
 ShowLineNonZero(32);
 ShowLineNonZero(33);
 ShowLineNonZero(34);
 ShowLineNonZero(35);

 for (j=23; j<=35; j++)
  L[36] = L[36] + L[j];
 showline(36);

 L[37] = L[22] - L[36];
 showline_wmsg( 37, "Adjusted Gross Income" );

 /* Taxes and Credits section. */
 L[38] = L[37];
 showline(38);

 GetLine( "L39", &L[39] );	/* Number of boxes checked (0-4), line-39:  You, Spouse : >65, blind.  */
 fprintf(outfile, "L39a = %d\n", (int)L[39] ); 

 GetLine( "Collectibles", &collectibles_gains );	/* Gains or Losses from Collectibles. (Usually zero.) */
 if (collectibles_gains != 0.0) fprintf(outfile, "Collectibles_Gains = %6.2f\n", collectibles_gains );

 /* Schedule A */
 GetLine( "A1", &SchedA[1] );	/* Unreimbursed medical expenses. */
  showschedA(1);
 SchedA[2] = L[38];
  showschedA(2);
 if (over65) 
  SchedA[3] = 0.075 * SchedA[2];	
 else
  SchedA[3] = 0.10 * SchedA[2];	
  showschedA(3);
 SchedA[4] = NotLessThanZero( SchedA[1] - SchedA[3] );
  showschedA(4);
 GetLine( "A5", &SchedA[5] );	/* State and local income taxes. */
  showschedA(5);
 GetLine( "A6", &SchedA[6] );	/* Real estate taxes. */
  showschedA(6);
 GetLine( "A7", &SchedA[7] );	/* Personal property (eg. automobile) taxes. */
  showschedA(7);
 GetLine( "A8", &SchedA[8] );	/* Other taxes. */
  showschedA(8);
 SchedA[9] = SchedA[5] + SchedA[6] + SchedA[7] + SchedA[8];
  showschedA(9);
 GetLine( "A10", &SchedA[10] );	/* Home mortgage interest and points */
  showschedA(10);
 GetLine( "A11", &SchedA[11] );	/* Home mortgage interest not reported to you on Form 1098.*/
  showschedA(11);
 GetLine( "A12", &SchedA[12] );	/* Points not reported to you on Form 1098.*/
  showschedA(12);
 GetLine( "A13", &SchedA[13] );	/* Mortgage insurance premiums (see instructions). */
  if (SchedA[13] != 0.0)
   { double threshA=109000.0, threshB=100000.0;
    if (status == MARRIED_FILLING_SEPARAT)
     { threshA = 54500.0;  threshB = 50000.0; }
    if (L[38] > threshA)
     { SchedA[13] = 0.0; printf("You cannot deduct your mortgage insurance premiums.\n"); }
    else
    if (L[38] > threshB)
     { printf("Your mortgage insurance premiums deduction is limited. Use worksheet.\n"); }
   }
  showschedA(13);
 GetLine( "A14", &SchedA[14] );	/* Investment interest. Attach Form 4952*/
  showschedA(14);
 SchedA[15] = SchedA[10] + SchedA[11] + SchedA[12] + SchedA[13] + SchedA[14];
  showschedA(15);
 GetLine( "A16", &SchedA[16] );	/* Charity contributions.*/
  showschedA(16);
 GetLine( "A17", &SchedA[17] );	/* Contributions other than cash or check.*/
  showschedA(17);
 GetLine( "A18", &SchedA[18] );	/* Carryover from prior year*/
  showschedA_wMsg(18, "Carryover from prior year" );
 SchedA[19] = SchedA[16] + SchedA[17] + SchedA[18];
  showschedA(19);
 GetLine( "A20", &SchedA[20] );	/* Casualty or theft loss(es).*/
  showschedA(20);
 GetLine( "A21", &SchedA[21] );	/* Unreimbursed employee expenses*/
  showschedA(21);
 GetLine( "A22", &SchedA[22] );	/* Tax preparation fees*/
  showschedA(22);
 GetLine( "A23", &SchedA[23] );	/* Other expenses*/
  showschedA(23);
 SchedA[24] = SchedA[21] + SchedA[22] + SchedA[23];  		showschedA(24);
 SchedA[25] = L[38];						showschedA(25);
 SchedA[26] = 0.02 * SchedA[25];				showschedA(26);
 SchedA[27] = NotLessThanZero( SchedA[24] - SchedA[26] );  	showschedA(27);
 GetLine( "A28", &SchedA[28] );	 /* Other deductions, list on page A-6. */
  showschedA(28);

 switch (status)
  {
     case SINGLE: 				idws_thresh = 254200.0;  break;
     case MARRIED_FILLING_JOINTLY: case WIDOW:  idws_thresh = 305050.0;  break;
     case MARRIED_FILLING_SEPARAT:   		idws_thresh = 152525.0;  break;
     case HEAD_OF_HOUSEHOLD:  			idws_thresh = 279650.0;  break;
     default:  printf("Status %d not handled.\n", status);  exit(1); 
  }

 if (L[38] <= 152525.0)
  {
   fprintf(outfile," Your deduction is not limited.\n");
   SchedA[29] = SchedA[4] + SchedA[9] + SchedA[15] + SchedA[19] + SchedA[20] + SchedA[27] + SchedA[28];
  }
 else
  { /* Itemized Deductions WorkSheet i1040sca.pdf page A-12. */
    idws[1] = SchedA[4] + SchedA[9] + SchedA[15] + SchedA[19] + SchedA[20] + SchedA[27] + SchedA[28];
    fprintf(outfile," Your deduction may be limited (from %8.2f).\n", idws[1] );
    idws[2] = SchedA[4] + SchedA[14] + SchedA[20];
    if (idws[2] >= idws[1])
     SchedA[29] = idws[1];
    else
     {
      idws[3] = idws[1] - idws[2];
      idws[4] = 0.80 * idws[3];
      idws[5] = L[38];
      idws[6] = idws_thresh;
      if (idws[6] < idws[5])
       {
	idws[7] = idws[5] - idws[6];
	idws[8] = 0.03 * idws[7];
	idws[9] = smallerof( idws[4], idws[8] );
	idws[10] = idws[1] - idws[9];
	SchedA[29] = idws[10];
       }
      else
       SchedA[29] = idws[1];
     }
    fprintf(outfile," Itemized Deductions Worksheet:\n");
    for (j=1; j <= 10; j++)
     if (idws[j] != 0.0)
      {
       printf(" IDWS[%d] = %6.2f\n", j, idws[j] );
       fprintf(outfile," IDWS[%d] = %6.2f\n", j, idws[j] );
      }
  }
  showschedA(29);
 L[40] = SchedA[29];
 if (L[40] > 0.0)  itemize = Yes;  else  itemize = No;

 if (L[39] == 0.0)
  {
   S_STD_DEDUC   =  6200.0;
   MFS_STD_DEDUC =  6200.0;
   MFJ_STD_DEDUC = 12400.0;
   HH_STD_DEDUC  =  9100.0;
  }
 else
  { /* Std. Deduction chart for People who were Born Before January 2, 1949, or Were Blind, pg 39. */
    switch ((int)L[39])		/* Does not handle if someone claims you or joint-spouse as dependent. */
     {
      case 1: 
	S_STD_DEDUC   =  7750.0;
	MFJ_STD_DEDUC = 13600.0;
	MFS_STD_DEDUC =  7400.0;
	HH_STD_DEDUC  = 10650.0;
	break;
      case 2: 
	S_STD_DEDUC   =  9300.0;
	MFJ_STD_DEDUC = 14800.0;
	MFS_STD_DEDUC =  8600.0;
	HH_STD_DEDUC  = 12200.0;
	break;
      case 3: 
	MFJ_STD_DEDUC = 16000.0;
	MFS_STD_DEDUC =  9800.0;
	S_STD_DEDUC   =  9300.0;	/* Cannot happen, but set to appease compiler. */
	HH_STD_DEDUC  = 12200.0;	/* .. */
	break;
      case 4: 
	MFJ_STD_DEDUC = 17200.0;
	MFS_STD_DEDUC = 11000.0;
	S_STD_DEDUC   =  9300.0;
	HH_STD_DEDUC  = 12200.0;
	break;
      default:  fprintf(outfile,"Error: L[39] (%g) not equal to 1, 2, 3, or 4.\n", L[39] );
		printf("Error: L[39] (%g) not equal to 1, 2, 3, or 4.\n", L[39] );
		exit(1); 
     }
    fprintf(outfile,"(Assuming no one is claiming your or your joint-spouse as a dependent.)\n");
  }

 if (status==SINGLE) 		/* Compare to std deduction. */
  { 
   if (L[40] < S_STD_DEDUC)
    { L[40] = S_STD_DEDUC;  fprintf(outfile,"Use standard deduction.\n");  itemize = 0; }
  }
 else
 if (status==MARRIED_FILLING_SEPARAT)
  { 
   if (L[40] < MFS_STD_DEDUC)
    { L[40] = MFS_STD_DEDUC;  fprintf(outfile,"Use standard deduction.\n");  itemize = 0; }
  }
 else
 if ((status==MARRIED_FILLING_JOINTLY) || (status==WIDOW))
  { 
   if (L[40] < MFJ_STD_DEDUC)
    { L[40] = MFJ_STD_DEDUC;  fprintf(outfile,"Use standard deduction.\n");  itemize = 0; }
  }
 else
 if (status==HEAD_OF_HOUSEHOLD) 
  { 
   if (L[40] < HH_STD_DEDUC)
    { L[40] = HH_STD_DEDUC;  fprintf(outfile,"Use standard deduction.\n");  itemize = 0; }
  }
 else 
  { printf("Case (Line 40) not handled.\n"); fprintf(outfile,"Case (Line 40) not handled.\n"); exit(1); }
 if (itemize) fprintf(outfile,"Itemizing.\n");

 showline(40);

 L[41] = L[38] - L[40];
 showline(41);

 if (L[38] <= idws_thresh)
  L[42] = 3950.0 * L[6];
 else
  { /* Deduction for Exemptions Worksheet (pg 42) */
    double thresh;
    dedexws[2] = 3950.0 * L[6];
    dedexws[3] = L[38];
    dedexws[4] = idws_thresh;
    dedexws[5] = dedexws[3] - dedexws[4];
    if (status == MARRIED_FILLING_SEPARAT) thresh = 61250.0; else thresh = 122500.0;
    if (dedexws[5] > thresh)
     L[42] = 0.0;
    else
     { int j;
      if (status != MARRIED_FILLING_SEPARAT) 
       j = dedexws[5] / 2500.0 + 0.999999;
      else
       j = dedexws[5] / 1250.0 + 0.999999;
      dedexws[6] = j;
      dedexws[7] = 0.02 * dedexws[6];
      dedexws[8] = dedexws[2] * dedexws[7];
      dedexws[9] = dedexws[2] - dedexws[8];
      L[42] = dedexws[9];
     }
  }
 showline( 42 );


 L[43] = NotLessThanZero( L[41] - L[42] );
 showline_wmsg( 43, "Taxable income" );

 /* Calculate the tax...  on L43 */
 L[44] = TaxRateFunction( L[43], status );

 if (L[43] <= 0.0) 
  { /*exception*/
    printf(" Exception (Sched-D Instructions page 14) - Do not use QDCGT or Sched-D Tax Worksheets.\n");
  } /*exception*/
 else
  { /*no_exception*/
   if ((!Do_SDTW) && (!Do_QDCGTW) && ((L9b > 0.0) || (L[13] > 0.0) || ((SchedD[15] > 0.0) && (SchedD[16] > 0.0)) ))
    Do_QDCGTW = Yes;
   if (Do_QDCGTW)
    {
     fprintf(outfile,"Doing 'Qualified Dividends and Captial Gain tax Worksheet', page 34.\n");
     capgains_qualdividends_worksheets( status, L9b );
    }
   else
   if (Do_SDTW)
   {
    fprintf(outfile,"Doing 'Schedule D Tax Worksheet', page D9.\n");
    sched_D_tax_worksheet( status, L9b );
   }
  } /*no_exception*/

 showline_wmsg( 44, "Tax" );

 GetLine( "L46", &L[46] );	/* Excess advance premium tax credit repayment. Form 8962. */
 GetLine( "L48", &L[48] );	/* Foreign tax credit. */

 L[45] = form6251_AlternativeMinimumTax( itemize );	/* (Depends on L[48].) */
 if (L[45] == 0.0) fprintf(outfile," (Not subject to Alternative Minimum Tax.)\n");
 else fprintf(outfile," (You must pay Alternative Minimum Tax.)\n");
 ShowLineNonZero_wMsg(45, "Alternative Minimum Tax" );

 showline(46);

 L[47] = L[44] + L[45] + L[46];	
 showline(47);

 ShowLineNonZero(48);

 GetLine( "L49", &L[49] );	/* Child / dependent care expense credits. Form 2441. */
 ShowLineNonZero(49);

 GetLine( "L50", &L[50] );	/*  Education credits. Form 8863. */
 ShowLineNonZero(50);

 GetLine( "L51", &L[51] );	/*  Retirement savings contributions credit. Form 8880. */
 ShowLineNonZero(51);

 GetLine( "L52", &L[52] );	/*  Child tax credit (see page 42). */
 ShowLineNonZero(52);

 GetLine( "L53", &L[53] );	/*  Residential energy credits. Form 5695. */
 ShowLineNonZero(53);

 GetLine( "L54", &L[54] );	/*  Other credits. Forms 3800, 8801, ect. */
 ShowLineNonZero(54);

 for (j = 48; j <= 54; j++)
  L[55] = L[55] + L[j];
 showline(55);

 L[56] = NotLessThanZero( L[47] - L[55] );
 showline(56);

 /* Other Taxes section. */

 GetLine( "L57", &L[57] );	/* Self-employment tax. Sched SE */
 ShowLineNonZero(57);

 GetLine( "L58", &L[58] );	/* Unreported social security and Medicare tax from Forms 4137, 8919 */
 ShowLineNonZero(58);

 GetLine( "L59", &L[59] );	/* Additional tax on IRAs, other qualified retirement plan, Form 5329 */
 ShowLineNonZero(59);

 GetLine( "L60a", &L[60] );	/* Household employment taxes. Sched H */
 if (L[60] != 0.0) fprintf(outfile, "L60a = %6.2f\n", L[60] );

 GetLine( "L60b", &L60b );	/* First-time homebuyer credit repayment. Form 5405. */
 if (L60b != 0.0) fprintf(outfile, "L60b = %6.2f\n", L60b );
 L[60] = L[60] + L60b;
 L["60b"] = L60b;

 GetLine( "L61", &L[61] );	/* Health care: individual responsibility. */
 ShowLineNonZero(61);

 GetLine( "L62", &L[62] );	/* Taxes from Forms 8959, 8960, others. */
 ShowLineNonZero(62);

 for (j = 56; j <= 62; j++)
  L[63] = L[63] + L[j];
 showline_wmsg(63,"total tax");


 /* Payments section. */

 GetLineF( "L64", &L[64] );	/* Federal income tax withheld, Forms W-2, 1099 */

 GetLine( "L65", &L[65] );	/* 2014 estimated payments + amnt applied from last year. */
 ShowLineNonZero(65);

 GetLine( "L66a", &L[66] );	/* Earned income credit (EIC) */
 ShowLineNonZero(66);

 GetLine( "L67", &L[67] );	/* Additional child tax credit. Attach Form 8812 */
 ShowLineNonZero(67);

 GetLine( "L68", &L[68] );	/* American opportunity credit. Form 8863, line 8. */
 ShowLineNonZero(68);

 GetLine( "L69", &L[69] );	/* Net premium tax credit. Form 8962. */
 ShowLineNonZero(69);

 GetLine( "L70", &L[70] );	/* Amnt paid in filing extension req. */
 ShowLineNonZero(70);

 GetLine( "L71", &L[71] );	/* Excess soc. sec. + tier 1 RRTA tax withheld */
 ShowLineNonZero(71);

 GetLine( "L72", &L[72] );	/* Credits for federal tax on fuels. Attach form 4136. */
 ShowLineNonZero(72);

 GetLine( "L73", &L[73] );	/* Credits from Form 2439, 4136, 6801, 8885 */
 ShowLineNonZero(73);

 for (j = 64; j <= 73; j++)
  L[74] = L[74] + L[j];
 showline_wmsg(74, "total payments");


 /* Refund or Owe section. */

 if (L[74] > L[63])
  {
   L[75] = L[74] - L[63];
   fprintf(outfile,"L75 = %6.2f  REBATE!!!\n", L[75] );
   fprintf(outfile,"L76a = %6.2f \n", L[75] );
  }
 else 
  {
   L[78] = L[63] - L[74];
   fprintf(outfile,"L78 = %6.2f  YOU OWE !!!\n", L[78] );
   fprintf(outfile,"         (Which is %2.1f%% of the total amount owed.)\n", 100.0 * L[78] / L[63] );
  }
 

 fclose(infile);
 fclose(outfile);

 outfile = fopen(f1040_xfdf_outfname,"w");
 output_xfdf_form_data(outfile, form_us_1040_2014, L); 
 fclose(outfile);

 if (itemize) {
  outfile = fopen(f1040sa_xfdf_outfname,"w");
  output_xfdf_form_data(outfile, form_us_1040_2014_schedule_a, SchedA); 
  fclose(outfile);
 }

 if (output_SchedD) { 
  outfile = fopen(f1040sd_xfdf_outfname,"w");
  output_xfdf_form_data(outfile, form_us_1040_2014_schedule_d, SchedD); 
  fclose(outfile);
 }

 printf("\nListing results from file: %s\n\n", outfname);
 Display_File( outfname );
 return 0;
}

