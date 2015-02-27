/************************************************************************/
/* TaxSolve_Routines.c - General purpose reusable routines for making	*/
/*  tax programs.  These routines are not specific to any particular	*/
/*  tax form or country.  This file is usually compiled-with, linked-	*/
/*  with, or included-in a form-specific program.			*/
/* 									*/
/* Copyright (C) 2003, 2004 - Aston Roberts				*/
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
/* Aston Roberts 1-1-2004	aston_roberts@yahoo.com			*/
/************************************************************************/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdlib.h>

#include <map>
#include <string>


class Lmap : public std::map<std::string, double>
{
	public:
		double &operator [](const int &key) {
			return std::map<std::string, double>::operator[](std::to_string(key));
		}
		double &operator [](const std::string &key) {
			return std::map<std::string, double>::operator[](key);
		}
};

Lmap L;

#define MAX_LINES 1000

FILE *infile=0,	 /* Main input file to be used for reading tax input data. */
     *outfile=0; /* Main output file. */
int verbose=0;	 /* Declare and set the "verbosity" flag. */


/********************************************************************************/
/* Input routines. 								*/
/********************************************************************************/

/*------------------------------------------------------------------------------*/
/* Get_Word - Read next word from input file, while ignoring any comments.	*/
/*------------------------------------------------------------------------------*/
void get_word( FILE *infile, char *word )	/* Absorb comments. */
{
 int j=0;

 do
  {  /*Absorb any leading white-space.*/
     word[j]=getc(infile); 
     if (word[j]=='{') 
      { 
       do word[j]=getc(infile); while ((word[j]!='}') && (!feof(infile)));
       word[j]=getc(infile);
      }
  } 
 while ((!feof(infile)) && ((word[j]==' ') || (word[j]=='\t') || (word[j]=='\n') || (word[j]=='\r')));
 if (word[j]=='$') word[j]=getc(infile);
 if (word[j]==';') j++;
 else
 if (word[j]=='"')
  { /* Get quoted string. */
    j = 0;
    do
     word[j++] = getc(infile);
    while ((word[j-1] != '"') && (!feof(infile)));
    if (word[j-1] == '"') j--;	/* Remove trailing quote. */
  }
 else
  { /* Normal case. */
   do {	/*Get word until white-space or ;.*/
        j++;  word[j] = getc(infile);
        if (word[j]=='{') do word[j] = getc(infile); while ((!feof(infile)) && (word[j]!='}'));
	if (word[j]==',') word[j] = getc(infile);
      } 
   while ((!feof(infile)) && ((word[j]!=' ') && (word[j]!='\t') && (word[j]!='\n') && (word[j]!=';')));
   if (word[j]==';') ungetc(word[j],infile);
  }
 word[j] = '\0';	/* Add termination character. */
 if (verbose) printf("Read: '%s'\n", word);
}


#ifdef microsoft	   /* Apparently Microsoft doesn't know of strcasecmp(), define one. */
int strcasecmp( char *str1, char *str2 )
{
 char *tstr1, *tstr2;  int i=0;
 tstr1 = (char *)malloc(strlen(str1+1)*sizeof(char));
 do { tstr1[i] = toupper(str1[i]); i++; } while (str1[i-1]!='\0');
 tstr2 = (char *)malloc(strlen(str2+1)*sizeof(char));
 i = 0;
 do { tstr2[i] = toupper(str2[i]); i++; } while (str2[i-1]!='\0');
 i = strcmp(tstr1,tstr2);
 free(tstr1); free(tstr2);
 return i;
}
int strncasecmp( char *str1, char *str2, int len )
{
 char *tstr1, *tstr2;  int i=0;
 tstr1 = (char *)malloc(strlen(str1+1)*sizeof(char));
 do { tstr1[i] = toupper(str1[i]); i++; } while ((str1[i-1]!='\0') && (i<len));
 tstr2 = (char *)malloc(strlen(str2+1)*sizeof(char));
 i = 0;
 do { tstr2[i] = toupper(str2[i]); i++; } while ((str2[i-1]!='\0') && (i<len));
 i = strcmp(tstr1,tstr2);
 free(tstr1); free(tstr2);
 return i;
}
#endif


