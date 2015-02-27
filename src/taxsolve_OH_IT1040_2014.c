/************************************************************************/
/* TaxSolve_OH_IT1040_2014.c - 						*/
/* Copyright (C) 2015 - Aston Roberts					*/
/* 									*/
/* Compile:   gcc taxsolve_OH_IT1040_2014.c -o taxsolve_OH_IT1040_2014	*/
/* Run:	      ./taxsolve_OH_IT1040_2014  OH_IT1040_2014.txt 		*/
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
/* Aston Roberts 1-8-2015	aston_roberts@yahoo.com			*/
/************************************************************************/

#include <stdio.h>
#include <time.h>

#include "taxsolve_routines.c"
#include "taxsolve_OH_IT1040_2014_forms.h"

double thisversion=12.0;

#define SINGLE 		        1
#define MARRIED_FILLING_JOINTLY 2
#define MARRIED_FILLING_SEPARAT 3
#define HEAD_OF_HOUSEHOLD       1

double TaxRateFunction( double x, int status )
{
 if (x <  5200.0) return    x * 0.00528; else
 if (x < 10400.0) return    27.46 + (x-5200.0) 	 * 0.01057; else
 if (x < 15650.0) return    82.42 + (x-10400.0)  * 0.02113; else
 if (x < 20900.0) return   193.35 + (x-15650.0)  * 0.02642; else
 if (x < 41700.0) return   332.06 + (x-20900.0)  * 0.03169; else
 if (x < 83350.0) return   991.21 + (x-41700.0)  * 0.03698; else
 if (x < 104250.0) return 2531.43 + (x-83350.0)  * 0.04226; else
 if (x < 208500.0) return 3414.66 + (x-104250.0) * 0.04906;
 else 		   return 8529.17 + (x-208500.0) * 0.05333;
}



/*----------------------------------------------------------------------------*/

