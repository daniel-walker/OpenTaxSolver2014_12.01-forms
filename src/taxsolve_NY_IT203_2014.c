/************************************************************************/
/* TaxSolve_NY_IT-203_2014.c - NY State Tax form IT-203 for 2014.	*/
/* Copyright (C) 2003-2015 - Aston Roberts, Skeet Monker		*/
/* 									*/
/* Compile:   gcc taxsolve_NY_IT203_2014.c -o taxsolve_NY_IT203_2014	*/
/* Run:	      ./taxsolve_NY_IT203_2014  NY_IT203_2014.txt 		*/
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
/* Aston Roberts 4-8-2003	aston_roberts@yahoo.com			*/
/* Modified for NY 2003 taxes - Skeet Monker				*/
/* Modified for NY 2004 taxes - A Roberts				*/
/* Modified for NY 2005-2014 taxes - Skeet Monker			*/
/************************************************************************/

float thisversion=12.00;

#include "taxsolve_routines.c"
#include "taxsolve_NY_IT203_2014_forms.h"

double A[10], S[100];

#define SINGLE                  1
#define MARRIED_FILLING_JOINTLY 2
#define MARRIED_FILLING_SEPARAT 3
#define HEAD_OF_HOUSEHOLD       4
#define WIDOW                   5
#define Yes 1
#define No  0

int status=0;

char statusnames[10][20]={"0","Single","Married/Joint","Married/Sep","Head_of_House","Widow"};

struct FedReturnData
 {
  double fedline[MAX_LINES], schedA[MAX_LINES], schedD[MAX_LINES];
  int Exception, Itemized;
  double IDWS[MAX_LINES];
 } PrelimFedReturn;


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


int ImportFederalReturnData( char *fedlogfile, struct FedReturnData *fed_data, bool doRound)
{
 FILE *infile;
 char fline[1000], word[1000];
 int linenum;

 for (linenum=0; linenum<MAX_LINES; linenum++) 
  { 
   fed_data->fedline[linenum] = 0.0;
   fed_data->schedA[linenum] = 0.0;
   fed_data->schedD[linenum] = 0.0;
	  fed_data->IDWS[linenum] = 0.0;
  }
 convert_slashes( fedlogfile );
 infile = fopen(fedlogfile, "r");
 if (infile==0)
  {
   printf("Error: Could not open Federal return '%s'\n", fedlogfile);
   fprintf(outfile,"Error: Could not open Federal return '%s'\n", fedlogfile);
   return 0; 
  }
 fed_data->Itemized = 1; /* Set initial default values. */
 read_line(infile,fline);  linenum = 0;
 while (!feof(infile))
  {
   if (strstr(fline,"Use standard deduction.")!=0) fed_data->Itemized = 0;
   next_word(fline, word, " \t=");
   if ((strstr(word,"L")==word) && (strstr(fline," = ")!=0))
    {
     if (strcmp(word,"L9b") != 0)
      {
       if (sscanf(&word[1],"%d",&linenum)!=1)
	{
	 printf("Error: Reading Fed line number '%s%s'\n",word,fline);
	 fprintf(outfile,"Error: Reading Fed line number '%s%s'\n",word,fline);
	}
       next_word(fline, word, " \t=");
       if (sscanf(word,"%lf", &fed_data->fedline[linenum])!=1)
	{
	 printf("Error: Reading Fed line %d '%s%s'\n",linenum,word,fline);
	 fprintf(outfile,"Error: Reading Fed line %d '%s%s'\n",linenum,word,fline);
	}
       if (verbose) printf("FedLin[%d] = %.2f\n", linenum, fed_data->fedline[linenum]);
      }
    }
   else
   if ((strstr(word,"A")==word) && (strstr(word,"AMT")!=word) && (strstr(fline," = ")!=0))
    {
     if (sscanf(&word[1],"%d",&linenum)!=1)
      {
	printf("Error: Reading Fed line number '%s%s'\n",word,fline);
	fprintf(outfile,"Error: Reading Fed line number '%s%s'\n",word,fline);
      }
     next_word(fline, word, " \t=");
     if (sscanf(word,"%lf", &fed_data->schedA[linenum])!=1) 
       {
	printf("Error: Reading Fed schedA %d '%s%s'\n",linenum,word,fline);
	fprintf(outfile, "Error: Reading Fed schedA %d '%s%s'\n",linenum,word,fline);
       }
     if (verbose) printf("FedLin[%d] = %.2f\n", linenum, fed_data->schedA[linenum]);
			if (doRound) fed_data->schedA[linenum] = Round(fed_data->schedA[linenum]);
    }
   else
		if ((strstr(word,"IDWS")==word) && (strstr(fline," = ")!=0))
		  {
			if (sscanf(&word[5],"%d",&linenum)!=1)
			  {
				printf("Error: Reading Fed line number '%s%s'\n",word,fline);
				fprintf(outfile,"Error: Reading Fed line number '%s%s'\n",word,fline);
			  }
			next_word(fline, word, " \t=");
			if (sscanf(word,"%lf", &fed_data->IDWS[linenum])!=1) 
			  {
				printf("Error: Reading Fed IDWS %d '%s%s'\n",linenum,word,fline);
				fprintf(outfile, "Error: Reading Fed IDWS %d '%s%s'\n",linenum,word,fline);
			  }
			if (verbose) printf("FedLin.IDWS[%d] = %.2f\n", linenum, fed_data->IDWS[linenum]);
		  }
		else
   if ((strstr(word,"D")==word) && (strstr(fline," = ")!=0))
    {
     if (sscanf(&word[1],"%d",&linenum)!=1)
      {
	printf("Error: Reading Fed line number '%s%s'\n",word,fline);
	fprintf(outfile,"Error: Reading Fed line number '%s%s'\n",word,fline);
      }
     next_word(fline, word, " \t=");
     if (strcmp(word,"d") == 0)
      { /*Basis,Sale,Gain line. Capture last value.*/
	next_word(fline, word, " \teh=" );
	while (word[0] != '\0')
	 {
	  if (sscanf(word,"%lf", &fed_data->schedD[linenum]) != 1) 
	   fprintf(outfile,"Error: Reading Fed schedD %d '%s %s'\n", linenum, word, fline);
	  next_word(fline, word, " \teh=" );
	 }
      }
     else
     if (sscanf(word,"%lf", &fed_data->schedD[linenum]) != 1) 
      {
       if (strncasecmp(word,"yes",1) == 0) fed_data->schedD[linenum] = 1;
       else
       if (strncasecmp(word,"no",1) == 0) fed_data->schedD[linenum] = 0;
       else
        {
         printf("Error: Reading fed schedD %d '%s%s'\n", linenum, word, fline);
	 fprintf(outfile,"Error: Reading Fed schedD %d '%s%s'\n", linenum, word, fline);
        }
      }
     if (verbose) printf("FedLin[%d] = %.2f\n", linenum, fed_data->schedD[linenum]);
    }
   else
   if (strcmp(word,"Status") == 0)
    {
     next_word(fline, word, " \t=");
     if (strncasecmp(word,"Single",4)==0) status = SINGLE; else
     if (strncasecmp(word,"Married/Joint",13)==0) status = MARRIED_FILLING_JOINTLY; else
     if (strncasecmp(word,"Married/Sep",11)==0) status = MARRIED_FILLING_SEPARAT; else
     if (strncasecmp(word,"Head_of_House",4)==0) status = HEAD_OF_HOUSEHOLD; else
     if (strncasecmp(word,"Widow",4)==0) status = WIDOW;
     else 
      { 
       printf("Error: unrecognized status '%s'. Exiting.\n", word); 
       fprintf(outfile,"Error: unrecognized status '%s'. Exiting.\n", word);
       return 0; 
      }
    }
   read_line(infile,fline);
  }
 fclose(infile);
 return 1;
}


