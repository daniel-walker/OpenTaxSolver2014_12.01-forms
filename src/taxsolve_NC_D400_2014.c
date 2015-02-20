/************************************************************************/
/* TaxSolve_NC_D400_2014.c - North Carolina 2014 NC-DC400 State Taxes.	*/
/* Copyright (C) 2014 - S.Jenkins					*/
/* 									*/
/* Compile:   gcc taxsolve_NC_D400_2014.c -o taxsolve_NC_D400_2014	*/
/* Run:  ./taxsolve_NC_D400_2014		  			*/
/*  Uses log from TaxSolve Federal 1040.				*/
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
/* 1-4-2015	http://opentaxsolver.sourceforge.com/			*/
/*	Earlier versions - Lincoln Baxter (lab@lincolnbaxter.com)	*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "taxsolve_routines.c"

float thisversion=12.0;

#define SINGLE                  1
#define MARRIED_FILLING_JOINTLY 2
#define MARRIED_FILLING_SEPARAT 3
#define HEAD_OF_HOUSEHOLD       4
#define WIDOW                   5
#define Yes 1
#define No  0


struct FedReturnData
 {
  double fedline[MAX_LINES];
  int Itemized, Limited, Limited_L6;
  double Sched_A[MAX_LINES];
 };


void convert_slashes( char *fname )
{ /* Convert slashes in file name based on OS type. */
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

 convert_slashes( fedlogfile );
 infile = fopen(fedlogfile, "r");
 if (infile==0)
  {
   printf("Error: Could not open federal return '%s'\n", fedlogfile);
   fprintf(outfile,"\nError: Could not open federal return '%s'\n", fedlogfile);
system("pwd");
system("ls -l");
   exit(1);
  }
 printf(" Reading file: %s\n", fedlogfile );

 /* Set initial default values. */
 fed_data->Itemized = 1; 
 fed_data->Limited = 1; 
 fed_data->Limited_L6 = 1; 
 for (linenum=0; linenum<MAX_LINES; linenum++) fed_data->fedline[linenum] = 0.0;
 for (linenum=0; linenum<MAX_LINES; linenum++) fed_data->Sched_A[linenum] = 0.0;

 read_line(infile,fline);  linenum = 0;
 while (!feof(infile))
  {
   if ( verbose ) printf( "Read Line: %s" ,fline );
   if (strstr(fline,"Use standard deduction.")!=0) fed_data->Itemized = 0;
   if (strstr(fline,"Deductions not limited")!=0) 
    {
     fed_data->Limited = 0;
     if ( strstr(fline,"line 6") ) fed_data->Limited_L6 = 0;
    }
   next_word(fline, word, " \t=");
   if ((strstr(word,"A")==word) && (isdigit(word[1])) && (strstr(fline," = ")!=0))
    {
     if (sscanf(&word[1],"%d",&linenum)!=1) printf("Error: Reading fed sched-A line number '%s%s'\n",word,fline);
     next_word(fline, word, " 	=");
     if (sscanf(word,"%lf", &fed_data->Sched_A[linenum])!=1) printf("Error: Reading fed sched-A line %d '%s%s'\n",linenum,word,fline);
     if (verbose) printf("Sched_A[%d] = %2.2f\n", linenum, fed_data->Sched_A[linenum]);
    }
   if ((strstr(word,"L")==word) && (strstr(fline," = ")!=0))
    {
     if (sscanf(&word[1],"%d",&linenum)!=1) printf("Error: Reading fed line number '%s%s'\n",word,fline);
     next_word(fline, word, " 	=");
     if (sscanf(word,"%lf", &fed_data->fedline[linenum])!=1) printf("Error: Reading fed line %d '%s%s'\n",linenum,word,fline);
     if (verbose) printf("FedLin[%d] = %2.2f\n", linenum, fed_data->fedline[linenum]);
    }
   read_line(infile,fline);
  }
 fclose(infile);
}


