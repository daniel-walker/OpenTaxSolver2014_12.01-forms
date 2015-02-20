/************************************************************************/
/* TaxSolve_NY_IT-201_2014.c - NY State Tax form IT-201 for 2014.	*/
/* Copyright (C) 2003-2015 - Aston Roberts, Skeet Monker		*/
/* 									*/
/* Compile:   gcc taxsolve_NY_IT201_2014.c -o taxsolve_NY_IT201_2014	*/
/* Run:	      ./taxsolve_NY_IT201_2014  NY_IT201_2014.txt 		*/
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

double A[10], S[10];

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


int ImportFederalReturnData( char *fedlogfile, struct FedReturnData *fed_data )
{
 FILE *infile;
 char fline[1000], word[1000];
 int linenum;

 for (linenum=0; linenum<MAX_LINES; linenum++) 
  { 
   fed_data->fedline[linenum] = 0.0;
   fed_data->schedA[linenum] = 0.0;
   fed_data->schedD[linenum] = 0.0;
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
       if (verbose) printf("FedLin[%d] = %2.2f\n", linenum, fed_data->fedline[linenum]);
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
     if (verbose) printf("FedLin[%d] = %2.2f\n", linenum, fed_data->schedA[linenum]);
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
     if (verbose) printf("FedLin[%d] = %2.2f\n", linenum, fed_data->schedD[linenum]);
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


void worksheet1()	/*Tax Computation Worksheet 1 (pg 51) */
{ double ws[100];
  ws[1] = L[33];
  ws[2] = L[38];
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
  L[39] = ws[9];
}


void worksheet2()	/*Tax Computation Worksheet 2 (pg 51) */
{ double ws[100];
  ws[1] = L[33];
  ws[2] = L[38];
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
  L[39] = ws[11];
}


void worksheet3()	/*Tax Computation Worksheet 3 (pg 51) */
{ double ws[100];
  ws[1] = L[33];
  ws[2] = L[38];
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
   L[39] = ws[11];
  }


void worksheet4()	/*Tax Computation Worksheet 4 (pg 51) */
{ double ws[100];
  ws[1] = L[33];
  ws[2] = L[38];
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
   L[39] = ws[11];
  }


void worksheet5()	/*Tax Computation Worksheet 5 (pg 52) */
{ double ws[100];
  ws[1] = L[33];
  ws[2] = L[38];
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
  L[39] = ws[9];
}


void worksheet6()	/*Tax Computation Worksheet 6 (pg 52) */
{ double ws[100];
  ws[1] = L[33];
  ws[2] = L[38];
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
  L[39] = ws[11];
}


void worksheet7()	/*Tax Computation Worksheet 7 (pg 52) */
{ double ws[100];
  ws[1] = L[33];
  ws[2] = L[38];
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
   L[39] = ws[11];
  }


void worksheet8()	/*Tax Computation Worksheet 8 (pg 53) */
{ double ws[100];
  ws[1] = L[33];
  ws[2] = L[38];
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
  L[39] = ws[9];
}


void worksheet9()	/*Tax Computation Worksheet 9 (pg 53) */
{ double ws[100];
  ws[1] = L[33];
  ws[2] = L[38];
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
  L[39] = ws[11];
}


void worksheet10()	/*Tax Computation Worksheet 10 (pg 53) */
{ double ws[100];
  ws[1] = L[33];
  ws[2] = L[38];
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
   L[39] = ws[11];
  }


void tax_computation_worksheet( int status )
{ /* Worksheets from pages 51-53. Come here when AGI L[33] > $104,600. */
 switch (status)
  {
     case MARRIED_FILLING_JOINTLY:  case WIDOW:
	if (L[33] <= 2092800.0)
	 {
	   if (L[38] <= 156900.0)
	    worksheet1();
	   else
	   if ((L[33] > 156900.0) && (L[38] <= 313850.0))
	    worksheet2();
	   else
	   if ((L[33] > 313850.0) && (L[38] > 313850.0))
	    worksheet3();
	   else
	    worksheet4();
	 }
	else
	 worksheet4();
	break;
     case SINGLE:  case MARRIED_FILLING_SEPARAT:
	if (L[33] <= 1046350.0)
	 {
	   if (L[38] <= 209250.0)
	    worksheet5();
	   else
	   if ((L[33] > 209250.0) && (L[38] > 209250.0))
	    worksheet6();
	   else
	    worksheet7();
	 }
	else
	 worksheet7();
	break;
     case HEAD_OF_HOUSEHOLD:
	if (L[33] <= 15569550.0)
	 {
	   if (L[38] <= 261550.0)
	    worksheet8();
	   else
	   if ((L[33] > 261550.0) && (L[38] > 261550.0))
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


/*----------------------------------------------------------------------------*/

int main( int argc, char *argv[] )
{
 int j, k, argk;
 char word[1000], outfname[1000];
 time_t now;
 int Dependent, Exemptions, nyc_resident;
 double itemized_ded, std_ded=0.0, LTC=0, AddAdj=0.0, CollegeDed=0.0;
 double ded_sched[MAX_LINES];
 char prelim_1040_outfilename[5000];

 /* Intercept any command-line arguments. */
 printf("NY-IT201 - 2011 - v%3.1f\n", thisversion);
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
    j = strlen(outfname)-1;
    while ((j>=0) && (outfname[j]!='.')) j--;
    if (j<0) strcat(outfname,"_out.txt"); else strcpy(&(outfname[j]),"_out.txt");
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

 get_parameter( infile, 's', word, "FileName" );      /* Preliminary Return Output File-name. */
 get_word(infile, prelim_1040_outfilename );
 if (ImportFederalReturnData( prelim_1040_outfilename, &PrelimFedReturn ) == 0)
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
 fprintf(outfile,"Status = %s (%d)\n", statusnames[status], status);

 get_parameter( infile, 's', word, "Dependent" );
 get_parameter( infile, 'b', &Dependent, "Dependent?"); 
 if (Dependent==1) fprintf(outfile," Check box C = Yes\n");  else fprintf(outfile," Check box C = No\n");

 // GetLineF( "L1", &L[1] );	/* Wages. */
 L[1] = PrelimFedReturn.fedline[7];
 showline(1);

 // GetLineF( "L2", &L[2] );	/* Taxable Interest. */
 L[2] = PrelimFedReturn.fedline[8];
 showline(2);

 // GetLineF( "L3", &L[3] );	/* Ordinary Dividends. */
 L[3] = PrelimFedReturn.fedline[9];
 showline(3);

 // GetLineF( "L4", &L[4] );	/* Taxable refunds, credits, offsets */
 L[4] = PrelimFedReturn.fedline[10];
 showline(4);

 // GetLineF( "L5", &L[5] );	/* Alimony received */
 L[5] = PrelimFedReturn.fedline[11];
 showline(5);

 // GetLineF( "L6", &L[6] );	/* Business income/loss (fed sched C) */
 L[6] = PrelimFedReturn.fedline[12];
 showline(6);

 // GetLineF( "L7", &L[7] );	/* Capital gains/losses (fed sched D) */
 L[7] = PrelimFedReturn.fedline[13];
 showline(7);

 // GetLineF( "L8", &L[8] );	/* Other gains/losses (fed form 4794) */
 L[8] = PrelimFedReturn.fedline[14];
 showline(8);

 // GetLineF( "L9", &L[9] );	/* Taxable IRA distributions */
 L[9] = PrelimFedReturn.fedline[15];
 showline(9);

 // GetLineF( "L10", &L[10] );	/* Taxable pension/annuity amounts  */
 L[10] = PrelimFedReturn.fedline[16];
 showline(10);

 // GetLineF( "L11", &L[11] );	/* Rental, royalties, partnership, S corp, (fed sched E) */
 L[11] = PrelimFedReturn.fedline[17];
 showline(11);

 // GetLineF( "L13", &L[13] );	/* Farm income (fed sched F) */
 L[13] = PrelimFedReturn.fedline[18];
 showline(13);

 // GetLineF( "L14", &L[14] );	/* Unemployment compensation */
 L[14] = PrelimFedReturn.fedline[19];
 showline(14);

 // GetLineF( "L15", &L[15] );	/* Taxable Social Sec. benefits */
 L[15] = PrelimFedReturn.fedline[20];
 showline(15);
 L[27] = L[15];

 // GetLineF( "L16", &L[16] );	/* Other income (pg. 14) */
 L[16] = PrelimFedReturn.fedline[21];
 showline(16);

 for (j = 1; j <= 11; j++)
  L[17] = L[17] + L[j];
 for (j=13; j <= 16; j++)
  L[17] = L[17] + L[j];
 showline(17);
 if (absolutev( L[17] - PrelimFedReturn.fedline[22]) > 0.1)
  {
   printf(" Warning: L[17] = %6.2f, while Fed-line[22] = %6.2f\n", L[17], PrelimFedReturn.fedline[22] );
   fprintf(outfile," Warning: L[17] = %6.2f, while Fed-line[22] = %6.2f\n", L[17], PrelimFedReturn.fedline[22] );
  }

 // GetLineF( "L18", &L[18] );	/* Total federal adjustments to income (pg 14) */
 L[18] = PrelimFedReturn.fedline[36];
 showline(18);

 L[19] = L[17] - L[18];
 showline_wmsg( 19, "Federal adjusted gross income" );
 if (absolutev(L[19] - PrelimFedReturn.fedline[38]) > 0.1)
  {
   printf(" Warning: L[19] = %6.2f, while Fed-line[38] = %6.2f\n", L[19], PrelimFedReturn.fedline[38] );
   fprintf(outfile," Warning: L[19] = %6.2f, while Fed-line[38] = %6.2f\n", L[19], PrelimFedReturn.fedline[38] );
  } 

 GetLineF( "L20", &L[20] );	/* Interest income from non-NY state or local bonds */

 GetLineF( "L21", &L[21] );	/* Public employee retirement contributions (pg 15) */

 GetLineF( "L22", &L[22] );	/* College choice tuition saving distributions */

 GetLineF( "L23", &L[23] );	/* Other (pg 16) */

 for (j = 19; j <= 23; j++) L[24] = L[24] + L[j];
 showline(24);

 L[25] = L[4];			/* Taxable refunds, credits, offsets */
 showline(25);

 GetLineF( "L26", &L[26] );	/* Pensions of NYS and local governments and the federal government (see page 16)  */

 L[27] = L[14];			/* Taxable amount of social security benefits */
 showline(27);

 GetLineF( "L28", &L[28] );	/* Interest income on U.S. government bonds */

 GetLine( "L29", &L[29] );	/* Pension and annuity income exclusion  */
 if (L[29] > 20000.0)
  {
   L[29] = 20000.0;
   showline_wmsg( 29, "(Limited to 20,000.)" );
  }
 else
  showline(29);

 GetLineF( "L30", &L[30] );	/* College choice tuition savings deduction / earnings distributions */

 GetLineF( "L31", &L[31] );	/* Other (see page 21) */

 for (j=25; j <= 31; j++)
  L[32] = L[32] + L[j];
 showline(32);

 L[33] = L[24] - L[32];
 showline_wmsg(33,"New York adjusted gross income (AGI)");

 /* NYS Itemized Deductions Schedule, pg. 38 */
 GetLine( "LTcare%", &LTC );
 GetLine( "AddAdj", &AddAdj );
 GetLine( "CollegeDed", &CollegeDed );
 ded_sched[1] = PrelimFedReturn.schedA[4];
 ded_sched[2] = PrelimFedReturn.schedA[9];
 ded_sched[3] = PrelimFedReturn.schedA[15];
 ded_sched[4] = PrelimFedReturn.schedA[19];
 ded_sched[5] = PrelimFedReturn.schedA[20];
 ded_sched[6] = PrelimFedReturn.schedA[27];
 ded_sched[7] = PrelimFedReturn.schedA[28];
 ded_sched[8] = PrelimFedReturn.schedA[29];
 ded_sched[9] = PrelimFedReturn.schedA[5] + PrelimFedReturn.schedA[8] 
		+ LTC * PrelimFedReturn.schedA[4];
 ded_sched[10] = ded_sched[8] - ded_sched[9];	
 ded_sched[11] = AddAdj;
 ded_sched[12] = ded_sched[10] + ded_sched[11];
 if (L[33] <= 100000.0)
  ded_sched[13] = 0.0;
 else
  { /*L33_morethan_100000*/
   double ws[50];
   if (L[33] <= 475000.0)
    { /* Worksheet 3 pg 40 */
      ws[1] = L[33];
      switch (status)
       {
        case SINGLE:  case MARRIED_FILLING_SEPARAT: ws[2] = 100000.0; break;
        case HEAD_OF_HOUSEHOLD:                     ws[2] = 150000.0; break;
        case MARRIED_FILLING_JOINTLY:  case WIDOW:  ws[2] = 200000.0; break;
	default: ws[2] = 0.0;
       }
      ws[3] = ws[1] - ws[2];
      if (ws[3] < 0.0)
	ded_sched[13] = 0.0;
      else
       {
	ws[4] = smallerof( ws[3], 50000.0 );
	ws[5] = 0.0001 * Round( 10000.0 * (ws[4] / 50000) );
	ws[6] = 0.25 * ded_sched[1];
	ws[7] = ws[5] * ws[6];
	ded_sched[13] = ws[7];
       }
    }
   else
   if (L[33] <= 525000.0)
    { /* Worksheet 4 pg 40 */
      ws[1] = L[33] - 475000.0;
      ws[2] = 0.0001 * Round( 10000.0 * (ws[1] / 50000) );
      ws[3] = 0.25 * ded_sched[1];
      ws[4] = ws[2] * ws[3];
      ws[5] = ws[3] + ws[4];
      ded_sched[13] = ws[5];  
    }
   else
   if (L[33] <= 1000000.0)
    ded_sched[13] = 0.5 * ded_sched[12];
   else
   if (L[33] <= 10000000.0)
    { /* Worksheet 5 pg 40 */
      ws[1] = L[33];
      ws[2] = 0.5 * ded_sched[4];
      ws[3] = ws[1] - ws[2];
      ded_sched[13] = ws[3];  
    }
   else
    { /* Worksheet 6 pg 40 */
      ws[1] = L[33];
      ws[2] = 0.25 * ded_sched[4];
      ws[3] = ws[1] - ws[2];
      ded_sched[13] = ws[3];  
    }
   } /*L33_morethan_100000*/
 ded_sched[14] = ded_sched[12] - ded_sched[13];
 ded_sched[15] = CollegeDed;
 ded_sched[16] = ded_sched[14] + ded_sched[15];
 itemized_ded = ded_sched[16];
 // GetLine( "Itemized_Deduction", &itemized_ded );     /* Form IT-201-ATT or 0.0 if std ded. */

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
   L[34] = std_ded; 
   showline_wmsg(34,"(Mark Std-deduction)");
  }
 else
  {
   L[34] = itemized_ded;
   showline_wmsg(34,"(Mark Itemized-deduction)");
  }

 L[35] = L[33] - L[34];
 if (L[35] < 0.0) L[35] = 0.0;
 else showline(35);

 get_parameter( infile, 's', word, "L36" );	/* Number of Dependent Exemptions (Pg 76, line e) */
 get_parameters( infile, 'i', &k, "L36" );
 L[36] = 1000.0 * (double)k;
 showline(36);
 
 L[37] = L[35] - L[36];
 if (L[37] < 0.0)
   L[37] = 0.0;
 showline_wmsg(37,"taxable income");
 L[38] = L[37];
 showline(38);

 if (L[33] <= 104600.0)
   L[39] = TaxRateLookup( L[38], status );
 else
   tax_computation_worksheet( status );
 showline(39);

 /* Household credit. */
 get_parameter( infile, 's', word, "Exemptions" );	/* NY dependent exemptions, Pg 19. */
 get_parameter( infile, 'i', &Exemptions, "Exemptions" );
 if (Dependent)
  L[40] = 0.0;
 else	/* From tables starting on page 25. */
 if (status==SINGLE)
  {
   if (L[19] <  5000.0) L[40] = 75.0; else
   if (L[19] <  6000.0) L[40] = 60.0; else
   if (L[19] <  7000.0) L[40] = 50.0; else
   if (L[19] < 20000.0) L[40] = 45.0; else
   if (L[19] < 25000.0) L[40] = 40.0; else
   if (L[19] < 28000.0) L[40] = 20.0; else  L[40] = 0.0;
  }
 else	/* Status = MARRIED_FILLING_JOINTLY, MARRIED_FILLING_SEPARAT, Head_of_house, Widow */
  if (status!=MARRIED_FILLING_SEPARAT)
   {
    if (L[19] <  5000.0) L[40] = 90.0 + 15.0 * (Exemptions-1); else
    if (L[19] <  6000.0) L[40] = 75.0 + 15.0 * (Exemptions-1); else
    if (L[19] <  7000.0) L[40] = 65.0 + 15.0 * (Exemptions-1); else
    if (L[19] < 20000.0) L[40] = 60.0 + 15.0 * (Exemptions-1); else
    if (L[19] < 22000.0) L[40] = 60.0 + 10.0 * (Exemptions-1); else
    if (L[19] < 25000.0) L[40] = 50.0 + 10.0 * (Exemptions-1); else
    if (L[19] < 28000.0) L[40] = 40.0 +  5.0 * (Exemptions-1); else  
    if (L[19] < 32000.0) L[40] = 20.0 +  5.0 * (Exemptions-1); else  L[40] = 0.0;
   }
  else
   {
    if (L[19] <  5000.0) L[40] = 45.0 + 8.0 * (Exemptions-1); else
    if (L[19] <  6000.0) L[40] = 37.5 + 8.0 * (Exemptions-1); else
    if (L[19] <  7000.0) L[40] = 32.5 + 8.0 * (Exemptions-1); else
    if (L[19] < 20000.0) L[40] = 30.0 + 8.0 * (Exemptions-1); else
    if (L[19] < 22000.0) L[40] = 30.0 + 5.0 * (Exemptions-1); else
    if (L[19] < 25000.0) L[40] = 25.0 + 5.0 * (Exemptions-1); else
    if (L[19] < 28000.0) L[40] = 20.0 + 3.0 * (Exemptions-1); else  
    if (L[19] < 32000.0) L[40] = 10.0 + 3.0 * (Exemptions-1); else  L[40] = 0.0;
   }
 showline_wmsg(40,"NY state household credit");	/* NY state household credit, (pg 29). */

 GetLineF( "L41", &L[41] );	/* Resident credit, Form IT-112-R or IT-112-C, pg 98 */

 GetLineF( "L42", &L[42] );	/* Other New York State nonrefundable credits  */

 L[43] = L[40] + L[41] + L[42];
 showline(43);

 L[44] = L[39] - L[43];
 if (L[44] < 0.0) L[44] = 0.0;
 else showline(44);

 GetLineF( "L45", &L[45] );	/* Net Other New York State taxes, Form IT-201-ATT, line 30 */

 L[46] = L[44] + L[45];
 showline_wmsg(46,"Total New York State taxes");

 get_parameter( infile, 's', word, "NYC_Resident" );
 get_parameters( infile, 'b', &nyc_resident, "NYC_Resident (yes/no) ?"); 

 GetLine( "L50", &L[50] );	/* Part-year New York City resident tax */
 GetLine( "L51", &L[51] );	/* Other New York City taxes */
 GetLine( "L53", &L[53] );	/* NY City nonrefundable credits */

 if (nyc_resident)
  { /*NYC*/
   L[47] = NYcityTaxRateFunction( L[38], status ); 
   showline(47);

   /* NYC Household credit. */
   if (Dependent) L[48] = 0.0;
   else
   if (status==SINGLE)		/* From table 4, 5, or 6 on page 30. */
    {
     if (L[19] <  10000.0) L[48] = 15.0; else
     if (L[19] <  12500.0) L[48] = 10.0; else  L[48] = 0.0;
    }
   else	/* Status = 2, 4, or 5. */
   if (status!=MARRIED_FILLING_SEPARAT)
    {
     if (L[19] <  15000.0) L[48] = 30.0 * Exemptions; else
     if (L[19] <  17500.0) L[48] = 25.0 * Exemptions; else
     if (L[19] <  20000.0) L[48] = 15.0 * Exemptions; else
     if (L[19] <  22500.0) L[48] = 10.0 * Exemptions; else  L[48] = 0.0;
    }
   else
    {
     if (L[19] <  15000.0) L[48] = 15.0 * Exemptions; else
     if (L[19] <  17500.0) L[48] = 13.0 * Exemptions; else
     if (L[19] <  20000.0) L[48] =  8.0 * Exemptions; else
     if (L[19] <  22500.0) L[48] =  5.0 * Exemptions; else  L[48] = 0.0;
    }
   showline_wmsg(48,"NY City household credit");	/* NY City household credit, (pg 34). */

   L[49] = L[47] - L[48];
   if (L[49] > 0.0)
    showline(49);
   else
    L[49] = 0.0;

   showline(50);
   showline(51);

   L[52] = L[49] + L[50] + L[51];
   showline(52);
   showline(53);

   L[54] = L[52] - L[53];
   if (L[54] > 0.0) showline(54);  else  L[54] = 0.0;

   /* Yonkers case not added. */

   L[58] = L[54] + L[55] + L[56] + L[57];
   showline_wmsg(58,"NYC taxes");
  } /*NYC*/

 GetLineF( "L59", &L[59] );	/* Sales or use tax, see pg 29. */

 GetLineF( "L60", &L[60] );	/* Voluntary Gift contibutions (pgs 30). */

 L[61] = L[46] + L[58] + L[59] + L[60];
 showline(61);

 L[62] = L[61];
 showline(62);

 GetLineF( "L63", &L[63] );	/* Empire State child credit (attach new Form IT-213) */

 GetLineF( "L64", &L[64] );	/* NYS/NYC Child care/dependent credit. Form IT-216. */

 GetLineF( "L65", &L[65] );	/* NYS Earned income credit. Form IT-213. */

 GetLineF( "L66", &L[66] );	/* NYS State noncustodial parent EIC (attach new Form IT-209) */

 GetLineF( "L67", &L[67] );	/* Real property credit. Form IT-214. */

 GetLineF( "L68", &L[68] );	/* College tution credit. Form IT-272. */

 if (nyc_resident)		/* City of NY school taxcredit. (pg 26) */
  { /*NYC*/
    if (Dependent) L[69] = 0.0;
    else
    if (L[37] < 250000)
     {
      if ((status==SINGLE) || (status==MARRIED_FILLING_SEPARAT) || (status==HEAD_OF_HOUSEHOLD))  L[69] = 63.0;  else
      if ((status==MARRIED_FILLING_JOINTLY) || (status==WIDOW))  L[69] = 125.0;
     }
    else
     L[69] = 0.0;
    showline(69);
    /* L[70] = earned_income_credit; */
    /* showline(70); */
  } /*NYC*/

 GetLineF( "L71", &L[71] );	/* Other refundable credits, IT-201-ATT line 18) */

 GetLineF( "L72", &L[72] );	/* Total NY State tax withheld. */

 GetLineF( "L73", &L[73] );	/* Total City of NY tax withheld. */
 GetLineF( "L74", &L[74] );	/* Yonkers tax withheld. */
 
 GetLineF( "L75", &L[75] );	/* Total estimated tax payments (from IT-370)*/

 for (j = 63; j <= 75; j++) L[76] = L[76] + L[j];
 showline(76);

 if (L[76] > L[62])
  {
   L[77] = L[76] - L[62];
   fprintf(outfile,"L77 = %6.2f	REFUND !!!\n", L[77] );
   L[78] = L[77];
   showline(78);
  }
 else
  {
   L[80] = L[62] - L[76];
   fprintf(outfile,"L80 = %6.2f	YOU OWE !!!\n", L[80] );
  }
 
 fclose(infile);
 fclose(outfile);
 printf("\nListing results from file: %s\n\n", outfname);
 Display_File( outfname );
 return 0;
}