double TaxRateFunction( double income, int status )
{
 double tax;

 switch (status)
  {
   case MARRIED_FILLING_JOINTLY: case WIDOW:
	if (income <=   16700.0) tax = 	         0.04 * income; else
	if (income <=   22950.0) tax =   668.0 + 0.045  * (income - 16700.0); else
	if (income <=   27150.0) tax =   949.0 + 0.0525 * (income - 22950.0); else
	if (income <=   41800.0) tax =  1170.0 + 0.059  * (income - 27150.0); else
	if (income <=  156900.0) tax =  2034.0 + 0.0645 * (income - 41800.0); else
	if (income <=  313850.0) tax =  9458.0 + 0.0665 * (income - 156900.0); else
	if (income <= 2092800.0) tax = 19895.0 + 0.0685 * (income - 313850.0); 
	else			tax = 141753.0 + 0.0882 * (income - 2092800.0);
      break;
   case SINGLE: case MARRIED_FILLING_SEPARAT:
	if (income <=    8300.0) tax =  	  0.04   * income; else
	if (income <=   11450.0) tax =    332.0 + 0.045  * (income - 8300.0); else
	if (income <=   13550.0) tax =    474.0 + 0.0525 * (income - 11450.0); else
	if (income <=   20850.0) tax =    584.0 + 0.059  * (income - 13350.0); else
	if (income <=   78400.0) tax =   1015.0 + 0.0645 * (income - 20850.0); else
	if (income <=  209250.0) tax =   4727.0 + 0.0665 * (income - 78400.0); else
	if (income <= 1046350.0) tax =  13428.0 + 0.0685 * (income - 209250.0); 
	else 		 	 tax =  70770.0 + 0.0882 * (income - 1046350.0);
      break;
   case HEAD_OF_HOUSEHOLD:
	if (income <=   12550.0) tax = 	         0.04 * income; else
	if (income <=   17200.0) tax =   502.0 + 0.045  * (income - 12550.0); else
	if (income <=   20350.0) tax =   711.0 + 0.0525 * (income - 17200.0); else
	if (income <=   31350.0) tax =   877.0 + 0.059  * (income - 20350.0); else
	if (income <=  104600.0) tax =  1526.0 + 0.0645 * (income - 31350.0); else
	if (income <=  261550.0) tax =  6250.0 + 0.0665 * (income - 104600.0); else
	if (income <= 1569550.0) tax = 16687.0 + 0.0685 * (income - 261550.0);
	else			tax = 106285.0 + 0.0882 * (income - 1569550.0);
      break;
   default: printf("Error: Unhandled status\n"); exit(0); break;
  }
 return tax;
}


double TaxRateLookup( double income, int status )
{
 double tax, dx;
 int m;

 if (income < 25.0)  dx = 12.5;  else
 if (income < 50.0)  dx = 25.0;  else  dx = 50.0;

 /* Round and truncate results from tax-function to approximate table lookup. */
 m = income / dx;             /* Round income to nearest $50. */
 income = (double)m * dx + 0.5 * dx;      /* Place into center of a $50 bracket. */
 tax = TaxRateFunction( income, status );

 return (int)(tax + 0.5);
}


double NYcityTaxRateFunction( double income, int status )	/* From page 68. */
{
 double tax, dx;
 int m;

 if (income < 25.0) dx = 12.5; else
 if (income < 50.0) dx = 25.0; else dx = 50.0;

 m = income / dx;             /* Round income to nearest $50. */
 if (income < 65000.0)
  income = m * dx + 0.5 * dx;      /* Place into center of a $50 bracket. */

 if ((status==MARRIED_FILLING_JOINTLY) || (status==WIDOW))
  {
   if (income < 21600.0)  tax = income * 0.02907; else
   if (income < 45000.0)  tax = (income - 21600.00) * 0.03534 + 628.00; else
   if (income < 90000.0)  tax = (income - 45000.00) * 0.03591 + 1455.0; else
   if (income < 500000.0) tax = (income - 90000.00) * 0.03648 + 3071.0; else
   tax = (income - 500000.0) * 0.03876 + 18028.00;
  }
 else
 if ((status==SINGLE) || (status==MARRIED_FILLING_SEPARAT))
  {
   if (income < 12000.0) tax = income * 0.02907; else
   if (income < 25000.0) tax = (income - 12000.00) * 0.03534 + 349.0; else
   if (income < 50000.0) tax = (income - 25000.00) * 0.03591 + 808.0; else
   if (income < 500000.0) tax = (income - 50000.00) * 0.03648 + 1706.0; else
   tax = (income - 500000.0) * 0.03876 + 18122.00;
  }
 else
 if (status==HEAD_OF_HOUSEHOLD) 
  {
   if (income < 14400.00) tax = income * 0.02907; else
   if (income < 30000.00) tax = (income - 14000.00) * 0.03534 + 419.0; else
   if (income < 60000.00) tax = (income - 30000.00) * 0.03591 + 970.0; else
   if (income < 500000.0) tax = (income - 60000.00) * 0.03648 + 2047.0;
   tax = (income - 500000.0) * 0.03876 + 18098.00;
 }
 else {printf("Status not covered.\n");  exit(1);}

 if (income < 65000.0) tax = (int)(tax + 0.5);   /* Round result to whole dollar. */
 return tax;
}


void worksheet1()	/*Tax Computation Worksheet 1 (pg 60) */
{ double ws[100];
  ws[1] = L[32];
  ws[2] = L[37];
  ws[3] = 0.0645 * ws[2];
  if (ws[1] >= 154600.0)
    ws[9] = ws[3];
  else
   {
    ws[4] = TaxRateFunction( ws[2], status );
    ws[5] = ws[3] - ws[4];
    ws[6] = ws[1] - 104600.0;
    /* Divide by 50k and round to forth decimal place. */
    ws[7] = 0.0001 * (double)Round( 10000.0 * (ws[6] / 50000.0) );
    ws[8] = ws[5] * ws[7];
    ws[9] = ws[4] + ws[8];
   }
  L[38] = ws[9];
}


void worksheet2()	/*Tax Computation Worksheet 2 (pg 60) */
{ double ws[100];
  ws[1] = L[32];
  ws[2] = L[37];
  ws[3] = 0.0665 * ws[2];
  if (ws[1] >= 206900.0)
    ws[11] = ws[3];
  else
   {
    ws[4] = TaxRateFunction( ws[2], status );
    ws[5] = ws[3] - ws[4];
    ws[6] = 662.0;
    ws[7] = ws[5] - ws[6];
    ws[8] = ws[1] - 156900.0;
    /* Divide by 50k and round to forth decimal place. */
    ws[9] = 0.0001 * (double)Round( 10000.0 * (ws[8] / 50000.0) );
    ws[10] = ws[7] * ws[9];
    ws[11] = ws[4] + ws[6] + ws[10];
   }
  L[38] = ws[11];
}


