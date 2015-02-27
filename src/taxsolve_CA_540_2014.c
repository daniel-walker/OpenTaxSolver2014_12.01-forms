/************************************************************************/
/* TaxSolve_CA_540_2014.c - California state 540 tax form.		*/
/* Copyright (C) 2015 - Aston Roberts					*/
/* 									*/
/* Compile:   gcc taxsolve_CA_540_2014.c -o taxsolve_CA_540_2014	*/
/* Run:	      ./taxsolve_CA_540_2014  CA_540_2014.txt 			*/
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
/* Aston Roberts 1-2-2015	aston_roberts@yahoo.com			*/
/************************************************************************/

float thisversion=12.0;

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "taxsolve_routines.c"
#include "taxsolve_CA_540_2014_forms.h"

#define SINGLE 		        1
#define MARRIED_FILLING_JOINTLY 2
#define MARRIED_FILLING_SEPARAT 3
#define HEAD_OF_HOUSEHOLD       4
#define WIDOW		        5

int status=0;	/* Value for filing status. */


double TaxRateFormula( double income, int status )
{
 double tax;

 if ((status==SINGLE) || (status==MARRIED_FILLING_SEPARAT))
  {
   if (income <   7749.00)  tax =             0.01 * income;		    else
   if (income <  18371.00)  tax =    77.49 +  0.02 * (income -   7749.00);  else
   if (income <  28995.00)  tax =   289.93 +  0.04 * (income -  18371.00);  else
   if (income <  40250.00)  tax =   714.89 +  0.06 * (income -  28995.00);  else
   if (income <  50869.00)  tax =  1390.19 +  0.08 * (income -  40250.00);  else
   if (income < 259844.00)  tax =  2239.71 + 0.093 * (income -  50869.00);  else
   if (income < 311812.00)  tax = 21674.39 + 0.103 * (income - 259844.00);  else
   if (income < 519687.00)  tax = 27027.09 + 0.113 * (income - 311812.00);
   else                     tax = 50516.97 + 0.123 * (income - 519687.00);
  }
 else
 if ((status==MARRIED_FILLING_JOINTLY) || (status==WIDOW))
  {
   if (income <  15498.00)  tax =             0.01 * income;                else
   if (income <  36742.00)  tax =   154.98 +  0.02 * (income -  15498.00);  else
   if (income <  57990.00)  tax =   579.86 +  0.04 * (income -  36742.00);  else
   if (income <  80500.00)  tax =  1429.78 +  0.06 * (income -  57990.00);  else
   if (income < 101738.00)  tax =  2780.38 +  0.08 * (income -  80500.00);  else
   if (income < 519688.00)  tax =  4479.42 + 0.093 * (income - 101738.00);  else
   if (income < 623624.00)  tax = 43348.77 + 0.103 * (income - 519688.00);  else
   if (income < 1039374.00)  tax = 54054.18 + 0.113 * (income - 623624.00);
   else                     tax = 101033.93 + 0.123 * (income - 1039374.00);
  }
 else
  {
   if (income <  15508.00)  tax =             0.01 * income;                else
   if (income <  36743.00)  tax =   155.08 +  0.02 * (income -  15508.00);  else
   if (income <  47366.00)  tax =   579.78 +  0.04 * (income -  36743.00);  else
   if (income <  58621.00)  tax =  1004.70 +  0.06 * (income -  47366.00);  else
   if (income <  69242.00)  tax =  1680.00 +  0.08 * (income -  58621.00);  else
   if (income < 353387.00)  tax =  2529.68 + 0.093 * (income -  69242.00);  else
   if (income < 424065.00)  tax = 28955.17 + 0.103 * (income - 353387.00);  else
   if (income < 706774.00)  tax = 36235.00 + 0.113 * (income - 424065.00);
   else                     tax = 68181.12 + 0.123 * (income - 706774.00);
  }
 return (int)(tax+0.5);
}