/*------------------------------------------------------------------------------*/
/* Get Parameter - Get a single value.						*/
/*   Expect value kinds:  'i'=integer, 'f'=float, 's'=string, 'b'=boolean.	*/
/*------------------------------------------------------------------------------*/
void get_parameter( FILE *infile, char kind, void *x, char const *emssg )
{
 char word[1024], *owrd;
 int i, *ii;
 double y, *yy;

 get_word(infile, word);

 if (feof(infile))
  {
   printf("ERROR: Unexpected EOF on '%s'\n",emssg);
   if (outfile) fprintf(outfile,"ERROR: Unexpected EOF on '%s'\n",emssg);
   exit(1);
  }
 if (kind=='i')
  {
   if (sscanf(word,"%d",&i)!=1)
    {printf("ERROR: Bad integer '%s', reading %s.\n", word, emssg); fprintf(outfile,"ERROR: Bad integer '%s', reading %s.\n", word, emssg); exit(1); }
   ii = (int *)x;
   *ii = i;
  }
 else
 if (kind=='f')
  {
   if (sscanf(word,"%lf",&y)!=1) 
    {printf("ERROR: Bad float '%s', reading %s.\n", word, emssg); fprintf(outfile,"ERROR: Bad float '%s', reading %s.\n", word, emssg); exit(1); }
   yy = (double *)x;
   *yy = y;
  }
 else
 if (kind=='s')
  {
   owrd = (char *)x;
   strcpy( owrd, word );
   if (emssg[0]!='\0')
    { if (strcmp(word,emssg)!=0) 
       {printf("ERROR1: Found '%s' when expecting '%s'\n", word, emssg); fprintf(outfile,"ERROR1: Found '%s' when expecting '%s'\n", word, emssg); exit(1); } 
    }
  }
 else
 if (kind=='l')		/* Literal string. Do not check for match. */
  {
   owrd = (char *)x;
   strcpy( owrd, word );
  }
 else
 if (kind=='b')
  {
   if ((strcasecmp(word,"TRUE")==0) || (strcasecmp(word,"YES")==0) || (strcmp(word,"1")==0)) i = 1;
   else if ((strcasecmp(word,"FALSE")==0) || (strcasecmp(word,"NO")==0) || (strcmp(word,"0")==0)) i = 0;
   else {printf("ERROR: Bad boolean '%s', reading %s.\n", word, emssg); fprintf(outfile,"ERROR: Bad boolean '%s', reading %s.\n", word, emssg); exit(1);}
   ii = (int *)x;
   *ii = i;
  }
 else
  {printf("ERROR: Unknown type '%c'\n", kind); fprintf(outfile,"ERROR: Unknown type '%c'\n", kind); exit(1);}
}




/*------------------------------------------------------------------------------*/
/* Get Parameters - Get sum of list of values terminated by ";".		*/
/*   Expect value kinds:  'i'=integer, 'f'=float, 's'=string, 'b'=boolean.	*/
/*------------------------------------------------------------------------------*/
void get_parameters( FILE *infile, char kind, void *x, char const *emssg )
{
 char word[1024], *owrd;
 int i, *ii;
 double y, *yy;

 if (kind=='f') { yy = (double *)x;  *yy = 0.0; }

 get_word(infile,word);
 while (word[0]!=';')
 {
 if (feof(infile))
  {printf("ERROR: Unexpected EOF on '%s'\n",emssg); fprintf(outfile,"ERROR: Unexpected EOF on '%s'\n",emssg); exit(1);}
 if (kind=='i')
  {
   if (sscanf(word,"%d",&i)!=1)
    {printf("ERROR: Bad integer '%s', reading %s.\n", word, emssg); fprintf(outfile,"ERROR: Bad integer '%s', reading %s.\n", word, emssg); exit(1); }
   ii = (int *)x;
   *ii = i;
  }
 else
 if (kind=='f')
  {
   if (sscanf(word,"%lf",&y)!=1) 
    {printf("ERROR: Bad float '%s', reading %s.\n", word, emssg); fprintf(outfile,"ERROR: Bad float '%s', reading %s.\n", word, emssg); exit(1); }
   yy = (double *)x;
   *yy = *yy + y;
   /*  printf("	+ %f = %f\n", y, *yy); */
  }
 else
 if (kind=='s')
  {
   owrd = (char *)x;
   strcpy( owrd, word );
   if (emssg[0]!='\0')
    { if (strcmp(word,emssg)!=0)
       {printf("ERROR2: Found '%s' when expecting '%s'\n", word, emssg); fprintf(outfile,"ERROR2: Found '%s' when expecting '%s'\n", word, emssg); exit(1); }
    }
  }
 else
 if (kind=='b')
  {
   if ((strcasecmp(word,"TRUE")==0) || (strcasecmp(word,"YES")==0) || (strcmp(word,"1")==0)) i = 1;
   else if ((strcasecmp(word,"FALSE")==0) || (strcasecmp(word,"NO")==0) || (strcmp(word,"0")==0)) i = 0;
   else {printf("ERROR: Bad boolean '%s', reading %s.\n", word, emssg); fprintf(outfile,"ERROR: Bad boolean '%s', reading %s.\n", word, emssg); exit(1);}
   ii = (int *)x;
   *ii = i;
  }
 else
  {printf("ERROR: Unknown type '%c'\n", kind); fprintf(outfile,"ERROR: Unknown type '%c'\n", kind); exit(1);}
 get_word(infile,word);
 }
}