void worksheet3()	/*Tax Computation Worksheet 3 (pg 60) */
{ double ws[100];
  ws[1] = L[32];
  ws[2] = L[37];
  ws[3] = 0.0685 * ws[2];
  if (ws[1] >= 363850.0)
   ws[11] = ws[3];
  else
   {
    ws[4] = TaxRateFunction( ws[2], status );
    ws[5] = ws[3] - ws[4];
    ws[6] = 976.0;
    ws[7] = ws[5] - ws[6];
    ws[8] = ws[1] - 313850.0;
    /* Divide by 50k and round to forth decimal place. */
    ws[9] = 0.0001 * (double)Round( 10000.0 * (ws[8] / 50000.0) );
    ws[10] = ws[7] * ws[9];
    ws[11] = ws[4] + ws[6] + ws[10];
    }
   L[38] = ws[11];
  }


void worksheet4()	/*Tax Computation Worksheet 4 (pg 60) */
{ double ws[100];
  ws[1] = L[32];
  ws[2] = L[37];
  ws[3] = 0.0882 * ws[2];
  if (ws[1] >= 2142800.0)
   ws[11] = ws[3];
  else
   {
    ws[4] = TaxRateFunction( ws[2], status );
    ws[5] = ws[3] - ws[4];
    if (ws[2] <= 156900.0)
     ws[6] = 662.0;
    else
    if (ws[2] <= 313850.0)
     ws[6] = 976.0;
    else
     ws[6] = 1604.0;
    ws[7] = ws[5] - ws[6];
    ws[8] = ws[1] - 2092800.0;
    /* Divide by 50k and round to forth decimal place. */
    ws[9] = 0.0001 * (double)Round( 10000.0 * (ws[8] / 50000.0) );
    ws[10] = ws[7] * ws[9];
    ws[11] = ws[4] + ws[6] + ws[10];
    }
   L[38] = ws[11];
  }


void worksheet5()	/*Tax Computation Worksheet 5 (pg 61) */
{ double ws[100];
  ws[1] = L[32];
  ws[2] = L[37];
  ws[3] = 0.0665 * ws[2];
  if (ws[1] >= 154600.0)
    ws[9] = ws[3];
  else
   {
    ws[4] = TaxRateFunction( ws[2], status );
    ws[5] = ws[3] - ws[4];
    ws[6] = ws[1] - 104600.0;
    /* Divide by 50k and round to forth decimal place. */
    ws[7] = 0.0001 * (double)Round( 10000.0 * (ws[6] / 50000.0) );
    ws[8] = ws[5] * ws[7];
    ws[9] = ws[4] + ws[8];
   }
  L[38] = ws[9];
}


void worksheet6()	/*Tax Computation Worksheet 6 (pg 61) */
{ double ws[100];
  ws[1] = L[32];
  ws[2] = L[37];
  ws[3] = 0.0685 * ws[2];
  if (ws[1] >= 259250.0)
    ws[11] = ws[3];
  else
   {
    ws[4] = TaxRateFunction( ws[2], status );
    ws[5] = ws[3] - ws[4];
    ws[6] = 487.0;
    ws[7] = ws[5] - ws[6];
    ws[8] = ws[1] - 209250.0;
    /* Divide by 50k and round to forth decimal place. */
    ws[9] = 0.0001 * (double)Round( 10000.0 * (ws[8] / 50000.0) );
    ws[10] = ws[7] * ws[9];
    ws[11] = ws[4] + ws[6] + ws[10];
   }
  L[38] = ws[11];
}


void worksheet7()	/*Tax Computation Worksheet 7 (pg 52) */
{ double ws[100];
  ws[1] = L[32];
  ws[2] = L[37];
  ws[3] = 0.0882 * ws[2];
  if (ws[1] >= 1096350.0)
   ws[11] = ws[3];
  else
   {
    ws[4] = TaxRateFunction( ws[2], status );
    ws[5] = ws[3] - ws[4];
    if (ws[2] <= 209250.0)
     ws[6] = 487.0;
    else
     ws[6] = 905.0;
    ws[7] = ws[5] - ws[6];
    ws[8] = ws[1] - 1046350.0;
    /* Divide by 50k and round to forth decimal place. */
    ws[9] = 0.0001 * (double)Round( 10000.0 * (ws[8] / 50000.0) );
    ws[10] = ws[7] * ws[9];
    ws[11] = ws[4] + ws[6] + ws[10];
    }
   L[38] = ws[11];
  }


void worksheet8()	/*Tax Computation Worksheet 8 (pg 62) */
{ double ws[100];
  ws[1] = L[32];
  ws[2] = L[37];
  ws[3] = 0.0665 * ws[2];
  if (ws[1] >= 154600.0)
    ws[9] = ws[3];
  else
   {
    ws[4] = TaxRateFunction( ws[2], status );
    ws[5] = ws[3] - ws[4];
    ws[6] = ws[1] - 104600.0;
    /* Divide by 50k and round to forth decimal place. */
    ws[7] = 0.0001 * (double)Round( 10000.0 * (ws[6] / 50000.0) );
    ws[8] = ws[5] * ws[7];
    ws[9] = ws[4] + ws[8];
   }
  L[38] = ws[9];
}


void worksheet9()	/*Tax Computation Worksheet 9 (pg 62) */
{ double ws[100];
  ws[1] = L[32];
  ws[2] = L[37];
  ws[3] = 0.0685 * ws[2];
  if (ws[1] >= 311550.0)
    ws[11] = ws[3];
  else
   {
    ws[4] = TaxRateFunction( ws[2], status );
    ws[5] = ws[3] - ws[4];
    ws[6] = 706.0;
    ws[7] = ws[5] - ws[6];
    ws[8] = ws[1] - 261550.0;
    /* Divide by 50k and round to forth decimal place. */
    ws[9] = 0.0001 * (double)Round( 10000.0 * (ws[8] / 50000.0) );
    ws[10] = ws[7] * ws[9];
    ws[11] = ws[4] + ws[6] + ws[10];
   }
  L[38] = ws[11];
}


void worksheet10()	/*Tax Computation Worksheet 10 (pg 62) */
{ double ws[100];
  ws[1] = L[32];
  ws[2] = L[37];
  ws[3] = 0.0882 * ws[2];
  if (ws[1] >= 1619550.0)
   ws[11] = ws[3];
  else
   {
    ws[4] = TaxRateFunction( ws[2], status );
    ws[5] = ws[3] - ws[4];
    if (ws[2] <= 261550.0)
     ws[6] = 706.0;
    else
     ws[6] = 1229.0;
    ws[7] = ws[5] - ws[6];
    ws[8] = ws[1] - 1569550.0;
    /* Divide by 50k and round to forth decimal place. */
    ws[9] = 0.0001 * (double)Round( 10000.0 * (ws[8] / 50000.0) );
    ws[10] = ws[7] * ws[9];
    ws[11] = ws[4] + ws[6] + ws[10];
    }
   L[38] = ws[11];
  }