int main( int argc, char *argv[] )
{
 int i, j, k;
 char word[1000], outfname[1000], it1040_xfdf_outfname[1000];
 int status=0, exemptions=0, qualify_jfc=0;
 time_t now;
 double factor62, factor67, limit;
 double L35a=0.0, L35b=0.0, L35c=0.0, L35d=0.0, L35e=0.0, L35f=0.0, L35g=0.0;
 double L37a=0.0, L37b=0.0, L39a=0.0, L39b=0.0;
 double L40a=0.0, L40b=0.0, L40c=0.0;
 double L44a=0.0, L44b=0.0;
 double L46a=0.0, L46b=0.0, L46c=0.0;
 double L72a=0.0, L72b=0.0, L72c=0.0, L72d=0.0, L72e=0.0;
 double jfc, exemption_amnt;

 /* Intercept any command-line arguments. */
 printf("OH IT1040 2014 - v%3.1f\n", thisversion);
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
    strcpy(it1040_xfdf_outfname,argv[i]);
    j = strlen(outfname)-1;
    while ((j>=0) && (outfname[j]!='.')) j--;
    if (j<0) {
     strcat(outfname,"_out.txt");
     strcat(it1040_xfdf_outfname,"_it1040.xfdf");
    } else {
     strcpy(&(outfname[j]),"_out.txt");
     strcpy(&(it1040_xfdf_outfname[j]),"_it1040.xfdf");
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
  }

 /* Accept parameters from input file. */
 /* Expect  OH IT1040 lines, something like:
	Title:  OH IT1040 1999 Return
	L1		{Wages}
*/

 /* Accept Form's "Title" line, and put out with date-stamp for records. */
 read_line( infile, word );
 now = time(0);
 fprintf(outfile,"\n%s		%s\n", word, ctime( &now ));

 /* get_parameter(infile, kind, x, emssg ) */
 get_parameter( infile, 's', word, "Status" );
 get_parameter( infile, 'l', word, "Status ?");
 if (strncasecmp(word,"Single",4)==0) { status = SINGLE; L["Single"] = 1; } else
 if (strncasecmp(word,"Married/Joint",11)==0) { status = MARRIED_FILLING_JOINTLY; L["MFJ"] = 1; }else
 if (strncasecmp(word,"Married/Sep",11)==0) { status = MARRIED_FILLING_SEPARAT; L["MFS"] = 1; }else
 if (strncasecmp(word,"Head_of_House",4)==0) { status = HEAD_OF_HOUSEHOLD; L["HOH"] = 1; }
 else
  { 
   printf("Error: unrecognized status '%s'. Must be: Single, Married/joint, Married/sep, Head_of_house.\nExiting.\n", word); 
   fprintf(outfile,"Error: unrecognized status '%s'. Must be: Single, Married/joint, Married/sep, Head_of_house.\nExiting.\n", word); 
   exit(1); 
  }
 fprintf(outfile,"Status = %s (%d)\n", word, status);

 GetLine( "L1", &L[1] );	/* Federal Adjusted Gross Income */

 get_parameter( infile, 's', word, "Exemptions" );	/* Exemptions, self/depend. */
 get_parameters( infile, 'i', &exemptions, "Exemptions"); 

 /* Answer YES only if Married Filing Jointly, and you and your spouse */
 /* each have qualifying Ohio adjusted gross income of at least $500. */
 get_parameter( infile, 's', word, "JointCredit" );
 get_parameter( infile, 'b', &qualify_jfc, "JointCredit ?"); 
 
 GetLine( "L14", &L[14] );	/* Earned income credit (see the worksheet on page 20). */
 GetLine( "L15", &L[15] );	/* Ohio adoption credit ($1,500 per child adopted during the year). */
 GetLine( "L16", &L[16] );	/* Manufacturing equipment grant. */
 GetLine( "L19", &L[19] );	/* Unpaid Ohio Use Tax */
 GetLine( "L21", &L[21] );	/* Ohio Tax Withheld (box 17 on your W-2) */
 GetLine( "L22", &L[22] );	/* 1040ES payments, IT 40P extension payments for 2014, 2013 overpayments ... */

 /* Additions - add to extent not included in fed adjusted gross income (line 1). */
 GetLine( "L33", &L[33] );	/* Non-Ohio state or local gov't interest, dividends. */
 GetLine( "L34", &L[34] );	/* Pass-through Entity addback. */

 GetLine( "L35a", &L35a );	/* Fed int+div subject to state tax & misc. fed adj. */
 L["35a"] = L35a;
 GetLine( "L35b", &L35b );	/* Reimbursed college tuit. fees deducted prev yrs. */
 L["35b"] = L35b;
 GetLine( "L35c", &L35c );	/* Losses, sale of Ohio Public Obligations. */
 L["35c"] = L35c;
 GetLine( "L35d", &L35d );	/* Nonmedical withdrawals OH Med. Savings Acct. */
 L["35d"] = L35d;
 GetLine( "L35e", &L35e );	/* Reimb. exp. prev. deducted, if reimb. not in FAGI .*/
 L["35e"] = L35e;
 GetLine( "L35f", &L35f );	/* Lump sum distribution add-back + misc. fed income tax adjustments */
 L["35f"] = L35f;
 GetLine( "L35g", &L35g );	/* Adjustment for IRC section 168(k) and 179 depreciation expense */
 L["35g"] = L35g;
 L[35] = L35a + L35b + L35c + L35d + L35e + L35f + L35g;

 /* Deductions - see limitations in instructions. */
 GetLine( "L37a", &L37a );	/* Fed int + div exempt from state taxation. */
 L["37a"] = L37a;
 GetLine( "L37b", &L37b );	/* Adjustment for Internal Revenue Code sections 168(k) + 179 deprec. expense */
 L["37b"] = L37b;
 L[37] = L37a + L37b;
 GetLine( "L38", &L[38] );	/* Employee compensation earned in OH by non-residents. */
 GetLine( "L39a", &L39a );	/* Military pay. */
 L["39a"] = L39a;
 GetLine( "L39b", &L39b );	/* Military Retirement Income. */
 L["39b"] = L39b;
 L[39] = L39a + L39b;
 GetLine( "L40a", &L40a );	/* State/municipal income tax overpayments. */
 L["40a"] = L40a;
 GetLine( "L40b", &L40b );	/* Refund or reimbursements shown on IRS form 1040, line 21 */
 L["40b"] = L40b;
 GetLine( "L40c", &L40c );	/* Repayment of income reported in a prior year, ... */
 L["40c"] = L40c;
 L[40] = L40a + L40b + L40c;
 GetLine( "L41", &L[41] );	/* Small business investor income deduction */
 GetLine( "L42", &L[42] );	/* Disab. & survivor benef. */
 GetLine( "L43", &L[43] );	/* Qualifying soc. sec. + railroad benefits. */ 
 GetLine( "L44a", &L44a );	/* Education: Ohio 529 contributions */ 
 L["44a"] = L40a;
 GetLine( "L44b", &L44b );	/* Education: Ohio 529 contributions */ 
 L["44b"] = L40b;
 L[44] = L44a + L44b;
 GetLine( "L45", &L[45] );	/* Ohio National Guard reimbursements */
 GetLine( "L46a", &L46a );	/* Unreimbursed health insurance ... */
 L["46a"] = L40a;
 GetLine( "L46b", &L46b );	/* Funds deposited into, and earnings of, a medical savings account */
 L["46b"] = L40b;
 GetLine( "L46c", &L46c );	/* Qualified organ donor expenses */
 L["46c"] = L40c;
 L[46] = L46a + L46b + L46c;
 GetLine( "L47", &L[47] );	/* Wage expense not deducted */
 GetLine( "L48", &L[48] );	/* Interest income from Ohio Public Obligations ... */

 GetLine( "L51", &L[51] );	/* Retirement Income Credit (see instructions for credit table) (Limit $200) */
 if (L[51] > 200.0) L[51] = 200.0;
 GetLine( "L52", &L[52] );	/* Senior Citizen Credit (Limit $50 per return) */
 if (L[52] > 50.0) L[52] = 50.0;
 GetLine( "L53", &L[53] );	/* Lump Sum Distribution Credit (you must be 65 years of age or older to claim this credit) */
 GetLine( "L54", &L[54] );	/* Child and Dependent Care Credit (see instructions and worksheet) */
 GetLine( "L55", &L[55] );	/* Lump Sum Retirement Credit */
 GetLine( "L57", &L[57] );	/* Displaced worker credit (Limit $500 single; $1,000 joint, if both spouses qualify) */
 if (status == MARRIED_FILLING_JOINTLY)
  limit = 2.0 * 500.0;
 else
  limit = 500.0;
 L[57] = smallerof( L[57], limit );

 GetLine( "L58", &L[58] );	/* Ohio Political Contributions Credit (Limit $50 single; $100 joint) */
 if (status == MARRIED_FILLING_JOINTLY)
  limit = 2.0 * 50.0;
 else
  limit = 50.0;
 L[58] = smallerof( L[58], limit );

 GetLine( "L60", &L[60] );	/* Portion of line 3 subjected to tax by other states. */
 GetLine( "L63", &L[63] );	/* 2014 income tax paid to other states ... */

 /* For non-residents only. */
 GetLine( "L65", &L[65] );	/* Portion Ohio Adj. Gross Income not earned in Oh. */

 GetLine( "L68", &L[68] );	/* Nonrefundable Business Credits, Sched-E line 10 */

 GetLine( "L72a", &L72a );	/* Refundable Business Jobs Credits, */
 L[72] = L72a;
 GetLine( "L72b", &L72b );	/* Refundable Pass-through Entity Credits, */
 L["72b"] = L72b;
 GetLine( "L72c", &L72c );	/* Refundable Histroic Preservation Credits, */
 L["72c"] = L72c;
 GetLine( "L72d", &L72d );	/* Refundable Motion Picture Credits */
 L["72d"] = L72d;
 GetLine( "L72e", &L72e );	/* Financial Institutions Tax (FIT) credit */
 L["72e"] = L72e;
 L[73] = L72a + L72b + L72c + L72d + L72e;
 L[23] = L[73];



 /* ---- Do Calculations. ---- */

 L[36] = L[33] + L[34] + L[35];

 for (j = 37; j <= 48; j++)
  L[49] = L[49] + L[j];

 L[50] = L[36] - L[49];
 L[2] = L[50];

 L[3] = L[1] + L[2];

 if (L[3] <= 40000.0)
  exemption_amnt = 2200.0;
 else
 if (L[3] <= 80000.0)
  exemption_amnt = 1950.0;
 else
  exemption_amnt = 1700.0;
 L[4] = exemption_amnt * exemptions;

 L[5] = NotLessThanZero( L[3] - L[4] );
 L[6] = TaxRateFunction( L[5], status );

 if (L[5] <= 10000.0) L[56] = 88.0;

 for (j = 51; j <= 58; j++)
  L[59] = L[59] + L[j];
 L[7] = L[59];

 L[8] = NotLessThanZero( L[6] - L[7] );

 if (L[5] < 30000)
  L[9] = 20.0 * exemptions;

 L[10] = NotLessThanZero( L[8] - L[9] );

 if ((status == MARRIED_FILLING_JOINTLY) && (qualify_jfc))
  { /*Joint_Filing_Credit*/
    if (L[5] < 25000) jfc = 0.20;
    else
    if (L[5] < 50000) jfc = 0.15;
    else
    if (L[5] < 75000) jfc = 0.10;
    else jfc = 0.05;
    L[11] = smallerof( jfc * L[10], 650.0 );
    L["11p"] = jfc*100.0;
  } /*Joint_Filing_Credit*/

 L[12] = L[10] - L[11];

 /* L[13] computed below, which depends on L[71] and the following prior lines. */

 L[61] = L[3];
 factor62 = 0.0001 * (int)(10000.0 * (L[60] / L[61]));	/* 4-digits right of decimal-pt. */
 L["62p"] = factor62;
 L[62] = factor62 * L[12];
 L[64] = smallerof( L[62], L[63] );
 L[69] = L[64];

 L[66] = L[3];
 factor67 = 0.0001 * (int)(10000.0 * (L[65] / L[66]));  /* 4-digits right of decimal-pt. */
 L["67p"] = factor67;
 L[67] = factor67 * L[12];
 L[70] = L[67];

 L[71] = L[68] + L[69] + L[70];
 L[13] = L[71];

 /* Resuming now with calculations at line 17. */
 L[17] = NotLessThanZero( L[12] - (L[13] + L[14] + L[15] + L[16]) );
 L[20] = L[17] + L[18] + L[19];
 L[24] = L[21] + L[22] + L[23];

 if ((status == SINGLE) && (L[1] <= 12200.0) && (L[50] == 0.0))
  fprintf(outfile, "You do not need to file Ohio tax return (Fed AGI < minimum).\n");

 if ((status == MARRIED_FILLING_JOINTLY) && (L[1] <= 14400.0) && (L[50] == 0.0))
  fprintf(outfile, "You do not need to file Ohio tax return (Fed AGI < minimum).\n");

 if ((L[4] >= L[3]) && (L[50] == 0.0))
  fprintf(outfile, "You do not need to file Ohio tax return (L[4] >= L[3]).\n");


 /* Output the Results. */

 showline(1);
 showline(2);
 showline(3);
 fprintf(outfile,"Exemptions = %d\n", exemptions );
 for (j = 4; j <= 10; j++)
  showline(j);
 fprintf(outfile, "L10a = %6.2f\n", L[10]);
 for (j = 11; j <= 19; j++)
  showline(j);
 showline_wmsg( 20, "Total Ohio Tax" );
 showline_wmsg( 21, "Amount Withheld" );
 showline(22);
 showline(23);
 showline_wmsg( 24, "Total Payments" );
 if (L[24] > L[20])
  {
   L[25] = L[24] - L[20];
   showline_wmsg( 25, "AMOUNT OVERPAID" );
  }
 else
  {
   L[29] = L[20] - L[24];
   showline_wmsg( 29, "AMOUNT DUE" );
  }
 showline(33);
 showline(34);
 showline_wlabel( "L35a", L35a );
 showline_wlabel( "L35b", L35b );
 showline_wlabel( "L35c", L35c );
 showline_wlabel( "L35d", L35d );
 showline_wlabel( "L35e", L35e );
 showline_wlabel( "L35f", L35f );
 showline_wlabel( "L35g", L35g );
 showline_wmsg( 36, "Total additions" );
 showline_wlabel( "L37a", L37a );
 showline_wlabel( "L37b", L37b );
 showline(38);
 showline_wlabel( "L39a", L39a );
 showline_wlabel( "L39b", L39b );
 showline_wlabel( "L40a", L40a );
 showline_wlabel( "L40b", L40b );
 showline_wlabel( "L40c", L40c );
 showline(41);
 showline(42);
 showline(43);
 showline_wlabel( "L44a", L44a );
 showline_wlabel( "L44b", L44b );
 showline(45);
 showline_wlabel( "L46a", L46a );
 showline_wlabel( "L46b", L46b );
 showline_wlabel( "L46c", L46c );
 showline(47);
 showline(48);
 showline_wmsg( 49, "Total Deductions" );
 showline_wmsg( 50, "Net adjustments" );
 for (j = 51; j <= 58; j++)
  showline(j);
 showline_wmsg(59,"Total Schedule B Credits");
 showline(60);
 showline(61);
 fprintf(outfile," (L62 [%g]) \t", factor62 );
 for (j = 62; j <= 66; j++)
  showline(j);
 fprintf(outfile," (L67 [%g]) \t", factor67 );
 for (j = 67; j <= 73; j++)
  showline(j);

 fclose(infile);
 fclose(outfile);

 outfile = fopen(it1040_xfdf_outfname,"w");
 output_xfdf_form_data(outfile, it1040_2014, L); 
 fclose(outfile);

 Display_File( outfname );
 printf("\nResults written to file:  %s\n", outfname);
 return 0;
}