/*.......................................................................
  .     NEXT_WORD - accepts a line of text, and returns with the        .
  . next word in that text in the third parameter, the original line    .
  . is shortened from the beginning so that the word is removed.        .
  . If the line encountered is empty, then the word returned will be    .
  . empty.                                                              .
  . NEXTWORD can parse on an arbitrary number of delimiters, and it 	.
  . returns everthing that was cut away in the second parameter.	.
  . Parameters:								.
  .   line - input character string, on output shortened by word.	.
  .   pre_trash - white-space or delimiters skipped before word.	.
  .   word - output of this routine, single word, without delimiters.   .
  .   delim - list of delimiters, whitepace chars, etc..		.
  .......................................................................*/
void next_word( char *line, char *word, char const *delim )
{
 int i=0, j=0, m=0, flag=1;

 /* Eat away preceding garbage */
 while ((line[i] !='\0') && (flag))
  {
   j = 0;
   while ((delim[j] != '\0') && (line[i] != delim[j])) j = j + 1;
   if (line[i] == delim[j]) i++;
   else  flag = 0;
  }
 while ((line[i] != '\0') && (!flag))
  {
   word[m++] = line[i++];
   if (line[i] != '\0')
    {
     j = 0;
     while ((delim[j] != '\0') && (line[i] != delim[j])) j = j + 1;
     if (line[i] == delim[j]) flag = 1;
    }
  }
 /* Shorten line. */
 j = 0;
 while (line[i]!='\0') { line[j++] = line[i++]; }
 /* Terminate the char-strings. */
 line[j] = '\0';
 word[m] = '\0';
}