void tax_computation_worksheet( int status )
{ /* Worksheets from pages 60-62. Come here when AGI L[32] > $104,600. */
 switch (status)
  {
     case MARRIED_FILLING_JOINTLY:  case WIDOW:
	if (L[32] <= 2092800.0)
	 {
	   if (L[37] <= 156900.0)
	    worksheet1();
	   else
	   if ((L[32] > 156900.0) && (L[37] <= 313850.0))
	    worksheet2();
	   else
	   if ((L[32] > 313850.0) && (L[37] > 313850.0))
	    worksheet3();
	   else
	    worksheet4();
	 }
	else
	 worksheet4();
	break;
     case SINGLE:  case MARRIED_FILLING_SEPARAT:
	if (L[32] <= 1046350.0)
	 {
	   if (L[37] <= 209250.0)
	    worksheet5();
	   else
	   if ((L[32] > 209250.0) && (L[37] > 209250.0))
	    worksheet6();
	   else
	    worksheet7();
	 }
	else
	 worksheet7();
	break;
     case HEAD_OF_HOUSEHOLD:
	if (L[32] <= 1569550.0)
	 {
	   if (L[37] <= 261550.0)
	    worksheet8();
	   else
	   if ((L[32] > 261550.0) && (L[37] > 261550.0))
	    worksheet9();
	   else
	    worksheet10();
	 }
	else
	 worksheet10();
	break;
     default: printf("Case not handled.\n");  fprintf(outfile,"Case not handled.\n"); exit(1);
  }
}


double WorksheetItemizedDed(double LTCfrac)
{
  // Worksheet2
  double dedws2[12];
  dedws2[1]	= PrelimFedReturn.IDWS[9];
  dedws2[2]	= PrelimFedReturn.IDWS[3];
  dedws2[3]	= 0.0001 * (double)Round(10000.0 * (dedws2[1]/dedws2[2]));
  dedws2[4]	= PrelimFedReturn.schedA[5] + PrelimFedReturn.schedA[8];
  dedws2[5]	= 0.0;	// Assume no subtactions adjustments B and C and E
  dedws2[6]	= dedws2[4]	+ dedws2[5];
  dedws2[7]	= dedws2[3]	* dedws2[6];
  dedws2[8]	= dedws2[6]	- dedws2[7];
  dedws2[9]	= 0.0;	// Assume no subtraction adjustments D and E
  dedws2[10]= LTCfrac * PrelimFedReturn.schedA[4];
  dedws2[11]= dedws2[8] + dedws2[9] + dedws2[10];
  int	ii;
  if (verbose) for(ii=1; ii <= 11; ii++) fprintf(outfile, "dedws2[%d] = %6.2f\n", ii, dedws2[ii]);
  return dedws2[11];
}

/*----------------------------------------------------------------------------*/

