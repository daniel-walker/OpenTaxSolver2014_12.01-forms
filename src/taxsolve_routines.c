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
#include <math.h>

#include <map>
#include <string>


class Lmap : public std::map<std::string, double>
{
	public:
		double &operator [](const int &key) {
			return std::map<std::string, double>::operator[](std::to_string((long long int)key));
		}
		double &operator [](const std::string &key) {
			return std::map<std::string, double>::operator[](key);
		}
};

Lmap L;
Lmap StateL;
Lmap null_lmap;

char name1FirstMid[4096], name1Last[4096], name2FirstMid[4096], name2Last[4096];
char address1[4096], addressAptNo[4096], address2[4096];
char addressTown[64],addressState[64],addressZip[64];
char socsec1[4096], socsec2[4096];
char prez1[4096], prez2[4096];
char routingnum[4096], accountnum[4096], accounttype[4096];
char dob1[4096], dob2[4096];
char occupation[4096], spouse_occupation[4096];
char email[4096], phone[4096], phonearea[4096], phonenum[4096];

#define MAX_LINES 1000
#define CHAR_DOUBLEQUOTE	'"'
#define CHAR_SINGLEQUOTE	'\''
#define CHAR_NULL		'\0'

FILE *infile=0,	 /* Main input file to be used for reading tax input data. */
     *outfile=0; /* Main output file. */
int verbose=0;	 /* Declare and set the "verbosity" flag. */


/********************************************************************************/
/* Input routines. 								*/
/********************************************************************************/

static void Convert_slashes( char *fname )
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

char
char_skipping_comment(FILE* _infile)
{
  char	cc		= '\0';
  //  bool	inComment	= false;
  do
    {  /*Absorb any leading white-space.*/
      if(feof(_infile)) return '\0';
      cc	= getc(_infile); 
      if(cc == '{') 
	{ 
	  if(feof(_infile)) return '\0';
	  // inComment	= true;
	  do	cc	= getc(_infile);
	  while ((cc != '}') && !feof(_infile));
	  if(cc == '}')
	    {
	      // inComment	= false;
	      if(feof(_infile))	return '\0';
	      cc	= getc(_infile);
	    }
	}
    } 
  while(((cc == ' ') || (cc == '\t') || (cc == 10) || (cc == 13)) && !feof(_infile));
  if(cc == '\0')
    {
      /* should not happen */
    }
  else if((cc == ' ') || (cc == '\t') || (cc == 10) || (cc == 13))
    {
      /* must be that feof(infile) */
    }
  else
    {
      return cc;
      ungetc(cc, infile);
    }
  return '\0';
}

/*------------------------------------------------------------------------------*/
/* New_Get_Word - Read next word from input file, while ignoring any comments.	*/
/*------------------------------------------------------------------------------*/
void new_get_word( FILE *infile, char *word )	/* Absorb comments. */
{
  int 	jj	= 0;
  word[0]       = char_skipping_comment(infile);
  if(word[0] == '\0') 
    return;
  if(word[0] == '$') 
    {
      if(feof(infile))
	{
	  word[0]	= '\0';
	  return;
	}
      word[0]		= getc(infile);
    }

 if( word[0] == ';')
   {
     word[1] = '\0';	/* Add termination character. */
     if (verbose) printf("Read: '%s'\n", word);
     return;
   }

 if(word[0] == CHAR_DOUBLEQUOTE)	/* Get quoted string. */
   { 
     for(jj = 0; !feof(infile); jj++)
       {
	 word[jj]	= getc(infile);
	 if(word[jj] == CHAR_DOUBLEQUOTE) break;
       }
     if(jj && (word[jj] == CHAR_DOUBLEQUOTE))	/* Remove trailing quote, even if input had opening but no closing quote */
       word[jj]		= '\0';
     if (verbose) printf("Read: '%s'\n", word);
     return;
   }

 if(1)
  { /* Normal case. */
    for(jj = 1; !feof(infile); )	/*Get word until white-space or ;.*/
      {
	word[jj]	= getc(infile);
	if(word[jj] == ' ')	break;
	if(word[jj] == '\t')	break;
	if(word[jj] == 10)	break;
	if(word[jj] == ';')
	  {
	    ungetc(word[jj],infile);
	    break;
	  }
	if(word[jj] == '{')
	  {
	    while(!feof(infile))
	      if(getc(infile) == '}')
		break;
	    word[jj] = '\0';
	  }
	else jj++;
      }
    word[jj]	= '\0';	/* Add termination character. */
    if (verbose) printf("Read: '%s'\n", word);
  }
}