/************************************************************************/
/* get_date - Returns days from 1-1-1980, for use in capital gains 	*/
/* calculations to determine short/long type.				*/
/* Probably more accurate than needed.  Usually just need to know if 	*/
/* buy/sell dates differ by more or less than 1 year.			*/
/************************************************************************/
int get_date(char *word, char const *emssg )	/* Returns days from 1-1-1980. */
{ /* For use in capital gains calculations to determine short/long type. */
 char word1[500], owrd[1000];
 int month, day, year, days;

 /* Expect month-day-year, 3-3-01 */
 strcpy(owrd,word);

 next_word( word, word1, "-_/ \t");
 if (strncasecmp( word1, "Jan", 3 ) == 0)  month = 1;  else
 if (strncasecmp( word1, "Feb", 3 ) == 0)  month = 2;  else
 if (strncasecmp( word1, "Mar", 3 ) == 0)  month = 3;  else
 if (strncasecmp( word1, "Apr", 3 ) == 0)  month = 4;  else
 if (strncasecmp( word1, "May", 3 ) == 0)  month = 5;  else
 if (strncasecmp( word1, "Jun", 3 ) == 0)  month = 6;  else
 if (strncasecmp( word1, "Jul", 3 ) == 0)  month = 7;  else
 if (strncasecmp( word1, "Aug", 3 ) == 0)  month = 8;  else
 if (strncasecmp( word1, "Sep", 3 ) == 0)  month = 9;  else
 if (strncasecmp( word1, "Oct", 3 ) == 0)  month = 10;  else
 if (strncasecmp( word1, "Nov", 3 ) == 0)  month = 11;  else
 if (strncasecmp( word1, "Dec", 3 ) == 0)  month = 12;  else
 if ((sscanf(word1,"%d",&month)!=1) || (month>12))
  {printf("ERROR: Bad month '%s' on '%s'\n",owrd,emssg); fprintf(outfile,"ERROR: Bad month '%s' on '%s'\n",owrd,emssg); exit(1);}
 next_word( word, word1, "-_/ \t");
 if ((sscanf(word1,"%d",&day)!=1) || (day>31))
  {printf("ERROR: Bad day '%s' on '%s'\n",owrd,emssg); fprintf(outfile,"ERROR: Bad day '%s' on '%s'\n",owrd,emssg); exit(1);}
 next_word( word, word1, "-_/ \t");
 if (sscanf(word1,"%d",&year)!=1) 
  {printf("ERROR: Bad year '%s' on '%s'\n",owrd,emssg); fprintf(outfile,"ERROR: Bad year '%s' on '%s'\n",owrd,emssg); exit(1);}
 if (year>99) year = year - 1900;  /* Handle case where four-digit year was specified. */
 if (year<80) year = year + 100;   /* Y-2k, assume years less than 80 are in 2000's, not 1900's. */
 
 if ((year<85) || (year>115)) printf("Warning:  Unusual year in '%s' .  Use mm-dd-yy date like 5-23-02.\n", owrd);

 switch (month)
  {
   case 1: days = 0; break;
   case 2: days = 31; break;
   case 3: days = 59; break;
   case 4: days = 90; break;
   case 5: days = 120; break;
   case 6: days = 151; break;
   case 7: days = 181; break;
   case 8: days = 212; break;
   case 9: days = 243; break;
   case 10: days = 273; break;
   case 11: days = 304; break;
   case 12: days = 334; break;
   default: printf("ERROR: Bad month '%d'\n",month); fprintf(outfile,"ERROR: Bad month '%d'\n",month); exit(1); break;
  } 

 /* Assumes all years have 365-days. */
 days = days + day + 365 * (year - 80) - 1;
 return days;
}




void read_line( FILE *infile, char *line )
{
 int j=0;
 do  line[j++] = getc(infile);  while ((!feof(infile)) && (line[j-1] != '\n'));
 line[j-1] = '\0';
}


/* Show a line-number and it's value. */
void showline( int j )
{ fprintf(outfile, "L%d = %6.2f\n", j, L[j]); }

/* Show an integer valued line. */
void shownum( int j )
{ fprintf(outfile, "L%d = %d\n", j, (int)L[j]); }

/* Show line only if non-zero. */	/* Depricated in favor of ShowLineNonZero (clearer name). */
void ShowLine( int j )	
{ if (L[j]!=0) showline( j ); }

/* Show line only if non-zero. */
void ShowLineNonZero( int j )
{ if (L[j]!=0) showline( j ); }

/* Show-Line with a message. */
void showline_wmsg( int j, char const *msg )	
{ fprintf(outfile,"L%d = %6.2f\t\t%s\n", j, L[j], msg); }

/* Show line only if non-zero. */
void ShowLineNonZero_wMsg( int j, char *msg )
{ if (L[j]!=0) showline_wmsg( j, msg ); }

/* For worksheet calculations, indent and show special line character. */
void showline_wrksht( char wrksht, int j, double *x )
{ fprintf(outfile," %c%d = %6.2f\n", wrksht, j, x[j]); }

/* Show-line with specified label and value. */
void showline_wlabel( char const *label, double value )
{ fprintf(outfile, "%s = %6.2f\n", label, value ); }

/* Show-line with specified label and value. */
void showline_wlabelmsg( char *label, double value, char *msg )
{ fprintf(outfile, "%s = %6.2f\t\t%s\n", label, value, msg ); }


int Round( double x )
{ int y; if (x<0.0) y = x - 0.5; else y = x + 0.5;  return y; }


/* Get a line value. */
void GetLine( char const *linename, double *value )
{
 char word[500];
 get_parameter( infile, 's', word, linename);
 get_parameters( infile, 'f', value, linename);
}