double TaxRateFunction( double income, int status )     /* Emulates table lookup or function appropriately. */
{
 double x, tx;
 int k;

 if (income < 100000.0)   /* Quantize to match tax-table exactly. */
  {
   if (income < 99951.0) 
    {
     k = (income + 49) / 100;
     x = 100 * (double)k;
    }
   else x = 99975.0;
   tx = (int)(TaxRateFormula( x, status ));
  }
 else
  tx = TaxRateFormula( income, status );
 return tx;
}


void test_tax_function()
{
 double income;
 for (income=50.0; income < 100000.0; income = income + 100.0)
  printf("%g: %8g %8g %8g\n", income,
		TaxRateFunction( income, SINGLE ),
		TaxRateFunction( income, MARRIED_FILLING_JOINTLY ), 
		TaxRateFunction( income, HEAD_OF_HOUSEHOLD ) );
 exit(0);
}


/*----------------------------------------------------------------------------*/


struct FedReturnData
 {
  double fedline[MAX_LINES], schedA[MAX_LINES];
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
 char fline[2000], word[2000];
 int linenum;

 for (linenum=0; linenum<MAX_LINES; linenum++) 
  { 
   fed_data->fedline[linenum] = 0.0;
   fed_data->schedA[linenum] = 0.0;
  }
 convert_slashes( fedlogfile );
 infile = fopen(fedlogfile, "r");
 if (infile==0)
  {
   printf("Error: Could not open Federal return '%s'\n", fedlogfile);
   fprintf(outfile,"Error: Could not open Federal return '%s'\n", fedlogfile);
   exit( 1 ); 
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



/*----------------------------------------------------------------------------*/
/* ---				Main					  --- */
/*----------------------------------------------------------------------------*/
int main( int argc, char *argv[] )
{
 int argk, j, k, iline7, iline8, iline9, iline10;
 double min2file=0.0, threshA=0, std_ded=0;
 Lmap sched540;
 char word[4000], outfname[4000], prelim_1040_outfilename[5000], ca540_xfdf[1000], ca_schedule_540_xfdf[1000];
 time_t now;

 /* Decode any command-line arguments. */
 argk = 1;  k=1;
 while (argk < argc)
 {
  if (strcmp(argv[argk],"-verbose")==0)  { verbose = 1; }
  else
  if (k==1)
   {
    infile = fopen(argv[argk],"r");
    if (infile==0) {printf("ERROR: Parameter file '%s' could not be opened.\n", argv[argk]); exit(1);}
    k = 2;
    /* Base name of output file on input file. */
    strcpy(outfname,argv[argk]);
    strcpy(ca540_xfdf,argv[argk]);
    strcpy(ca_schedule_540_xfdf,argv[argk]);
    j = strlen(outfname)-1;
    while ((j>=0) && (outfname[j]!='.')) j--;
    if (j<0) {
     strcat(outfname,"_out.txt");
     strcat(ca540_xfdf,"_ca540.xfdf");
     strcat(ca_schedule_540_xfdf,"_540ca.xfdf");
    } else {
     strcpy(&(outfname[j]),"_out.txt");
     strcpy(&(ca540_xfdf[j]),"_ca540.xfdf");
     strcpy(&(ca_schedule_540_xfdf[j]),"_540ca.xfdf");
    }
    outfile = fopen(outfname,"w");
    if (outfile==0) {printf("ERROR: Output file '%s' could not be opened.\n", outfname); exit(1);}
    printf("Writing results to file:  %s\n", outfname);
   }
  else
   {printf("Unknown command-line parameter '%s'\n", argv[argk]); exit(1);}
  argk = argk + 1;
 }
 // test_tax_function();

 if (infile==0) {printf("Error: No input file on command line.\n"); exit(1);}

 /* Pre-initialize all lines to zeros. */
 for (j=0; j<MAX_LINES; j++) 
  { L[j] = 0.0;  sched540[j] = 0.0; }

 /* Accept parameters from input file. */
 /* Expect  CA-540 lines, something like:
	Title:  CA 540 1999 Return
	L12	34900.0  {Wages}
 */

 printf("CA-540 2014 - v%3.2f\n", thisversion);

 /* Accept Form's "Title" line, and put out with date-stamp for your records. */
 read_line( infile, word );
 now = time(0);
 fprintf(outfile,"\n%s		%s\n", word, ctime( &now ));

 get_parameter( infile, 's', word, "FileName" );      /* Preliminary Fed Return Output File-name. */
 get_word(infile, prelim_1040_outfilename );
 ImportFederalReturnData( prelim_1040_outfilename, &PrelimFedReturn );

 /* Filing Status. */
 /* get_parameter(infile, kind, x, emssg ) */
 // get_parameter( infile, 's', word, "Status" );	/* Single, Married/joint, Married/sep, Head house, Widow(er) */
 // get_parameter( infile, 'l', word, "Status?");
 // if (strncasecmp(word,"Single",4)==0) status = SINGLE; else
 // if (strncasecmp(word,"Married/Joint",13)==0) status = MARRIED_FILLING_JOINTLY; else
 // if (strncasecmp(word,"Married/Sep",11)==0) status = MARRIED_FILLING_SEPARAT; else
 // if (strncasecmp(word,"Head_of_House",4)==0) status = HEAD_OF_HOUSEHOLD; else
 // if (strncasecmp(word,"Widow",4)==0) status = WIDOW;
 // else
 //  { 
 //   printf("Error: unrecognized status '%s'. Must be: Single, Married/joint, Married/sep, Head_of_house, Widow(er)\nExiting.\n", word); 
 //   fprintf(outfile,"Error: unrecognized status '%s'. Must be: Single, Married/joint, Married/sep, Head_of_house, Widow(er)\nExiting.\n", word); 
 //   exit(1); 
 //  }
 switch (status)
 {
  case SINGLE: 			fprintf(outfile,"Status = Single (%d)\n", status); L["Single"] = 1; break;
  case MARRIED_FILLING_JOINTLY: fprintf(outfile,"Status = Married/Joint (%d)\n", status); L["MFJ"] = 1; break;
  case MARRIED_FILLING_SEPARAT: fprintf(outfile,"Status = Married/Sep (%d)\n", status); L["MFS"] = 1; break;
  case HEAD_OF_HOUSEHOLD: 	fprintf(outfile,"Status = Head_of_Household (%d)\n", status); L["HOH"] = 1; break;
  case WIDOW: 		  	fprintf(outfile,"Status = Widow(er) (%d)\n", status); L["QW"] = 1; break;
 }
 fprintf(outfile,"\nStep-2 fill-in box %d\n", status );

 /* Exemptions. */
 get_parameter( infile, 's', word, "L6" );	/* Are you a dependent? (yes/No). */
 get_parameter( infile, 'b', &j, "L6"); 
 L[6] = j;
 if (L[6] == 0) fprintf(outfile," L6 = no\n"); else fprintf(outfile," L6 = yes, (check box on line 6).\n");

 if ((status==SINGLE) || (status==MARRIED_FILLING_SEPARAT) || (status==HEAD_OF_HOUSEHOLD))
  iline7 = 1;  else  iline7 = 2;
 if (L[6] != 0.0) iline7 = 0; /* <-- Possible exceptions here. */
 L[7] = 108.0 * iline7;
 L["7m"] = iline7;
 showline(7);

 get_parameter( infile, 's', word, "L8" );	/* Blind?, 1 if you or spouse, 2 if both. */
 get_parameter( infile, 'i', &iline8, "L8" );
 L[8] = iline8 * 108.0;
 L["8m"] = iline8;
 showline(8);

 get_parameter( infile, 's', word, "L9" );	/* Senior?, 1 if you or spouse, 2 if both. */
 get_parameter( infile, 'i', &iline9, "L9" );
 L[9] = iline9 * 108.0;
 L["9m"] = iline9;
 showline(9);

 get_parameter( infile, 's', word, "L10" );  /* Number of Dependents. */
 get_parameter( infile, 'i', &iline10, "L10"); 
 L[10] = iline10 * 333.0;
 L["10m"] = iline10;
 showline(10);

 L[11] = L[7] + L[8] + L[9] + L[10];
 showline_wmsg(11,"Exemption amount");

 /* Taxable Income. */
 GetLineF( "L12", &L[12] );	/* State Wages (W2 box 16). */

 L[13] = PrelimFedReturn.fedline[37];	/* Fed Wages (Fed 1040 line 37). */


 /* -- Sched540 Part II -- */

 sched540[38] = PrelimFedReturn.schedA[29];
 sched540[39] = PrelimFedReturn.schedA[5] + PrelimFedReturn.schedA[8];
 sched540[40] = sched540[38] - sched540[39];
 GetLine( "Adj", &sched540[41] ); 
 sched540[42] = sched540[40] + sched540[41];
 switch (status)
  {
   case SINGLE:
   case MARRIED_FILLING_SEPARAT:  threshA = 176413.0;	std_ded = 3992.0;  break;
   case HEAD_OF_HOUSEHOLD:        threshA = 264623.0;	std_ded = 7984.0;  break;
   case MARRIED_FILLING_JOINTLY:
   case WIDOW:                    threshA = 352830.0;	std_ded = 7984.0;  break;
  }
 if (L[13] > threshA)
  { /*Itemized Deductions Worksheet*/
    double ws[40];
    ws[1] = sched540[42];
    ws[2] = PrelimFedReturn.schedA[4] + PrelimFedReturn.schedA[14] + PrelimFedReturn.schedA[20] + PrelimFedReturn.schedA[28];
    ws[3] = ws[1] = ws[2];
    if (ws[3] == 0.0)
     sched540[43] = ws[1];
    else
     {
      ws[4] = 0.8 * ws[3];
      ws[5] = L[13];
      ws[6] = threshA;
      ws[7] = ws[5] - ws[6];
      if (ws[7] == 0.0)
       sched540[43] = ws[1];
      else
       {
	ws[8] = 0.06 * ws[7];
	ws[9] = smallerof( ws[4], ws[8] );
	ws[10] = ws[1] - ws[9];
	sched540[43] = ws[10];
       }
     }
  } /*Itemized Deductions Worksheet*/
 else
  sched540[43] = sched540[42];
 sched540[44] = largerof( sched540[43], std_ded );

 for (j=38; j <= 44; j++)	/* Display the worksheet calculations. */
  fprintf(outfile," SchedCA540_%d = %6.2f\n", j, sched540[j] );

 L[18] = sched540[44];

 /* -- End Sched540 Part II -- */


 GetLineF( "L14", &L[14] );	/* CA Adjustments, Schedule CA 540 line 37 column B. */

 L[15] = L[13] - L[14];
 if (L[15] < 0.0) fprintf(outfile,"L15 = (%f6.2)\n", -L[15] );
 else showline(15);

 GetLineF( "L16", &L[16] );	/* CA Adjustments, Schedule CA 540 line 37 column C. */

 L[17] = L[15] + L[16];		/* CA Adjusted Gross Income (AGI). */
 showline(17);

 switch (status)
  {
   case WIDOW: 		  	
		if (iline9 == 0)		/*Under65*/
		   switch (iline10)		  /*Dependents*/
		    {
		     case 0:  min2file = 0.0;		break;
		     case 1:  min2file = 23938.0;	break;
		     default: min2file = 32263.0;	break;
		    }
		else			 	/*Over65*/
		   switch (iline10)		  /*Dependents*/
		    {
		     case 0:  min2file = 0.0;		break;
		     case 1:  min2file = 26563.0;	break;
		     default: min2file = 33223.0;	break;
		    }
	  break;
   case MARRIED_FILLING_JOINTLY: 
		if (iline9 == 0)		 /*Both Under65*/
		   switch (iline10)
		    {
		     case 0:  min2file = 25678.0;	break;
		     case 1:  min2file = 36778.0;	break;
		     default: min2file = 44105.0;	break;
		    }
		else
		if (iline9 == 1)		 /*One Over65*/
		   switch (iline10)
		    {
		     case 0:  min2file = 31078.0;	break;
		     case 1:  min2file = 39403.0;	break;
		     default: min2file = 46063.0;	break;
		    }
		else
		   switch (iline10)		 /*Both Over65*/
		    {
		     case 0:  min2file = 36478.0;	break;
		     case 1:  min2file = 44803.0;	break;
		     default: min2file = 51463.0;	break;
		    }
	  break;
    case SINGLE:  case HEAD_OF_HOUSEHOLD:
		if (iline9 == 0)		/*Under65*/
		   switch (iline10)		  /*Dependents*/
		    {
		     case 0:  min2file = 16047.0;	break;
		     case 1:  min2file = 27147.0;	break;
		     default: min2file = 35472.0;	break;
		    }
		else			 	/*Over65*/
		   switch (iline10)		  /*Dependents*/
		    {
		     case 0:  min2file = 21447.0;	break;
		     case 1:  min2file = 29772.0;	break;
		     default: min2file = 36432.0;	break;
		    }
	  break;
  }
 if (L[17] <= min2file)
  fprintf(outfile,"You may not need to file CA Taxes, due to your California Adjusted Gross Income (%6.2f <= %6.2f).\n", L[17], min2file );

 showline(18);

 L[19] = NotLessThanZero( L[17] - L[18] );
 showline_wmsg(19,"Taxable Income");		/* Taxable income. */

 /* Tax. */
 if (L[19] < 100000.00) {
  fprintf(outfile,"Fill in circle from: Tax Table.\n");
  L["TT"] = 1;
 } else {
  fprintf(outfile,"Fill in circle from: Tax Rate Schedule.\n");
  L["TRS"] = 1;
 }
 L[31] = TaxRateFunction( L[19], status );
 showline( 31 );

 if (L[13] > threshA)
  { /*Line32-Exemption-credits-worksheet*/
    double ws_a, ws_b, ws_c, ws_d, ws_e, ws_f, ws_g, ws_h, ws_i, ws_j, ws_k, ws_l, ws_m, ws_n;
    printf(" Doing AGI Limitations worksheet.\n"); 
    ws_a = L[13];
    ws_b = threshA;
    ws_c = ws_a - ws_b;
    if (status != MARRIED_FILLING_SEPARAT)
	ws_d = Round(ws_c / 2500.0);  
    else 
	ws_d = Round(ws_c / 1250.0);
    ws_e = 6.0 * ws_d;  
    ws_f = iline7 + iline8 + iline9; 
    ws_g = ws_e * ws_f;
    ws_h = L[7] + L[8] + L[9];
    ws_i = NotLessThanZero( ws_h - ws_g );
    ws_j = iline10;
    ws_k = ws_e * ws_j;
    ws_l = L[10];
    ws_m = NotLessThanZero( ws_l - ws_k );
    ws_n = ws_i + ws_m;
    fprintf(outfile," AGI Worksheet:\n   a: %6.2f\n", ws_a);
    fprintf(outfile,"  b: %6.2f\n", ws_b);
    fprintf(outfile,"  c: %6.2f\n", ws_c);
    fprintf(outfile,"  d: %6.2f\n", ws_d);
    fprintf(outfile,"  e: %6.2f\n", ws_e);
    fprintf(outfile,"  f: %6.2f\n", ws_f);
    fprintf(outfile,"  g: %6.2f\n", ws_g);
    fprintf(outfile,"  h: %6.2f\n", ws_h);
    fprintf(outfile,"  i: %6.2f\n", ws_i);
    fprintf(outfile,"  j: %6.2f\n", ws_j);
    fprintf(outfile,"  k: %6.2f\n", ws_k);
    fprintf(outfile,"  l: %6.2f\n", ws_l);
    fprintf(outfile,"  m: %6.2f\n", ws_m);
    fprintf(outfile,"  n: %6.2f\n", ws_n);
    fprintf(outfile," Your exemptions may be limited. Used Exemptions-Credits-Worksheet for Line 21.\n");
    fprintf(outfile,"   WorkSheet[n]=%6.2f (vs. L11=%6.2f)\n", ws_n, L[11] );
    L[32] = ws_n;
  }
 else  L[32] = L[11];
 showline(32);

 L[33] = NotLessThanZero( L[31] - L[32] );
 showline(33);

 GetLineF( "L34", &L[34] );
 showline(34);		/* Taxes on distributions (sched G-1 or form FTB 5870A) */

 L[35] = L[33] + L[34];
 showline(35);

 /* Special Credits. */
 GetLineF( "L40", &L[40] );	/* Nonrefundable Child + Dependent Care Expenses Credit (pg 11). */
 fprintf(outfile," "); /* Indent next entry. */
 GetLineF( "L41", &L[41] );	/* Special credit 1 */
 GetLineF( "L42", &L[42] );	/* Special credit 2 */
 GetLineF( "L43", &L[43] );	/* Special credit 3 */
 GetLineF( "L44", &L[44] );	/* Special credit 4 */
 GetLineF( "L45", &L[45] );	/* Special credit 5+ */
 GetLineF( "L46", &L[46] );	/* Nonrefundable renter's credit */

 L[47] = L[40] + L[42] + L[43] + L[44] + L[45] + L[46];
 showline(47);			/* Total credits. */

 L[48] = NotLessThanZero( L[35] - L[47] );
 showline(48);

 /* Other taxes. */
 GetLineF( "L61", &L[61] );	/* Alternative minimum tax Sched P. */

 GetLineF( "L62", &L[62] );	/* Mental Health Services Tax. */

 GetLineF( "L63", &L[63] );	/* Other taxes and credit recapture. */

 L[64] = L[48] + L[61] + L[62] + L[63];
 showline_wmsg(64,"Total Tax");	/* Total tax. */

 /* Payments. */
 GetLineF( "L71", &L[71] ); 	/* CA income tax withheld. */
 
 GetLineF( "L72", &L[72] ); 	/* Estimated tax paid. */
 
 GetLineF( "L73", &L[73] ); 	/* Realestate withholding. */
 
 GetLineF( "L74", &L[74] ); 	/* Excess SDI. */
 
 L[75] = L[71] + L[72] + L[73] + L[74];
 showline_wmsg(75,"Total Payments");

 /* Refund / Tax-Due. */
 if (L[75] > L[64])
  {
   L[91] = L[75] - L[64];
   fprintf(outfile,"L91 = %6.2f  REBATE!!!\n", L[91] );
  }
 else
  {
   L[94] = L[64] - L[75];
   fprintf(outfile,"L94 = %6.2f  YOU OWE !!!\n", L[94] );
  }
 
 fprintf(outfile,"\nSelect any charity contributions and complete\n form accordingly.\n");

 fclose(infile);
 fclose(outfile);

 outfile = fopen(ca540_xfdf,"w");
 output_xfdf_form_data(outfile, ca540_2014, L); 
 fclose(outfile);

 outfile = fopen(ca_schedule_540_xfdf,"w");
 output_xfdf_form_data(outfile, ca_schedule_540_2014, sched540); 
 fclose(outfile);

 Display_File( outfname );
 return 0;
}