int main( int argc, char *argv[] )
{
 int j, k, argk;
 char word[1000], outfname[1000], it203_xfdf_outfname[1000], it203d_xfdf_outfname[1000];
 time_t now;
 int Dependent, Exemptions, nyc_resident, foreign_account;
 int nyc_resident_months = 0;
 int spouse_nyc_resident_months = 0;
 double itemized_ded, std_ded=0.0, LTC=0, AddAdj=0.0, CollegeDed=0.0;
 Lmap ded_sched;
 char prelim_1040_outfilename[5000];

 /* Intercept any command-line arguments. */
 printf("NY-IT203 - 2014 - v%3.1f\n", thisversion);
 argk = 1;  k=1;
 while (argk < argc)
 {
  if (strcmp(argv[argk],"-verbose")==0)  verbose = 1;
  else
  if (k==1)
   {
    infile = fopen(argv[argk],"r");
    if (infile==0) {printf("ERROR: Parameter file '%s' could not be opened.\n", argv[argk]);  exit(1);}
    k = 2;
    /* Base name of output file on input file. */
    strcpy(outfname,argv[argk]);
    strcpy(it203_xfdf_outfname,argv[argk]);
    strcpy(it203d_xfdf_outfname,argv[argk]);
    j = strlen(outfname)-1;
    while ((j>=0) && (outfname[j]!='.')) j--;
    if (j<0) {
     strcat(outfname,"_out.txt");
     strcat(it203_xfdf_outfname,"_it203.xfdf");
     strcat(it203d_xfdf_outfname,"_it203d.xfdf");
    } else {
     strcpy(&(outfname[j]),"_out.txt");
     strcpy(&(it203_xfdf_outfname[j]),"_it203.xfdf");
     strcpy(&(it203d_xfdf_outfname[j]),"_it203d.xfdf");
    }
    outfile = fopen(outfname,"w");
    if (outfile==0) {printf("ERROR: Output file '%s' could not be opened.\n", outfname);  exit(1);}
    printf("Writing results to file:  %s\n", outfname);
   }
  else {printf("Unknown command-line parameter '%s'\n", argv[argk]);  exit(1);}
  argk = argk + 1;
 }

 if (infile==0) {printf("Error: No input file on command line.\n"); exit(1);}

 /* Pre-initialize all lines to zeros. */
 for (j=0; j<MAX_LINES; j++) 
  {
   L[j] = 0.0;
   StateL[j] = 0.0;
   ded_sched[j] = 0.0;
  }

 /* Accept parameters from input file. */
 /* Expect  NY IT200 form lines, something like:
	Title:  NY IT-200 1999 Return
	L1	;	{Wages}
	L2	;	{Interest}
	L3	;	{Dividends}
	...
*/


 /* Accept Form's "Title" line, and put out with date-stamp for records. */
 read_line( infile, word );
 now = time(0);
 fprintf(outfile,"\n%s		%s\n", word, ctime( &now ));

 get_personal_details(infile);

 get_parameter( infile, 's', word, "FileName" );      /* Preliminary Return Output File-name. */
 get_word(infile, prelim_1040_outfilename );
 if (ImportFederalReturnData( prelim_1040_outfilename, &PrelimFedReturn, true ) == 0)
  {
   fclose(infile);
   fclose(outfile);
   Display_File( outfname );
   exit(1);
  }

 /* get_parameter(infile, kind, x, emssg ) */
 // get_parameter( infile, 's', word, "Status" );
 // get_parameter( infile, 'l', word, "Status?");
 // if (strncasecmp(word,"Single",4)==0) status = SINGLE; else
 // if (strncasecmp(word,"Married/Joint",13)==0) status = MARRIED_FILLING_JOINTLY; else
 // if (strncasecmp(word,"Married/Sep",11)==0) status = MARRIED_FILLING_SEPARAT; else
 // if (strncasecmp(word,"Head_of_House",4)==0) status = HEAD_OF_HOUSEHOLD; else
 // if (strncasecmp(word,"Widow",4)==0) status = WIDOW;
 // else { printf("Error: unrecognized status '%s'. Exiting.\n", word); exit(1); }

 if (PrelimFedReturn.Itemized) L["itemized"] = 1;
 else L["did_not_itemize"] = 1;
if (PrelimFedReturn.Itemized==1) fprintf(outfile," Check box B = Yes\n");  else fprintf(outfile," Check box B = No\n");
 
 switch (status)
 {
  case SINGLE: L["Single"] = 1; break;
  case MARRIED_FILLING_SEPARAT: L["MFS"] = 1; break;
  case HEAD_OF_HOUSEHOLD: L["HOH"] = 1;; break;
  case MARRIED_FILLING_JOINTLY: L["MFJ"] = 1; break;
  case WIDOW: L["QW"] = 1; break;
 }
 fprintf(outfile,"Status = %s (%d)\n", statusnames[status], status);

 get_parameter( infile, 's', word, "Dependent" );
 get_parameter( infile, 'b', &Dependent, "Dependent?"); 
 if (Dependent==1) {
  fprintf(outfile," Check box C = Yes\n");
  L["dependent"] = 1;
 } else { 
  fprintf(outfile," Check box C = No\n");
  L["not_dependent"] = 1;
 }

 get_parameter( infile, 's', word, "Foreign_Account" );
 get_parameter( infile, 'b', &foreign_account, "Foreign_account?"); 
 if (foreign_account==1) {
  fprintf(outfile," Check box D = Yes // Because the PDF autofilling DOES NOT WORK FOR THIS FIELD\n");
  L["foreign_account"] = 1;
 } else { 
  fprintf(outfile," Check box D = No\n");
  L["not_foreign_account"] = 1;
 }

 get_parameter( infile, 's', word, "NYC_Months" );
 get_parameters( infile, 'i', &nyc_resident_months, "NYC_Months ?"); 
 fprintf(outfile, "NYC resident months = %d\n", nyc_resident_months);
 L["nyc_months"]	= nyc_resident_months;

 get_parameter( infile, 's', word, "Spouse_NYC_Months" );
 get_parameters( infile, 'i', &spouse_nyc_resident_months, "Spouse NYC_Months ?"); 
 fprintf(outfile, "Spouse NYC resident months = %d\n", spouse_nyc_resident_months);
 fprintf(outfile, "\n");
 L["spouse_nyc_months"]	= spouse_nyc_resident_months;

 if ((nyc_resident_months > 0) || (spouse_nyc_resident_months > 0))
   L["nys_living_quarters"]	= 1;
 else 
   L["not_nys_living_quarters"]	= 1;

 // GetLineF( "L1", &L[1] );	/* Wages. */
 L[1] = PrelimFedReturn.fedline[7];
 showline(1);
 GetLineRoundedF_defaulting("L1.ny", &StateL[1], "FED", L[1]);
 fprintf(outfile, "L1.us = %6.2f    L1.ny = %6.2f\n", L[1], StateL[1]);

 // GetLineF( "L2", &L[2] );	/* Taxable Interest. */
 L[2] = PrelimFedReturn.fedline[8];
 showline(2);
  GetLineRoundedF_defaulting( "L2.ny", &StateL[2], "FED", L[2]);
  fprintf(outfile, "L2.us = %6.2f    L2.ny = %6.2f\n", L[2], StateL[2]);

 // GetLineF( "L3", &L[3] );	/* Ordinary Dividends. */
 L[3] = PrelimFedReturn.fedline[9];
 showline(3);
 GetLineRoundedF_defaulting( "L3.ny", &StateL[3], "FED", L[3] );
  fprintf(outfile, "L3.us = %6.2f    L3.ny = %6.2f\n", L[3], StateL[3]);

 // GetLineF( "L4", &L[4] );	/* Taxable refunds, credits, offsets */
 L[4] = PrelimFedReturn.fedline[10];
 showline(4);
  GetLineRoundedF_defaulting( "L4.ny", &StateL[4], "FED", L[4] );
  fprintf(outfile, "L4.us = %6.2f    L4.ny = %6.2f\n", L[4], StateL[4]);

 // GetLineF( "L5", &L[5] );	/* Alimony received */
 L[5] = PrelimFedReturn.fedline[11];
 showline(5);
 GetLineRoundedF_defaulting( "L5.ny", &StateL[5], "FED", L[5] );
  fprintf(outfile, "L5.us = %6.2f    L5.ny = %6.2f\n", L[5], StateL[5]);

 // GetLineF( "L6", &L[6] );	/* Business income/loss (fed sched C) */
 L[6] = PrelimFedReturn.fedline[12];
 showline(6);
 GetLineRoundedF_defaulting( "L6.ny", &StateL[6], "FED", L[6] );
  fprintf(outfile, "L6.us = %6.2f    L6.ny = %6.2f\n", L[6], StateL[6]);

 // GetLineF( "L7", &L[7] );	/* Capital gains/losses (fed sched D) */
 L[7] = PrelimFedReturn.fedline[13];
 showline(7);
  GetLineRoundedF_defaulting( "L7.ny", &StateL[7] , "FED", L[7]);
  fprintf(outfile, "L7.us = %6.2f    L7.ny = %6.2f\n", L[7], StateL[7]);

 // GetLineF( "L8", &L[8] );	/* Other gains/losses (fed form 4794) */
 L[8] = PrelimFedReturn.fedline[14];
 showline(8);
  GetLineRoundedF_defaulting( "L8.ny", &StateL[8], "FED", L[8]);
  fprintf(outfile, "L8.us = %6.2f    L8.ny = %6.2f\n", L[8], StateL[8]);

 // GetLineF( "L9", &L[9] );	/* Taxable IRA distributions */
 L[9] = PrelimFedReturn.fedline[15];
 showline(9);
  GetLineRoundedF_defaulting( "L9.ny", &StateL[9], "FED", L[9]);
  fprintf(outfile, "L9.us = %6.2f    L9.ny = %6.2f\n", L[9], StateL[9]);

 // GetLineF( "L10", &L[10] );	/* Taxable pension/annuity amounts  */
 L[10] = PrelimFedReturn.fedline[16];
 showline(10);
  GetLineRoundedF_defaulting( "L10.ny", &StateL[10], "FED", L[10] );
  fprintf(outfile, "L10.us = %6.2f    L10.ny = %6.2f\n", L[10], StateL[10]);

 // GetLineF( "L11", &L[11] );	/* Rental, royalties, partnership, S corp, (fed sched E) */
 L[11] = PrelimFedReturn.fedline[17];
 showline(11);
  GetLineRoundedF_defaulting( "L11.ny", &StateL[11], "FED", L[11]);
  fprintf(outfile, "L11.us = %6.2f    L11.ny = %6.2f\n", L[11], StateL[11]);

  GetLineF( "L12", &L[12] );	/* Rental real estate included in L[11] (federal amount) */
  showline(12);

 // GetLineF( "L13", &L[13] );	/* Farm income (fed sched F) */
 L[13] = PrelimFedReturn.fedline[18];
 showline(13);
  GetLineRoundedF_defaulting( "L13.ny", &StateL[13], "FED", L[13] );
  fprintf(outfile, "L13.us = %6.2f    L13.ny = %6.2f\n", L[13], StateL[13]);

 // GetLineF( "L14", &L[14] );	/* Unemployment compensation */
 L[14] = PrelimFedReturn.fedline[19];
 showline(14);
  GetLineRoundedF_defaulting( "L14.ny", &StateL[14], "FED", L[14] );
  fprintf(outfile, "L14.us = %6.2f    L14.ny = %6.2f\n", L[14], StateL[14]);

 // GetLineF( "L15", &L[15] );	/* Taxable Social Sec. benefits */
 L[15] = PrelimFedReturn.fedline[20];
 showline(15);
  GetLineRoundedF_defaulting( "L15.ny", &StateL[15], "FED", L[15] );
  fprintf(outfile, "L15.us = %6.2f    L15.ny = %6.2f\n", L[15], StateL[15]);

L[26] = L[15];

 // GetLineF( "L16", &L[16] );	/* Other income (pg. 22) */
 L[16] = PrelimFedReturn.fedline[21];
 showline(16);
  GetLineRoundedF_defaulting( "L16.ny", &StateL[16], "FED", L[16] );
  fprintf(outfile, "L16.us = %6.2f    L16.ny = %6.2f\n", L[16], StateL[16]);

 for (j = 1; j <= 11; j++)
   {  L[17] = L[17] + L[j];  StateL[17] = StateL[17] + StateL[j];}
 for (j=13; j <= 16; j++)
   { L[17] = L[17] + L[j];  StateL[17] = StateL[17] + StateL[j];}
  fprintf(outfile, "L17.us = %6.2f    L17.ny = %6.2f\n", L[17], StateL[17]);

 if (absolutev( L[17] - PrelimFedReturn.fedline[22]) > 0.1)
  {
   printf(" Warning: L[17] = %6.2f, while Fed-line[22] = %6.2f\n", L[17], PrelimFedReturn.fedline[22] );
   fprintf(outfile," Warning: L[17] = %6.2f, while Fed-line[22] = %6.2f\n", L[17], PrelimFedReturn.fedline[22] );
  }

 // GetLineF( "L18", &L[18] );	/* Total federal adjustments to income (pg 22) */
 L[18] = PrelimFedReturn.fedline[36];
 showline(18);
  GetLineRoundedF_defaulting( "L18.ny", &StateL[18], "FED", L[18] );
  fprintf(outfile, "L18.us = %6.2f    L18.ny = %6.2f\n", L[18], StateL[18]);

 L[19] = L[17] - L[18];
 StateL[19] = StateL[17] - StateL[18];
 fprintf(outfile, "Federal adjusted gross income : L19.us = %6.2f    L19.ny = %6.2f\n", L[19], StateL[19]);
 if (absolutev(L[19] - PrelimFedReturn.fedline[38]) > 0.1)
  {
   printf(" Warning: L[19] = %6.2f, while Fed-line[38] = %6.2f\n", L[19], PrelimFedReturn.fedline[38] );
   fprintf(outfile," Warning: L[19] = %6.2f, while Fed-line[38] = %6.2f\n", L[19], PrelimFedReturn.fedline[38] );
  } 

 // GetLineF( "L20", &L[20] );	/* Interest income from non-NY state or local bonds */
  GetLineRounded( "L20.us", &L[20] );  /* Interest income from non-NY state or local bonds */
 GetLineRounded( "L20.ny", &StateL[20] );	/* Interest income from non-NY state or local bonds */
 fprintf(outfile, "L20.us = %6.2f    L20.ny = %6.2f\n", L[20], StateL[20]);

 // GetLineF( "L21", &L[21] );	/* Public employee retirement contributions (pg 15) */
 GetLineRounded( "L21.us", &L[21] );	/* Public employee retirement contributions */
 GetLineRounded( "L21.ny", &StateL[21] );	/* Public employee retirement contributions */
 fprintf(outfile, "L21.us = %6.2f    L21.ny = %6.2f\n", L[21], StateL[21]);

 //  GetLineF( "L22", &L[22] );	/* Other (IT-225 pg 9) */
 GetLineRounded( "L22.us", &L[22] ); /* Other (IT-225 pg 9) */
 GetLineRounded( "L22.ny", &L[22] ); /* Other (IT-225 pg 9) */
 fprintf(outfile, "L22.us = %6.2f    L22.ny = %6.2f\n", L[22], StateL[22]);

 for (j = 19; j <= 22; j++) { L[23] = L[23] + L[j]; StateL[23] = StateL[23] + StateL[j]; }
 fprintf(outfile, "L23.us = %6.2f    L23.ny = %6.2f\n", L[23], StateL[23]);

 L[24] = L[4];			/* Taxable refunds, credits, offsets */
 StateL[24] = StateL[4];
 fprintf(outfile, "L24.us = %6.2f    L24.ny = %6.2f\n", L[24], StateL[24]);

 // GetLineF( "L26", &L[26] );	/* Pensions of NYS and local governments and the federal government (see page 16)  */
 GetLineRounded( "L25.us", &L[25] );	/* Pensions of NYS and local governments and the federal government (see page 24)  */
 GetLineRounded( "L25.ny", &StateL[25] );
 fprintf(outfile, "L25.us = %6.2f    L25.ny = %6.2f\n", L[25], StateL[25]);

 L[26] = L[15];			/* Taxable amount of social security benefits */
  StateL[26] = StateL[15];
  fprintf(outfile, "L26.us = %6.2f    L26.ny = %6.2f\n", L[26], StateL[26]);

  // GetLineF( "L28", &L[28] );	/* Interest income on U.S. government bonds */
  GetLineRounded( "L27.us", &L[27] );	/* Interest income on U.S. government bonds */
  GetLineRounded( "L27.ny", &StateL[27] );
  fprintf(outfile, "L27.us = %6.2f    L27.ny = %6.2f\n", L[27], StateL[27]);

 GetLineRounded( "L28.us", &L[28] );	/* Pension and annuity income exclusion  */
 GetLineRounded( "L28.ny", &StateL[28] );
 if (L[28] > 20000.0)
  {
   L[28] = 20000.0;
   fprintf(outfile, "L28.us = %6.2f (Limited to 20,000)", L[28]);
  }
 else
   {
   fprintf(outfile, "L28.us = %6.2f ", L[28]);
   }
 if (StateL[28] > 20000.0)
  {
   StateL[28] = 20000.0;
   fprintf(outfile, "L28.ny (Limited to 20,000) = %6.2f\n", StateL[28]);
  }
 else
   {
	 fprintf(outfile, "L28.ny = %6.2f\n", StateL[28]);
   }

 GetLineRounded( "L29.us", &L[29] );	/* Other (see page 31) */
 GetLineRounded( "L29.ny", &StateL[29] );
 fprintf(outfile, "L29.us = %6.2f    L29.ny = %6.2f\n", L[29], StateL[29]);

 for (j=24; j<=29; j++) { L[30] = L[30] + L[j]; StateL[30] = StateL[30] + StateL[j]; }
 fprintf(outfile, "L30.us = %6.2f    L30.ny = %6.2f\n", L[30], StateL[30]);

 L[31] = L[23] - L[30];
 StateL[31] = StateL[23] - StateL[30];
 showline_wmsg(31,"New York adjusted gross income (AGI)");
  showline_wmsg_State(31,"New York (State) adjusted gross income (AGI)");

  L[32]	= L[31];
  showline(32);

 /* NYS Itemized Deductions Schedule, pg. 26 */
 GetLineRounded( "LTcare%", &LTC );
 GetLineRounded( "AddAdj", &AddAdj );
 GetLineRounded( "CollegeDed", &CollegeDed );
 ded_sched[1] = Round(PrelimFedReturn.schedA[4]);  fprintf(outfile, "ded_sched[1] = %6.2f\n", ded_sched[1]); //med/dental           
 ded_sched[2] = Round(PrelimFedReturn.schedA[9]);  fprintf(outfile, "ded_sched[2] = %6.2f\n", ded_sched[2]);    //taxes paid        
 ded_sched[3] = Round(PrelimFedReturn.schedA[15]); fprintf(outfile, "ded_sched[3] = %6.2f\n", ded_sched[3]);   // interest paid    
 ded_sched[4] = Round(PrelimFedReturn.schedA[19]); fprintf(outfile, "ded_sched[4] = %6.2f\n", ded_sched[4]);   // charity gifts    
 ded_sched[5] = Round(PrelimFedReturn.schedA[20]); fprintf(outfile, "ded_sched[5] = %6.2f\n", ded_sched[5]);   // casualty/theft loss
 ded_sched[6] = Round(PrelimFedReturn.schedA[27]); fprintf(outfile, "ded_sched[6] = %6.2f\n", ded_sched[6]);   // job expenses     
 ded_sched[7] = Round(PrelimFedReturn.schedA[28]); fprintf(outfile, "ded_sched[7] = %6.2f\n", ded_sched[7]);   // misc ded         
 ded_sched[8] = Round(PrelimFedReturn.schedA[29]); fprintf(outfile, "ded_sched[8] = %6.2f\n", ded_sched[8]);   // fed sched A total

  bool doWorksheet2	= false;
  switch (status)
	{
	case SINGLE:					if (L[19] >= 254200)  doWorksheet2 = true; break;
	case MARRIED_FILLING_SEPARAT:	if (L[19] >= 152525)  doWorksheet2 = true; break;
	case HEAD_OF_HOUSEHOLD:			if (L[19] >= 279650)  doWorksheet2 = true; break;
	case MARRIED_FILLING_JOINTLY:	if (L[19] >= 305050)  doWorksheet2 = true; break;
	}
  if (doWorksheet2)	ded_sched[9] = Round(WorksheetItemizedDed(LTC));
  else ded_sched[9] = PrelimFedReturn.schedA[5] + PrelimFedReturn.schedA[8] 
		+ LTC * PrelimFedReturn.schedA[4];
  fprintf(outfile, "ded_sched[9] = %0.2f\n", ded_sched[9]); 
 ded_sched[10] = ded_sched[8] - ded_sched[9];	 fprintf(outfile, "ded_sched[10] = %0.2f\n", ded_sched[10]); 
 ded_sched[11] = CollegeDed; fprintf(outfile, "ded_sched[11] = %6.2f\n", ded_sched[11]); 
 ded_sched[12] = AddAdj; fprintf(outfile, "ded_sched[12] = %6.2f\n", ded_sched[12]); 	// addition adj
 ded_sched[13] = ded_sched[10] + ded_sched[11] + ded_sched[12]; fprintf(outfile, "ded_sched[13] = %6.2f\n", ded_sched[13]); 
 // itemized deduction worksheets page 42
 if (L[32] <= 100000.0)
  ded_sched[14] = 0.0;
 else
  { /*L32_morethan_100000*/
   double ws[50];
   if (L[32] <= 475000.0)
    { /* Worksheet 3 pg 42 */
      ws[1] = L[32];	fprintf(outfile,"	itemized deduction worksheet3-L1 = %6.2f\n", ws[1]);
      switch (status)
       {
        case SINGLE:  case MARRIED_FILLING_SEPARAT: ws[2] = 100000.0; break;
        case HEAD_OF_HOUSEHOLD:                     ws[2] = 150000.0; break;
        case MARRIED_FILLING_JOINTLY:  case WIDOW:  ws[2] = 200000.0; break;
	default: ws[2] = 0.0;
       }
	  ws[3] = ws[1] - ws[2];
	   if (ws[3] < 0.0)
	ded_sched[14] = 0.0;
      else
       {
	ws[4] = smallerof( ws[3], 50000.0 );
	ws[5] = 0.0001 * Round( 10000.0 * (ws[4] / 50000) );
	ws[6] = 0.25 * ded_sched[13];
	ws[7] = ws[5] * ws[6];
	ded_sched[14] = ws[7];
       }
    }
   else
   if (L[32] <= 525000.0)
    { /* Worksheet 4 pg 42 */
      ws[1] = L[32] - 475000.0;
      ws[2] = 0.0001 * Round( 10000.0 * (ws[1] / 50000) );
      ws[3] = 0.25 * ded_sched[13];
      ws[4] = ws[2] * ws[3];
      ws[5] = ws[3] + ws[4];
      ded_sched[14] = ws[5];  
    }
   else
   if (L[32] <= 1000000.0)
    ded_sched[14] = 0.5 * ded_sched[13];
   else
   if (L[32] <= 10000000.0)
    { /* Worksheet 5 pg 42 */
      ws[1] = ded_sched[13];
      ws[2] = 0.5 * ded_sched[4];
      ws[3] = ws[1] - ws[2];
      ded_sched[14] = ws[3];  
    }
   else
    { /* Worksheet 6 pg 40 */
      ws[1] = ded_sched[13];
      ws[2] = 0.25 * ded_sched[4];
      ws[3] = ws[1] - ws[2];
      ded_sched[14] = ws[3];  
    } /*L32_morethan_10000000*/
   }
 ded_sched[14]	= Round(ded_sched[14]); fprintf(outfile, "ded_sched[14] = %6.2f\n", ded_sched[14]); 
 ded_sched[15] = ded_sched[13] - ded_sched[14]; fprintf(outfile, "ded_sched[15] = %6.2f\n", ded_sched[15]); 
 itemized_ded = ded_sched[15];
 // GetLine( "Itemized_Deduction", &itemized_ded );     /* Form IT-203-ATT or 0.0 if std ded. */

 switch (status)	/* Determine the Std. Deduction. Pg. 18. */
  {
   case SINGLE: if (Dependent)   std_ded = 3100.0; 
		else 		 std_ded = 7800.0; break;
   case MARRIED_FILLING_JOINTLY: std_ded = 15650.0; break;
   case MARRIED_FILLING_SEPARAT: std_ded =  7800.0; break;
   case HEAD_OF_HOUSEHOLD: 	 std_ded = 10950.0; break;
   case WIDOW: 			 std_ded = 15650.0; break;
  }

 if (std_ded > itemized_ded) 
  {
   L[33] = std_ded; 
   L["ny_standard"] = 1;
   showline_wmsg(33,"(Mark Std-deduction)");
  }
 else
  {
   L[33] = itemized_ded;
   L["ny_itemized"] = 1;
   showline_wmsg(33,"(Mark Itemized-deduction)");
  }

 L[34] = L[32] - L[33];
 if (L[34] < 0.0) L[34] = 0.0;
 else showline(34);

 get_parameter( infile, 's', word, "L35" );	/* Number of Dependent Exemptions (Pg 26) */
 get_parameters( infile, 'i', &k, "L35" );
 L[35] = 1000.0 * (double)k;
 showline(35);
 
 L[36] = L[34] - L[35];
 if (L[36] < 0.0)
   L[36] = 0.0;
 showline_wmsg(36,"taxable income");
 L[37] = L[36];
 showline(37);

 if (L[32] <= 104600.0)
   L[38] = TaxRateLookup( L[37], status );
 else
   tax_computation_worksheet( status );
 L[38] = Round(L[38]);
 showline(38);

 /* Household credit. */
 get_parameter( infile, 's', word, "Exemptions" );	/* NY dependent exemptions, Pg 19. */
 get_parameter( infile, 'i', &Exemptions, "Exemptions" );
 if (Dependent)
  L[39] = 0.0;
 else	/* From tables starting on page 25. */
 if (status==SINGLE)
  {
   if (L[19] <  5000.0) L[39] = 75.0; else
   if (L[19] <  6000.0) L[39] = 60.0; else
   if (L[19] <  7000.0) L[39] = 50.0; else
   if (L[19] < 20000.0) L[39] = 45.0; else
   if (L[19] < 25000.0) L[39] = 40.0; else
   if (L[19] < 28000.0) L[39] = 20.0; else  L[39] = 0.0;
  }
 else	/* Status = MARRIED_FILLING_JOINTLY, MARRIED_FILLING_SEPARAT, Head_of_house, Widow */
  if (status!=MARRIED_FILLING_SEPARAT)
   {
    if (L[19] <  5000.0) L[39] = 90.0 + 15.0 * (Exemptions-1); else
    if (L[19] <  6000.0) L[39] = 75.0 + 15.0 * (Exemptions-1); else
    if (L[19] <  7000.0) L[39] = 65.0 + 15.0 * (Exemptions-1); else
    if (L[19] < 20000.0) L[39] = 60.0 + 15.0 * (Exemptions-1); else
    if (L[19] < 22000.0) L[39] = 60.0 + 10.0 * (Exemptions-1); else
    if (L[19] < 25000.0) L[39] = 50.0 + 10.0 * (Exemptions-1); else
    if (L[19] < 28000.0) L[39] = 40.0 +  5.0 * (Exemptions-1); else  
    if (L[19] < 32000.0) L[39] = 20.0 +  5.0 * (Exemptions-1); else  L[39] = 0.0;
   }
  else
   {
    if (L[19] <  5000.0) L[39] = 45.0 + 8.0 * (Exemptions-1); else
    if (L[19] <  6000.0) L[39] = 37.5 + 8.0 * (Exemptions-1); else
    if (L[19] <  7000.0) L[39] = 32.5 + 8.0 * (Exemptions-1); else
    if (L[19] < 20000.0) L[39] = 30.0 + 8.0 * (Exemptions-1); else
    if (L[19] < 22000.0) L[39] = 30.0 + 5.0 * (Exemptions-1); else
    if (L[19] < 25000.0) L[39] = 25.0 + 5.0 * (Exemptions-1); else
    if (L[19] < 28000.0) L[39] = 20.0 + 3.0 * (Exemptions-1); else  
    if (L[19] < 32000.0) L[39] = 10.0 + 3.0 * (Exemptions-1); else  L[39] = 0.0;
   }
 L[39]	= Round(L[39]);
 showline_wmsg(39,"NY state household credit");	/* NY state household credit, (pg 29). */

 L[40] = L[38] - L[39];
 if(L[40] < 0.0) L[40] = 0.0; 
 showline(40);

 GetLineRoundedF( "L41", &L[41] );	/* Child and Dependent care credit, pg 28 */

 L[42] = L[40] - L[41];
 if (L[42] < 0.0) L[42] = 0.0;
 else showline(42);

 GetLineRoundedF( "L43", &L[43] );	/* NYS Earned income credit. Pg 35 */

 L[44] = L[42] - L[43];	/* Base Tax */
 if (L[44] < 0.0) L[44] = 0.0;
 else showline(44);

  L[45] = 0.01 * Round(10000.0 * (StateL[31]/L[31]));		/* New York Income Percentage */
  showline_format(45, "%.3f");					/* show an extra decimal beyond required 4, to reassure reader */

  L[46]	= Round(0.01 * L[45] * L[44]);	/* Allocated New York State Tax */
  showline(46);

 GetLineRoundedF( "L47", &L[47] );	/* New York State nonrefundable credits (from Form IT-203-ATT, line 8 */

  L[48] = L[46] - L[47];
  if (L[48] < 0.0) L[48] = 0.0;
  else showline(48);

  GetLineRoundedF( "L49", &L[49] );	/* Net Other New York State taxes, Form IT-203-ATT, line 33 */

  L[50] = L[48] + L[49];
  showline_wmsg(50,"Total New York State taxes");

 get_parameter( infile, 's', word, "NYC_Resident" );
 get_parameters( infile, 'b', &nyc_resident, "NYC_Resident (yes/no) ?"); 

 GetLine( "L51", &L[51] );	/* Part-year New York City resident tax, attach Form IT-360.1 */
 GetLine( "L52", &L[52] );	/* Part-year resident nonrefundable NYC child and dependent care credit */
 L["52a"]	= L[51] + L[52];		fprintf(outfile,"L52a = %6.2f\n", L["52a"] );
 GetLine( "L53", &L[53] );	/* Yonkers nonresident earnings tax, attach Form Y-203 */
 GetLine( "L54", &L[54] );	/* Part-year Yonkers resident income tax surcharge, attach Form IT-360.1 */

 L[55] = L["52a"] + L[53] + L[54];
   showline_wmsg(55,"Total NYC and Yonkers");

  GetLineRoundedF( "L56", &L[56] );	/* Sales or use tax, pg 29. */
  showline_wmsg(56,"Sales or use tax");
  GetLineRoundedF( "L57", &L[57] );	/* Voluntary Gift contibutions (pgs 30). */
  showline_wmsg(57,"Total voluntary contributions");

 L[58] = L[50] + L[55] + L[56] + L[57];
   showline_wmsg(58,"Total NYS, NYC, Yonkers, Sales and Use taxes, and Voluntary contributions");

 L[59] = L[58];
 showline(59);

 // Part-year New York City school tax credit
 if (nyc_resident)
  { /*NYC*/
    if (Dependent) L[60] = 0.0;
    else
    if (L[19] < 250000)
     {
	   static int singleRate[]	= {5,10,16,21,26,31,36,42,47,52,57,63};
	   static int marriedRate[]	= {10,21,31,42,52,63,73,83,94,104,115,125};
	   if ((status==SINGLE) || (status==MARRIED_FILLING_SEPARAT) || (status==HEAD_OF_HOUSEHOLD))  L[60] = singleRate[nyc_resident_months-1];  else
      if ((status==MARRIED_FILLING_JOINTLY) || (status==WIDOW))  L[60] = marriedRate[nyc_resident_months-1];
     }
    else
     L[60] = 0.0;
    showline(60);
  } /*NYC*/

 GetLineRoundedF( "L61", &L[61] );	/* Other refundable credits, IT-203-ATT line 17) */

 GetLineRoundedF( "L62", &L[62] );	/* Total NY State tax withheld. */

 GetLineRoundedF( "L63", &L[63] );	/* Total City of NY tax withheld. */
 GetLineRoundedF( "L64", &L[64] );	/* Yonkers tax withheld. */
 
 GetLineRoundedF( "L65", &L[65] );	/* Total estimated tax payments (from IT-370)*/

 for (j=60; j<=65; j++) L[66] = L[66] + L[j];
 showline(66);

 if (L[66] > L[59])
  {
   L[67] = L[66] - L[59];
   fprintf(outfile,"L67 = %6.2f	REFUND !!!\n", L[67] );
   GetLineRoundedF( "L69", &L[69] );	/* Amount to apply to next year tax  */
   GetLineRoundedF( "L71", &L[71] );	/* Estimated tax penalty */
   GetLineRoundedF( "L72", &L[72] );	/* Other penalties and interest*/

   L[67]	= L[67]	- L[71];
   fprintf(outfile,"L67 = %6.2f	Overpaid (after subtracting penalty) !!!\n", L[67] );
   L[68]	= L[67] - L[69];
   fprintf(outfile,"L68 = %6.2f	Amount to refund\n", L[68] );
   fprintf(outfile,"L69 = %6.2f	Amount you want applied to next year tax\n", L[69] );
  }
 else
  {
   L[70] = L[59] - L[66];
   fprintf(outfile,"L70 = %6.2f	YOU OWE !!!\n", L[70] );
   GetLineRoundedF( "L71", &L[71] );	/* Estimated tax penalty */
   GetLineRoundedF( "L72", &L[72] );	/* Other penalties and interest*/
   L[70] = L[70] + L[71] + L[72];
   fprintf(outfile,"L70 = %6.2f	YOU OWE (including penalty) !!!\n", L[70] );
  }
 
 fclose(infile);
 fclose(outfile);

 outfile = fopen(it203_xfdf_outfname,"w");
 output_xfdf_form_data(outfile, it203_2014, L, it203_2014_State, StateL);
 fclose(outfile);

 if (std_ded < itemized_ded) {
  outfile = fopen(it203d_xfdf_outfname,"w");
  output_xfdf_form_data(outfile, it203d_2014, ded_sched); 
  fclose(outfile);
 }

 printf("\nListing results from file: %s\n\n", outfname);
 Display_File( outfname );


 return 0;
}