/* Get a single line value. */
void GetLine1( char const *linename, double *value )
{
 char word[500];
 get_parameter( infile, 's', word, linename);
 get_parameter( infile, 'f', value, linename);
}

/* Get a line value, and print it to file. */
void GetLineF( char const *linename, double *value )
{
 GetLine( linename, value );
 fprintf(outfile, "%s = %6.2f\n", linename, *value );
}



double smallerof( double a, double b ) { if (a<b) return a; else return b; }
double largerof( double a, double b )  { if (a>b) return a; else return b; }
double NotLessThanZero( double a )    { if (a<0.0) return 0.0; else return a; }

double absolutev( double val ) { if (val >= 0.0)  return val;  else  return -val; }
		/* Convenience function - avoids needing to link with math-lib merely to get fabs(). */


void Display_File( char *filename )
{
 FILE *infile;
 char line[500];

 infile = fopen(filename,"r");
 if (infile==0) {printf("Could not open %s\n", filename); return;}
 fgets(line, 500, infile);
 while (!feof(infile))
  {
   printf("%s", line);
   fgets(line, 500, infile);
  } 
 fclose(infile);
}




/*------------------------------------------------------------------------------*/
/* Get_Comment - Read next Comment, if anym from input file.			*/
/*------------------------------------------------------------------------------*/
void get_comment( FILE *infile, char *word )
{
 int j=0;

 do  /*Absorb any leading white-space.*/
     word[j] = getc(infile); 
 while ((!feof(infile)) && ((word[j]==' ') || (word[j]=='\t') || (word[j]=='\n') || (word[j]=='\r')));
 if (word[j] == '{')
  {
   do  /*Get words until end of comment.*/
       word[j++] = getc(infile);
   while ((!feof(infile)) && (word[j-1] != '}'));
   word[j] = '\0';
  }
 else
  {
   ungetc(word[j], infile);
   word[0] = '\0';
  }
 if (verbose) printf("Read Coment: {%s}\n", word);
}

enum form_flags {
	DOLLAR_AND_CENTS,
	DOLLAR_AND_CENTS_ONE,
	DOLLAR_ONLY,
	USE_KEY_IN_FORM,
	IF_SET,
};

struct xfdf_form_translation {
	const char *line;
	enum form_flags flags;
	const char *pdf_line_dollar;
	const char *pdf_line_cents;
};

void output_xfdf_form_data(FILE *out, struct xfdf_form_translation *form, Lmap &lines)
{
	int i=0;

	fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(out, "<xfdf xmlns=\"http://ns.adobe.com/xfdf/\">\n");
	fprintf(out, "\t<fields>\n");

	while (form[i].line != NULL) {
		double dollar = 0.0, cents = 0.0;

		if (lines.count(form[i].line) == 1) {

			if (form[i].flags == DOLLAR_ONLY || form[i].flags == DOLLAR_AND_CENTS) {
				dollar = (int)lines[form[i].line];
				fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%.f</value>\n", dollar);
				fprintf(out,"\t\t</field>\n");
			}

			if (form[i].flags == DOLLAR_AND_CENTS) {
				cents = (lines[form[i].line]*100.0 - (int)dollar*100.0);
				fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_cents);
				fprintf(out,"\t\t\t<value>%02.f</value>\n", cents);
				fprintf(out,"\t\t</field>\n");
			}

			if (form[i].flags == DOLLAR_AND_CENTS_ONE) {
				fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%.2f</value>\n", lines[form[i].line]);
				fprintf(out,"\t\t</field>\n");
			}
		}

		if (form[i].flags == USE_KEY_IN_FORM && lines.count(form[i].line) == 1) {
			fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
			fprintf(out,"\t\t\t<value>%s</value>\n", form[i].line);
			fprintf(out,"\t\t</field>\n");
		}

		if (form[i].flags == IF_SET && lines.count(form[i].line) == 1) {
			fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
			fprintf(out,"\t\t\t<value>%s</value>\n", form[i].pdf_line_cents);
			fprintf(out,"\t\t</field>\n");
		}

		i++;
	}
	fprintf(out, "\t</fields>\n");
	fprintf(out, "</xfdf>");;

}