/* ------------------------------------------------------------------------------------ */
/* ---				Main						    --- */
/* ------------------------------------------------------------------------------------ */
int main( int argc, char *argv[] )
{
 int j, jj, k, status;
 char word[1000], outfname[1000];
 time_t now;
 struct FedReturnData fed_data;
 double stdded, min_payment=0.0, min2file;
 double L21a=0.0, L21b=0.0, L21c=0.0, L21d=0.0;

 /*-----------------------------------------*/
 /* --- Decode any command line options. -- */
 /*-----------------------------------------*/
 printf("NC D400 2014 - v%3.2f\n", thisversion);
 jj = 1;  k=1;
 while (jj < argc)
 {
  if (strcmp(argv[jj],"-verbose")==0)  { verbose = 1; }
  else
  if (k==1)
   {
    infile = fopen(argv[jj],"r");
    if (infile==0)
     {
	printf("ERROR: Parameter file '%s' could not be opened.\n", argv[jj]); 
	fprintf(outfile,"ERROR: Parameter file '%s' could not be opened.\n", argv[jj]); 
	exit(1);
     }
    k = 2;
    /* Base name of output file on input file. */
    strcpy(outfname,argv[jj]);
    j = strlen(outfname)-1;
    while ((j>=0) && (outfname[j]!='.')) j--;
    if (j<0) strcat(outfname,"_out.txt"); else strcpy(&(outfname[j]),"_out.txt");
    outfile = fopen(outfname,"w");
    if (outfile==0)
     {
	printf("ERROR: Output file '%s' could not be opened.\n", outfname); 
	fprintf(outfile,"ERROR: Output file '%s' could not be opened.\n", outfname); 
	exit(1);
     }
    printf("Writing results to file:  %s\n", outfname);
   }
  else
   {
	printf("Unknown command-line parameter '%s'\n", argv[jj]); 
	fprintf(outfile,"Unknown command-line parameter '%s'\n", argv[jj]); 
	exit(1);
   }
  jj++;
 }

 if (infile==0)
  {
	printf("Error: No input file on command line.\n"); 
	fprintf(outfile,"Error: No input file on command line.\n"); 
	exit(1);
  }
 

 /*--------------------------*/
 /* ---- Get Input Data ---- */
 /*--------------------------*/

 /* Pre-initialize all lines to zeros. */
 for (j = 0; j < MAX_LINES; j++) { L[j] = 0.0; }

 /* Accept parameters from input file. */
 /* Expect  D-400 lines, something like:
	Title:  NC-400 1999 Return
	L6	34900.0  {Wages}
 */

 /* Accept Form's "Title" line, and put out with date-stamp for your records. */
 read_line( infile, word );
 now = time(0);
 fprintf(outfile,"\n%s		%s\n", word, ctime( &now ));


 /* get_parameter(infile, kind, x, emssg ) */
 get_parameter( infile, 's', word, "FedReturn" );	/* File name of Federal Return log file. */
 if ( verbose ) printf( "word: %s\n", word );
 get_word( infile, word ); 
 ImportFederalReturnData( word, &fed_data);

 get_parameter( infile, 's', word, "Status"); /* 1=single, 2=married/joint, 3=married/separate, 4=house-head, 5=widow */
 get_parameter( infile, 'l', word, "Status ?");
 if ((word[0]>'0') && (word[0]<'6')) status = word[0]-48; else
 if (strncasecmp(word,"Single",4)==0) status = SINGLE; else
 if (strncasecmp(word,"Married/Joint",13)==0) status = MARRIED_FILLING_JOINTLY; else
 if (strncasecmp(word,"Married/Sep",11)==0) status = MARRIED_FILLING_SEPARAT; else
 if (strncasecmp(word,"Head_of_House",4)==0) status = HEAD_OF_HOUSEHOLD; else
 if (strncasecmp(word,"Widow",4)==0) status = WIDOW;
 else
  { 
   printf("Error: unrecognized status '%s'. Must be: Single, Married/joint, Married/sep, Head_of_house, Widow(er)\nExiting.\n", word); 
   fprintf(outfile,"Error: unrecognized status '%s'. Must be: Single, Married/joint, Married/sep, Head_of_house, Widow(er)\nExiting.\n", word); 
   exit(1); 
  }
 fprintf(outfile,"Status = %s (%d)\n", word, status);

 GetLine( "L7", &L[7] );	/* Additions to Fed AGI - From Sched-S Part A, Line 4. */  
 GetLine( "L9", &L[9] );	/* Deductions from Fed AGI - From Sched-S Part B, Line 12. */  
 GetLine( "L11", &L[11] );	/* Enter 0 to use Std Deduction; Otherwise Itemized Ded. from Sched-S part C, Line 19. */  
 GetLine( "L13", &L[13] );	/* Enter 1.0 for full-year residents. Otherwise amount from Sched-S part D, Line 22. */  
 GetLine( "L16", &L[16] );	/* Tax credits. (D-400TC part 3 line 20) */  
 GetLine( "L18", &L[18] );	/* Consumer Use Tax. (pgs 9+10) */
 GetLine( "L20", &L[20] );	/* North Carolina Income Tax Withheld */
 GetLine( "L21a", &L21a );	/* Other tax payments. 2014 Estimated Tax. */
 GetLine( "L21b", &L21b );	/* Other tax payments. Paid with Extension. */
 GetLine( "L21c", &L21c );	/* Other tax payments. Partnership. */
 GetLine( "L21d", &L21d );	/* Other tax payments. S Corporation. */

 fclose(infile);


 /*-------------------------------*/
 /* ---- Do Tax Calculations ---- */
 /*-------------------------------*/

 L[6] = fed_data.fedline[37];		/* Taxable income from Fed1040 Line 37. */

 switch (status)
  {
   case SINGLE: 		 stdded = 7500.0; 	/* NC std single deduction. */
				 min2file = 7500.0;
	break;
   case MARRIED_FILLING_JOINTLY: stdded = 15000.0; 	/* NC std Married/joint deduction. */
				 min2file = 15000.0;
	break;
   case WIDOW:			 stdded = 15000.0; 	/* NC std widow(er) deduction. */
				 min2file = 15000.0;
	break;
   case MARRIED_FILLING_SEPARAT: stdded = 7500.0; 	/* NC std Married/sep deduction. */
				 min2file = 7500.0;
	break;
   case HEAD_OF_HOUSEHOLD:	 stdded = 12000.0; 	/* NC std Head of house deduction. */
				 min2file = 12000.0;
	break;
   default:  
	stdded = 0;  printf("Unknown status\n");  fprintf(outfile,"Unknown status\n");
	exit(1); 
  }

 if (L[6] <= min2file)
  fprintf(outfile, "You may not need to file NC tax return, due to your income.\n");

 L[8] = L[6] + L[7];

 L[10] = L[8] - L[9];

 if (L[11] < stdded)
  L[11] = stdded;

 L[12] = L[10] - L[11];

 L[14] = L[13] * L[12];		/* NC Taxable Income. */

 L[15] = 0.058 * L[14];		/* NC Income Tax. */

 L[17] = L[15] - L[16];

 /* Calculate USE tax, if not entered on L18
  * based on Use Tax Worksheet on page 9.
  *  Estimate as:   0.000675 * L[14]
  * If you made purchases greater that $1000 that need
  * to be reported, or taxes paid to another state, then
  * fill out the Work Sheet in the instructions and enter
  * the amount on L18 in data file.
  */

 printf( "Assuming you have calculated your USE tax (%2.2f) according to instructions pg 9\n", L[18] );

 L[19] = L[17] + L[18];

 L[21] = L21a + L21b + L21c + L21d;

 L[22] = L[20] + L[21];

 if (L[19] > L[22]) 
  {
   L[23] = L[19] - L[22];	/* You OWE */

   /* Check for under payment see form D422 Part I */
   min_payment = 0.9 * L[22]; /* Estimate min required tax payments, form D422 Part I */
   if ((L[23] < min_payment) && (L[23] > 1000.00)) 
    {
     /* We would calculate penalty here... */
     printf("WARNING: Possible underpayment of est. taxes penalty. Calculation not performed.\n"); 
    }
   L[28] = L[27];  /* Assumes no penalties. */
  }
 else
  {
   L[25] = L[22] - L[19];	/* REFUND */
  }


 /*-------------------------*/
 /* ---- Print Results ---- */
 /*-------------------------*/

 showline(6);	/* Taxable fed income */
 showline(7);	/* Additions to fed income */
 showline(8);	
 showline(9);	/* Deductions */
 showline(10);
 showline(11);
 showline(12);
 if (L[13] < 1.0) showline(13);	 /* Part-yr */
 showline_wmsg(14, "North Carolina Taxable Income");
 showline_wmsg(15, "North Carolina Income Tax");
 showline(16);
 showline(17);
 showline(18);
 showline(19);
 showline_wlabelmsg( "L20", L[20], "North Carolina Tax Withheld");
 showline_wlabel( "L21a", L21a );
 showline_wlabel( "L21b", L21b );
 showline_wlabel( "L21c", L21c );
 showline_wlabel( "L21d", L21d );
 showline(22);
 if (L[19] > L[22])
  {
   showline_wmsg(23, "TAX DUE");
   if ((L[23] < min_payment) && (L[23] > 1000.00))
    {
     fprintf(outfile," You may owe underpayment interest and penalties.\n");
     fprintf(outfile," See page 6+7 instructions to calculate them according to your situation.\n");
    }
  }
 else
  {
   showline_wmsg(25, "OVERPAYMENT");
  }

 fclose(outfile);
 Display_File( outfname );
 printf("\nResults written to file '%s'\n", outfname);
 return 0;
}