/* return true if default is found, and read past semicolon */
bool
get_parameters_or_default( FILE* infile, char kind, void* resultPtr, const char* emssg, const char* _defString)
{
  char		word[1024];

  if (kind=='f')	(*(double*)resultPtr)	= 0.0;

  new_get_word(infile, word);

  if(strncmp(_defString, word, strlen(_defString)) == 0)
    {
      char	nextChar	= getc(infile);
      // fprintf(stderr, "get_parameters_or_default: INFO: found char(%c) after word(%s)\n", nextChar, _defString);
      if(nextChar != ';') ungetc(nextChar, infile);
      return true;
    }

  while(word[0] != ';')
    {
      if (feof(infile)) {printf("ERROR: get_parameters_or_default: Unexpected EOF on '%s'\n",emssg); exit(1);}
      if(0);
      else if(kind == 'i')
	{
	  int	ival	= 0;
	  if (sscanf(word,"%d", &ival)!=1) {printf("ERROR: get_parameters_or_default: Bad integer '%s', reading %s.\n", word, emssg); exit(1); }
	  (*(int*) resultPtr)	= ival;
	}
      else if(kind=='f')
	{
	  double	dval	= 0.0;
	  if (sscanf(word,"%lf", &dval)!=1) {printf("ERROR: get_parameters_or_default: Bad float '%s', reading %s.\n", word, emssg); exit(1); }
	  (*(double *)resultPtr)		+= dval;
	  /*  printf("	+ %f = %f\n", y, *yy); */
	}
      else if(kind=='s')
	{
	  strcpy( (char*) resultPtr, word );
	  if (emssg[0]!='\0')
	    { if (strcmp(word,emssg)!=0) {printf("ERROR2: get_parameters_or_default: Found '%s' when expecting '%s'\n", word, emssg); exit(1); } }
	}
      else if(kind=='b')
	{
	  int	ival	= 0;
	  if ((strcasecmp(word,"TRUE")==0) || (strcasecmp(word,"YES")==0) || (strcmp(word,"1")==0))		ival	= 1;
	  else if ((strcasecmp(word,"FALSE")==0) || (strcasecmp(word,"NO")==0) || (strcmp(word,"0")==0))	ival	= 0;
	  else {printf("ERROR: get_parameters_or_default: Bad boolean '%s', reading %s.\n", word, emssg); exit(1);}
	  (*(int *)resultPtr)	= ival;
	}
      else
	{printf("ERROR: get_parameters_or_default: Unknown type '%c'\n", kind); exit(1);}
      new_get_word(infile,word);
    }
  return false;
}

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
void showline_format( int j , const char* _fmt)	
{
  char	fmt[128];
  strcpy(fmt, "L%d = ");
  strcat(fmt, _fmt);
  strcat(fmt, "\n");
  fprintf(outfile, fmt, j, L[j]);
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
void showline_wmsg( int j, const char *msg )	
{ fprintf(outfile,"L%d = %6.2f\t\t%s\n", j, L[j], msg); }
void showline_wmsg_State( int j, const char *msg )	{ fprintf(outfile,"StateL%d = %6.2f		%s\n", j, StateL[j], msg); }

/* Show line only if non-zero. */
void ShowLineNonZero_wMsg( int j, const char *msg )
{ if (L[j]!=0) showline_wmsg( j, msg ); }

/* For worksheet calculations, indent and show special line character. */
void showline_wrksht( char wrksht, int j, double *x )
{ fprintf(outfile," %c%d = %6.2f\n", wrksht, j, x[j]); }

/* Show-line with specified label and value. */
void showline_wlabel( char const *label, double value )
{ fprintf(outfile, "%s = %6.2f\n", label, value ); }

/* Show-line with specified label and value. */
void showline_wlabelmsg( const char *label, double value, const char *msg )
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
void GetLineRounded( const char *linename, double *value ) { GetLine(linename, value); *value = Round(*value); }

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


/* Get a line value (if value is defString, replace it with defValue). */
void GetLineF_defaulting( const char *linename, double *value, const char* _defString, double _defValue)
{
 char word[500];
 get_parameter( infile, 'l', word, linename);
 if(strcmp(word, linename) != 0)
   {
     printf("GetLineF_defaulting: ERROR: did not find expected line %s, instead found(%s)\n", linename, word);
     exit(1);
   }

  if (_defString && (*_defString))
   {
     if(0);
     else if(get_parameters_or_default( infile, 'f', value, linename, _defString)) // if(strcmp(word,_defString) == 0)
       {
	 *value		= _defValue;
	 //	 else
	 //	   {
	 //	     printf("ERROR2: Found '%s' when expecting '%s' OR '%s'\n", word, linename, _defString);
	 //	     exit(1);
	 // }
       }
   }
  else
    {
      get_parameters( infile, 'f', value, linename);
    }
// fprintf(outfile, "%s = %6.2f\n", linename, *value );
}

/* Get a line value, round it, and print it to file. */
void GetLineRoundedF( const char *linename, double *value )
{
 GetLine( linename, value );
 *value = Round(*value);
 fprintf(outfile, "%s = %6.2f\n", linename, *value );
}

/* Get a line value (if value is defString, replace it with defValue) and round the value. */
void GetLineRoundedF_defaulting( const char *linename, double *value, const char* _defString, double _defValue)
{
  GetLineF_defaulting(linename, value, _defString, _defValue);
  *value = Round(*value);
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

/*------------------------------------------------------------------------------*/
/* Get_Text - Read next Text, if anym from input file.			*/
/*------------------------------------------------------------------------------*/
void get_text( FILE *infile, char *word )
{
 int j=0;
 word[0]	= '\0';

 if (true) printf("get_text:0: word=%s\n", word);

 do  /*Absorb any leading white-space.*/
     word[0] = getc(infile); 
 while ((!feof(infile)) && ((word[0]==' ') || (word[0]=='\t') || (word[0]=='\n') || (word[0]=='\r')));
 if (true) printf("get_text:1: word=%s\n", word);
 if (word[0] == ';') {
   word[0] = '\0';
   if (true) printf("get_text:1a: Read Text: {%s}\n", word);
   return;
 }
 do  /*Get words until semicolon.*/
   word[++j] = getc(infile);
 while ((!feof(infile)) && (word[j] != ';'));
 word[j] = '\0';
 if (true) printf("get_text:1b: Read Text: {%s}\n", word);
 for(j = strlen(word) - 1; j >= 0; j--)
   {
	 if((word[j]==' ') || (word[j]=='\t') || (word[j]=='\n') || (word[j]=='\r')) {
	   word[j]	= '\0';
	 }
	 else break;
   }
 if (true) printf("get_text:3: Read Text: {%s}\n", word);
}

enum form_flags {
	DOLLAR_AND_CENTS,
	ABS_DOLLAR_AND_CENTS,
	DOLLAR_AND_CENTS_ONE,
	DOLLAR_ONLY,
	ABS_DOLLAR_AND_CENTS_ONE,
	USE_KEY_IN_FORM,
	IF_SET,
	FOUR_DIGITS,
	NAME1,
	NAME1FIRSTMID,
	NAME1LAST,
	NAME2,
	NAME2FIRSTMID,
	NAME2LAST,
	NAME1_AND_NAME2,
	ADDRESS1,
	ADDRESSAPTNO,
	ADDRESS2,
	SOCSEC1,
	SOCSEC2,
	PREZ1,
	PREZ2,
	ROUTINGNUM,
	ACCOUNTNUM,
	ACCOUNTTYPE_CHECKING,
	ACCOUNTTYPE_SAVINGS,
	DOB1,
	DOB2,
	ADDRESSTOWN,
	ADDRESSSTATE,
	ADDRESSZIP,
	OCCUPATION,
	SPOUSE_OCCUPATION,
	EMAIL,
	PHONE,
	PHONEAREA,
	PHONENUM,
};

struct xfdf_form_translation {
	const char *line;
	enum form_flags flags;
	const char *pdf_line_dollar;
	const char *pdf_line_cents;
};

struct xfdf_form_translation null_translation[] = {
	{NULL, DOLLAR_ONLY, NULL, NULL}
};

void output_xfdf_form_data(FILE *out, struct xfdf_form_translation *form, Lmap &lines, struct xfdf_form_translation* form2 = null_translation, Lmap& lines2 = null_lmap)
{

	fprintf(out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(out, "<xfdf xmlns=\"http://ns.adobe.com/xfdf/\">\n");
	fprintf(out, "\t<fields>\n");

	for (int i=0; form[i].line != NULL; i++) {
		double dollar = 0.0, cents = 0.0;

		if ((form[i].flags == NAME1FIRSTMID)  && (strlen(name1FirstMid) > 0)){
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", name1FirstMid);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == NAME1LAST)  && (strlen(name1Last) > 0)){
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", name1Last);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == NAME1) && (strlen(name1FirstMid) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s %s</value>\n", name1FirstMid, name1Last);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == NAME2FIRSTMID)  && (strlen(name2FirstMid) > 0)){
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", name2FirstMid);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == NAME2LAST)  && (strlen(name2Last) > 0)){
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", name2Last);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == NAME2) && (strlen(name2FirstMid) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s %s</value>\n", name2FirstMid, name2Last);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == NAME1_AND_NAME2) && (strlen(name1FirstMid) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s %s", name1FirstMid, name1Last);
		  if (strlen(name2FirstMid) > 0) fprintf(out,"  &  %s %s", name2FirstMid, name2Last);
		  fprintf(out,"</value>\n");
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == ADDRESS1) && (strlen(address1) > 0)){
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", address1);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == ADDRESSAPTNO) && (strlen(addressAptNo) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", addressAptNo);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == ADDRESS2) && (strlen(address2) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", address2);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == SOCSEC1) && (strlen(socsec1) > 0)){
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", socsec1);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == SOCSEC2) && (strlen(socsec2) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", socsec2);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == PREZ1) && (strcasecmp(prez1, "yes") == 0)) {
			fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
			fprintf(out,"\t\t\t<value>1</value>\n");
			fprintf(out,"\t\t</field>\n");
		}

		if ((form[i].flags == PREZ2) && (strcasecmp(prez2, "yes") == 0)) {
			fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
			fprintf(out,"\t\t\t<value>1</value>\n");
			fprintf(out,"\t\t</field>\n");
		}

		if ((form[i].flags == ROUTINGNUM) && (strlen(routingnum) > 0)){
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", routingnum);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == ACCOUNTNUM) && (strlen(accountnum) > 0)){
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", accountnum);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == ACCOUNTTYPE_CHECKING) && (strlen(accounttype) > 0) && (strcasecmp(accounttype, "checking") == 0)){
		  double val	= lines[form[i].line];
		  if (val > 0) {
			  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
			  fprintf(out,"\t\t\t<value>%s</value>\n", form[i].pdf_line_cents);
			  fprintf(out,"\t\t</field>\n");
			}
		}
		if ((form[i].flags == ACCOUNTTYPE_SAVINGS) && (strlen(accounttype) > 0) && (strcasecmp(accounttype, "savings") == 0)){
		  double val	= lines[form[i].line];
		  if (val > 0) {
			  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
			  fprintf(out,"\t\t\t<value>%s</value>\n", form[i].pdf_line_cents);
			  fprintf(out,"\t\t</field>\n");
			}
		}
		if ((form[i].flags == DOB1) && (strlen(dob1) > 0)){
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", dob1);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == DOB2) && (strlen(dob2) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", dob2);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == ADDRESSTOWN) && (strlen(addressTown) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", addressTown);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == ADDRESSSTATE) && (strlen(addressState) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", addressState);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == ADDRESSZIP) && (strlen(addressZip) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", addressZip);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == OCCUPATION) && (strlen(occupation) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", occupation);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == SPOUSE_OCCUPATION) && (strlen(spouse_occupation) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", spouse_occupation);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == EMAIL) && (strlen(email) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", email);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == PHONE) && (strlen(phone) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", phone);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == PHONEAREA) && (strlen(phonearea) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", phonearea);
		  fprintf(out,"\t\t</field>\n");
		}
		if ((form[i].flags == PHONENUM) && (strlen(phonenum) > 0)) {
		  fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
		  fprintf(out,"\t\t\t<value>%s</value>\n", phonenum);
		  fprintf(out,"\t\t</field>\n");
		}

		if (strlen(form[i].line) == 0) continue;
		
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

			if (form[i].flags == ABS_DOLLAR_AND_CENTS) {
			  dollar = absolutev((int)lines[form[i].line]);
				fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%.f</value>\n", dollar);
				fprintf(out,"\t\t</field>\n");
				cents = (absolutev(lines[form[i].line]*100.0) - (int)dollar*100.0);
				fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_cents);
				fprintf(out,"\t\t\t<value>%02.f</value>\n", cents);
				fprintf(out,"\t\t</field>\n");
			}

			if (form[i].flags == DOLLAR_AND_CENTS_ONE) {
				fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%.2f</value>\n", lines[form[i].line]);
				fprintf(out,"\t\t</field>\n");
			}

			if (form[i].flags == ABS_DOLLAR_AND_CENTS_ONE) {
				fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%.2f</value>\n", absolutev(lines[form[i].line]));
				fprintf(out,"\t\t</field>\n");
			}

			if (form[i].flags == FOUR_DIGITS) {
				fprintf(out,"\t\t<field name=\"%s\">\n", form[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%4.f</value>\n", lines[form[i].line]*10000.0);
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

	}

	for (int i=0; form2[i].line != NULL; i++) {
		double dollar = 0.0, cents = 0.0;

		if(lines2.count(form2[i].line) == 1) {

		  if (strlen(form2[i].line) == 0) continue;

		if (lines2.count(form2[i].line) == 1) {

			if (form2[i].flags == DOLLAR_ONLY || form2[i].flags == DOLLAR_AND_CENTS) {
				dollar = (int)lines2[form2[i].line];
				fprintf(out,"\t\t<field name=\"%s\">\n", form2[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%.f</value>\n", dollar);
				fprintf(out,"\t\t</field>\n");
			}

			if (form2[i].flags == DOLLAR_AND_CENTS) {
				cents = (lines2[form2[i].line]*100.0 - (int)dollar*100.0);
				fprintf(out,"\t\t<field name=\"%s\">\n", form2[i].pdf_line_cents);
				fprintf(out,"\t\t\t<value>%02.f</value>\n", cents);
				fprintf(out,"\t\t</field>\n");
			}

			if (form2[i].flags == ABS_DOLLAR_AND_CENTS) {
			  dollar = absolutev((int)lines2[form2[i].line]);
				fprintf(out,"\t\t<field name=\"%s\">\n", form2[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%.f</value>\n", dollar);
				fprintf(out,"\t\t</field>\n");
				cents = (absolutev(lines2[form2[i].line]*100.0) - (int)dollar*100.0);
				fprintf(out,"\t\t<field name=\"%s\">\n", form2[i].pdf_line_cents);
				fprintf(out,"\t\t\t<value>%02.f</value>\n", cents);
				fprintf(out,"\t\t</field>\n");
			}

			if (form2[i].flags == DOLLAR_AND_CENTS_ONE) {
				fprintf(out,"\t\t<field name=\"%s\">\n", form2[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%.2f</value>\n", lines2[form2[i].line]);
				fprintf(out,"\t\t</field>\n");
			}

			if (form2[i].flags == ABS_DOLLAR_AND_CENTS_ONE) {
				fprintf(out,"\t\t<field name=\"%s\">\n", form2[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%.2f</value>\n", absolutev(lines2[form2[i].line]));
				fprintf(out,"\t\t</field>\n");
			}

			if (form2[i].flags == FOUR_DIGITS) {
				fprintf(out,"\t\t<field name=\"%s\">\n", form2[i].pdf_line_dollar);
				fprintf(out,"\t\t\t<value>%4.f</value>\n", lines2[form2[i].line]*10000.0);
				fprintf(out,"\t\t</field>\n");
			}
		}

		if (form2[i].flags == USE_KEY_IN_FORM && lines2.count(form2[i].line) == 1) {
			fprintf(out,"\t\t<field name=\"%s\">\n", form2[i].pdf_line_dollar);
			fprintf(out,"\t\t\t<value>%s</value>\n", form2[i].line);
			fprintf(out,"\t\t</field>\n");
		}

		if (form2[i].flags == IF_SET && lines2.count(form2[i].line) == 1) {
			fprintf(out,"\t\t<field name=\"%s\">\n", form2[i].pdf_line_dollar);
			fprintf(out,"\t\t\t<value>%s</value>\n", form2[i].pdf_line_cents);
			fprintf(out,"\t\t</field>\n");
		}
		}
	}
	fprintf(out, "\t</fields>\n");
	fprintf(out, "</xfdf>");
}


void get_personal_details( FILE *infile)
{
  FILE *persfile;
  char word[1000];
  char personal_details_filename[5000];

  get_parameter( infile, 's', word, "PersonalFilename" ); get_word(infile, personal_details_filename );	/* Personal Details File-name. */

  if (strlen(personal_details_filename) <= 0) return;
  if (strcasecmp(personal_details_filename, "none") == 0) return;

  Convert_slashes( personal_details_filename );
  persfile = fopen(personal_details_filename, "r");
  if (persfile==0)
	{
	  printf("Error: Could not open personal_details_filename '%s'\n", personal_details_filename);
	  fprintf(outfile,"Error: Could not open personal_details_filename '%s'\n", personal_details_filename);
	  return; 
	}

  get_parameter( persfile, 's', word, "Name1FirstMid" ); get_text(persfile, name1FirstMid);
  get_parameter( persfile, 's', word, "Name1Last" ); get_text(persfile, name1Last);
  if (((strlen(name1FirstMid) > 0) && (strlen(name1Last) <= 0)) || ((strlen(name1FirstMid) <= 0) && (strlen(name1Last) > 0))) {
	printf("Error: either both or none of Name1FirstMid(%s) and Name1Last(%s) must be empty. Exiting.\n", name1FirstMid, name1Last); 
	fprintf(outfile, "Error: either both or none of Name1FirstMid(%s) and Name1Last(%s) must be empty. Exiting.\n", name1FirstMid, name1Last); 
	exit(1);
  }

  get_parameter( persfile, 's', word, "Name2FirstMid" ); get_text(persfile, name2FirstMid);
  get_parameter( persfile, 's', word, "Name2Last" ); get_text(persfile, name2Last);
  if (((strlen(name2FirstMid) > 0) && (strlen(name2Last) <= 0)) || ((strlen(name2FirstMid) <= 0) && (strlen(name2Last) > 0))) {
	printf("Error: either both or none of Name2FirstMid(%s) and Name2Last(%s) must be empty. Exiting.\n", name2FirstMid, name2Last); 
	fprintf(outfile, "Error: either both or none of Name2FirstMid(%s) and Name2Last(%s) must be empty. Exiting.\n", name2FirstMid, name2Last); 
	exit(1);
  }
  if ((strlen(name2Last) > 0) && (strlen(name1Last) <= 0)) {
	printf("Error: if Name2Last(%s) is given, then so must Name1Last. Exiting.\n", name2Last); 
	fprintf(outfile, "Error: if Name2Last(%s) is given, then so must Name1Last. Exiting.\n", name2Last); 
	exit(1);
  }

  get_parameter( persfile, 's', word, "Address1" ); get_text(persfile, address1);
  get_parameter( persfile, 's', word, "AddressAptNo" ); get_text(persfile, addressAptNo);
  get_parameter( persfile, 's', word, "AddressTown" ); get_text(persfile, addressTown);
  get_parameter( persfile, 's', word, "AddressState" ); get_text(persfile, addressState);
  get_parameter( persfile, 's', word, "AddressZIP" ); get_text(persfile, addressZip);
  if (strlen(addressTown) > 0) strcpy(&address2[0], &addressTown[0]);
  if (strlen(addressState) > 0) strcat(&address2[0], " ");
  if (strlen(addressState) > 0) strcat(&address2[0], &addressState[0]);
  if (strlen(addressZip) > 0) strcat(&address2[0], " ");
  if (strlen(addressZip) > 0) strcat(&address2[0], &addressZip[0]);
  if (((strlen(address1) > 0) && (strlen(address2) <= 0)) || ((strlen(address1) <= 0) && (strlen(address2) > 0))) {
	printf("Error: either both or none of Address1(%s) and Address2(%s) must be empty. Exiting.\n", address1, address2); 
	fprintf(outfile, "Error: either both or none of Address1(%s) and Address2(%s) must be empty. Exiting.\n", address1, address2); 
	exit(1);
  }
  get_parameter( persfile, 's', word, "SSN1" ); get_text(persfile, socsec1);
  get_parameter( persfile, 's', word, "SSN2" ); get_text(persfile, socsec2);
  get_parameter( persfile, 's', word, "PREZ1" ); get_text(persfile, prez1);
  get_parameter( persfile, 's', word, "PREZ2" ); get_text(persfile, prez2);
  get_parameter( persfile, 's', word, "ROUTINGNUM" ); get_text(persfile, routingnum);
  get_parameter( persfile, 's', word, "ACCOUNTNUM" ); get_text(persfile, accountnum);
  get_parameter( persfile, 's', word, "ACCOUNTTYPE" ); get_text(persfile, accounttype);
  get_parameter( persfile, 's', word, "DOB1" ); get_text(persfile, dob1);
  get_parameter( persfile, 's', word, "DOB2" ); get_text(persfile, dob2);
  get_parameter( persfile, 's', word, "OCCUPATION" ); get_text(persfile, occupation);
  get_parameter( persfile, 's', word, "SPOUSE_OCCUPATION" ); get_text(persfile, spouse_occupation);
  get_parameter( persfile, 's', word, "EMAIL" ); get_text(persfile, email);
  get_parameter( persfile, 's', word, "PHONEAREA" ); get_text(persfile, phonearea);
  get_parameter( persfile, 's', word, "PHONENUM" ); get_text(persfile, phonenum);
  if (strlen(phonearea) > 0) strcpy(&phone[0], &phonearea[0]);
  if (strlen(phonenum) > 0) strcat(&phone[0], "-");
  if (strlen(phonenum) > 0) strcat(&phone[0], &phonenum[0]);
  fclose(persfile);
}

