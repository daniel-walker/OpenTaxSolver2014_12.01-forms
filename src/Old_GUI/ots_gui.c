/********************************************************/
/* OTS_GUI.c - OpenTaxSolver Graphical User Interface.	*/
/* Based on OpenToolKit (OTK).				*/
/*							*/
/* The OTS_GUI is designed to read-in standard OTS data	*/
/* files, and present the tax information on your 	*/
/* screen.  It allows entering the values, saving 	*/
/* return data, editing previously saved return sheets, */
/* and running the OTS tax-solver to compute your 	*/
/* taxes.						*/
/*							*/
/* The OTS tax-solver is a text program which can be	*/
/* used by itself. This GUI front-end simplifies using  */
/* OTS.  It walks you through the steps and invokes the	*/
/* regular OTS solver when you are ready.		*/
/*							*/
/* OTS data files, for example "tax_xx.txt", contain 	*/
/* the line numbers (or names) of the entries for a 	*/
/* given tax form, as well as any previously entered 	*/
/* values for each line.  Additional comments may 	*/
/* follow on any line.  				*/
/* A few example lines follow:				*/
/*							*/
/*	L15    	      ;  { Rental income }		*/
/*	L17    234.00 ;  { Savings interest }		*/
/*	L18     	 { Dividends }			*/
/*		23.00    {  Bank1 }			*/
/*		14.50 ;  {  Work loan }			*/
/*							*/
/* OTS_GUI reads these lines and places a label for 	*/
/* each line number/name, a text-box for filling in	*/
/* the value(s), and addtional labels for the comments.	*/
/* At the bottom are placed buttons to save and 	*/
/* calculate-taxes.					*/
/* 							*/
/* To compile this graphical program, you will need to  */
/* grab the Open graphics Tool-Kit (Otk). 		*/
/* Download Otk from:					*/
/*     https://sourceforge.net/projects/otk/		*/
/* It unpacks to a subdirectory called otk_lib.		*/
/*							*/
/* Compile:						*/
/*  cc -I/usr/X11R6/include -L/usr/X11R6/lib ots_gui.c  \
 	-lGLU -lGL -lXmu -lXext -lX11 -lm -o ots_gui	*/
/*							*/
/*							*/
/********************************************************/

float version=0.90;

/************************************************************/
/* Design Notes - 					    */
/*  Unlike the individual tax programs, which can know	    */
/*  what to expect on each line, this GUI does not know     */
/*  about the format of any particular tax form file.	    */
/*  Therefore, it applies some simple rules to parse	    */
/*  the fields:						    */
/*   1. The next non-comment / non-white-space character    */
/*	after a ";" is interpretted as a line-label.	    */
/*	(eg. L51)					    */
/*	However, some lines expecting single values were    */
/*	not terminated with ";", so other rules are needed. */
/*   2. Any line with a non-comment / non-white-space char  */
/*	in column 1 (first char on line), is assumed to be  */
/*	a line-label.  So all field values should be 	    */
/*	indented to use the GUI with them.		    */
/************************************************************/

#include "otk_lib/otk_lib.c"	/* Include the graphics library. */
int verbose=0;
FILE *infile;
int ots_column=0, ots_line=0;	/* Input file position. */
#define MaxFname 2048
char directory_bin[MaxFname]=".", wildcards_bin[MaxFname]="", filename_exe[MaxFname]="";
char directory_dat[MaxFname]=".", wildcards_dat[MaxFname]="*.txt", filename_dat[MaxFname]="";
char directory_incl[MaxFname]="examples_and_templates", wildcards_incl[MaxFname]="*_out.txt";
char title_line[900], *working_file=0, *invocation_path, include_file_name[MaxFname]="";
char wildcards_out[MaxFname]="*_out.txt";
OtkWidget title_panel, title_label, main_panel, warningpopup=0, popup, commentbox, bckgrnd;
OtkWidget selected_tog[40], selected_button[40], last_tog;
OtkTabbedPanel *TabbedPanel;
int selected_form=0;
int selected_other=0;
void open_taxfile();
void save_taxfile();
void printout( void *data );
void taxsolve();
char *taxsolvecmd=0, taxsolvestrng[MaxFname]="";
struct Otk_image *right_arrow_image, *left_arrow_image;

#define VKIND_FLOAT   0
#define VKIND_INT     1
#define VKIND_TEXT    2
#define VKIND_COMMENT 3
#define VKIND_COLON   4

#define VALUE_LABEL 0
#define COMMENT     1
#define SEMICOLON   2
#define NOTHING	    10
#define ENABLED 1
#define DISABLED 0

struct value_list
 {
  int	    kind;	/* 0=float, 1=integer, 2=text, 3=comment. */
  float     value;
  char      *comment, *text;
  int       column, linenum;
  struct taxline_record *parent;
  OtkWidget box;
  struct value_list *nxt;
 };

struct taxline_record
 {
  char *linename;
  int linenum, format_offset;
  struct value_list *values_hd, *values_tl;	/* Head and tail list pointers for a tax-line-entry. */
  struct taxline_record *nxt;
 } *taxlines_hd=0, *taxlines_tl=0;		/* Head and tail list pointers for tax-form. */


 struct taxline_record * 
new_taxline( char *linename, int linenum )
{
 struct taxline_record *tmppt;

 tmppt = (struct taxline_record *)malloc(sizeof(struct taxline_record));
 tmppt->linename = strdup(linename);
 tmppt->linenum = linenum;
 tmppt->format_offset = 0;
 tmppt->values_hd = 0;
 tmppt->values_tl = 0;
 tmppt->nxt = 0;
 if (taxlines_hd==0) taxlines_hd = tmppt;
 else taxlines_tl->nxt = tmppt;
 taxlines_tl = tmppt;
 return tmppt;
}


 struct value_list * 
new_list_item_value( int kind, struct taxline_record *txline, void *x, int column, int linenum )
{ 
 struct value_list *tmppt;

 tmppt = (struct value_list *)malloc(sizeof(struct value_list));
 tmppt->kind = kind;
 tmppt->text = 0;
 tmppt->comment = 0;
 tmppt->column = column;
 tmppt->linenum = linenum;
 tmppt->box = 0;
 switch (kind)
  {
   case VKIND_FLOAT:   tmppt->value = *(float *)x; break;
   case VKIND_INT:     tmppt->value = *(int *)x; break;
   case VKIND_TEXT:    tmppt->text = strdup( (char *)x ); break;
   case VKIND_COMMENT: tmppt->comment = strdup( (char *)x ); break;
  }
 tmppt->parent = txline;
 tmppt->nxt = 0;
 if (txline==0) {printf("ERROR1:  called add_value %d before any line.\n",kind); return tmppt;}
 if (txline->values_hd==0) txline->values_hd = tmppt;  else  txline->values_tl->nxt = tmppt;
 txline->values_tl = tmppt;
 return tmppt;
}


/*--------------------------------------------------------------*/
/* Get_Next_Entry - Reads next item from input file.		*/
/* Returns 0=VALUE_LABEL if reads data value or line-label. 	*/
/* Returns 1=COMMENT     if reads comment.			*/
/* Returns 2=SEMICOLON   if reads ';' entry-end character.	*/
/*								*/
/* Passes back the column and line number where the current     */
/* entry begins on the line in the input file.  		*/
/*--------------------------------------------------------------*/
int get_next_entry( char *word, int maxn, int *column, int *linenum, FILE *infile )
{
 int k=0;

 /* Get up to the next non-white-space character. */
 do 
  { 
   word[k] = getc(infile);
   if (word[k] == '\n') { ots_column = 0;  ots_line++; } else ots_column++;
  }
 while ((!feof(infile)) && ((word[k]==' ') || (word[k]=='\t') || (word[k]=='\n') || (word[k]=='\r')));
 *column = ots_column;
 *linenum = ots_line;

 if (feof(infile)) {word[0] = '\0'; return NOTHING;}
 if (word[k]=='{')
  { /*get_comment*/
    do 
     {
      word[k++] = getc(infile);
      if (word[k-1] == '\n') { ots_column = 0;  ots_line++; } else ots_column++;
     }
    while ((!feof(infile)) && (word[k-1]!='}') && (k<maxn));
    word[k-1] = '\0';
    if (k>=maxn) {printf("Error: Character buffer overflow detected.\n"); exit(0);}
    return COMMENT;
  } /*get_comment*/
 else
 if (word[k]=='"')
  { /*get_quoted_value*/
    k++;
    do 
     {
      word[k++] = getc(infile);
      if (word[k-1] == '\n') { ots_column = 0;  ots_line++; } else ots_column++;
     }
    while ((!feof(infile)) && (word[k-1]!='"') && (k<maxn));
    if (k>=maxn) {printf("Error: Character buffer overflow detected.\n"); exit(0);}
    word[k] = '\0';
    return VALUE_LABEL;
  } /*get_quoted_value*/
 else
  { /*get_value_or_linelabel*/
    k++;
    while ((!feof(infile)) && (word[k-1]!=' ') && (word[k-1]!='\t') && 
	   (word[k-1]!='\n') && (word[k-1]!='\r') && (word[k-1]!=';') && (k<maxn))
      { 
	word[k++] = getc(infile);
	if (word[k-1] == '\n') { ots_column = 0;  ots_line++; } else ots_column++;
      }
    if (k>=maxn) {printf("Error: Character buffer overflow detected.\n"); exit(0);}
    if (word[k-1]==';')
     { 
      if (k==1) { word[1] = '\0';  return SEMICOLON; }
      else { ungetc(word[k-1], infile); word[k-1] = '\0';  return VALUE_LABEL; }
     }
    else { word[k-1] = '\0';  return VALUE_LABEL; }
  } /*get_value_or_linelabel*/
}


void DisplayTaxInfo();
int save_needed=0;



int Update_box_info()	/* Capture entries from form-boxes. */
{
 struct taxline_record *txline;
 struct value_list *tmppt;
 char text[1024];

 txline = taxlines_hd;
 while (txline!=0)
  {
   tmppt = txline->values_hd;
   while (tmppt!=0)
    {
     if (tmppt->box != 0)
      {
       Otk_Get_Text( tmppt->box, text, 1024 );	
       tmppt->kind = VKIND_TEXT;
       if (strcmp( tmppt->text, text ) != 0) save_needed++;
       tmppt->text = strdup( text );
      }
     tmppt = tmppt->nxt;
    }
   txline = txline->nxt;
  }
 return save_needed;
}


void refresh()
{
 int pannum;

 /* Determine the current panel. */
 pannum = 0;
 while ((pannum < TabbedPanel->num) && (TabbedPanel->selection != TabbedPanel->selects[pannum].selection )) 
  pannum++;

 /* Remove and recreate the page-panels with the new line item. */
 Otk_RemoveObject( main_panel );
 main_panel =  OtkMakePanel( OtkOuterWindow, Otk_Raised, Otk_LightGray, 1, 7.5, 98, 87 );	/* Main Panel. */
 Otk_SetBorderThickness( main_panel, 0.25 );
 DisplayTaxInfo();

 /* Set to display the original page. */
 Otk_tabbed_panel_select( &(TabbedPanel->selects[pannum]) );
}


/*--------------------------------------------------------------*/
/* Add_New_Boxes - Callback for "+" button on form-boxes.	*/
/*  Adds new form-box(s) to the line item.			*/
/*--------------------------------------------------------------*/
void add_new_boxes( void *data, int num )
{
 struct taxline_record *txline;
 struct value_list *item, *lineitem, *newitem1, *newitem2, *oldtail;

 Update_box_info();

 item = (struct value_list *)data;
 oldtail = item->parent->values_tl;

 newitem1 = new_list_item_value( VKIND_TEXT, item->parent, "", 0, item->linenum + 1 );
 if (num>1)
  {
   new_list_item_value( VKIND_TEXT, item->parent, "", 0, item->linenum + 1 );
   new_list_item_value( VKIND_TEXT, item->parent, "", 0, item->linenum + 2 );
   newitem2 = new_list_item_value( VKIND_TEXT, item->parent, "", 0, item->linenum + 2 );
  }
 else newitem2 = newitem1;

 lineitem = item;	 /* Skip the items on the original line. */
 while ((lineitem->nxt!=newitem1) && (lineitem->nxt->linenum == item->linenum)) lineitem = lineitem->nxt;

 /* Insert in list and remove from tail, if not on end of list. */
 if (lineitem->nxt != newitem1)
  {
   newitem2->nxt = lineitem->nxt;
   lineitem->nxt = newitem1;
   oldtail->nxt = 0;  
   item->parent->values_tl = oldtail;   
  }

 /* Increment the effective file-line-number of all subsequent line entries. */
 lineitem = newitem2->nxt;
 while (lineitem!=0)		/* Now do remaining lines of this item. */
  {
   lineitem->linenum = lineitem->linenum + num;
   lineitem = lineitem->nxt;
  }
 txline = item->parent->nxt;	/* Next do remaining line items. */
 while (txline!=0)
  {
   txline->linenum = txline->linenum + num;
   lineitem = txline->values_hd;
   while (lineitem!=0)
    {
     lineitem->linenum = lineitem->linenum + num;
     lineitem = lineitem->nxt;
    }
   txline = txline->nxt;
  }

 refresh();
}


/*-------------------------------------------------------------------------*/
/* Add_new_capgain_boxes - Callback for "+" button on cap-gain form-boxes. */
/*  Adds a new form-boxes to the line item.				   */
/*-------------------------------------------------------------------------*/
void add_new_capgain_boxes( void *data )
{
 add_new_boxes( data, 2 ); 
}

void add_new_box_item( void *data )
{
 add_new_boxes( data, 1 );
}


void cancelpopup()
{ Otk_RemoveObject(popup); }

void acceptcomment( void *data )
{
 char comment[500];
 struct value_list *tmppt;

 tmppt = (struct value_list *)data;
 Otk_Get_Text( commentbox, comment, 500 );
 tmppt->comment = strdup(comment);
 cancelpopup();
 refresh();
}

void acceptcomment2( char *s, void *x ) { acceptcomment( x ); }

void edit_line_comment( void *data )
{
 char comment[1000];
 struct value_list *tmppt;

 tmppt = (struct value_list *)data;
 strcpy(comment,tmppt->comment);
 popup = OtkMakeWindow( Otk_Raised, Otk_Blue, OtkSetColor(0.8,0.8,0.8), 10.0, 10.0, 80.0, 25.0 );
 OtkMakeTextLabel( popup, "Edit Line Comment", Otk_Black, /*scale=*/ 1.5, /*weight=*/ 1, /*x=*/ 5, /*y=*/ 10 );
 commentbox = OtkMakeTextFormBox( popup, comment, 60,  8, 35, 90, 22, acceptcomment2, tmppt );
 OtkMakeButton( popup, 15, 75.0, 10, 14, " Ok ", acceptcomment, tmppt );	
 OtkMakeButton( popup, 80, 75.0, 10, 14, "Cancel", cancelpopup, 0 );	
}


/* Check if entry looks like a date. */
/* If so, return 1, else return 0.  */
int datecheck( char *word )
{
 int j, k=0;

 j = strlen(word) - 1;
 while (j>0)
  {
   if ((word[j]=='-') || (word[j]=='/')) k++;
   else if (word[j]>'9') return 0;
   j--;
  }
 if (k==2) return 1; else return 0;
}


#if (PLATFORM_KIND!=Posix_Platform) 
 char slashchr='\\';
#else
 char slashchr='/';
#endif


/***********************/
/* Read Tax Data File. */
/***********************/
void Read_Tax_File( char *fname )
{
 int j, k, kind, state=0, column=0, linenum=0, linecnt=0, lastline=0, newentry=0, entrycnt;
 char word[10000], *tmpstr, tmpstr2[100], tmpstr3[100];
 struct taxline_record *txline=0;
 struct value_list *tmppt, *newitem, *oldtail;

 Otk_Set_Default_Button_BorderThickness( 2.0 );
 OtkMakeButton( bckgrnd, 6, 95.3, 15, 4, "Save ", save_taxfile, 0 );	/* Save Button. */
 OtkMakeButton( bckgrnd, 27, 95.3, 21, 4, "Compute Tax ", taxsolve, 0 );	/* TaxSolve Button. */
 OtkMakeButton( bckgrnd, 55, 95.3, 15, 4, "Print", printout, 0 );	/* Print Button. */
 Otk_Set_Default_Button_BorderThickness( 1.0 );

 /* Read the Tax Data Form File. */
 working_file = strdup(fname);
 taxlines_hd = 0;
 /* Accept the form's Title line.  (Must be first line!) */
 fgets(word, 200, infile);
 strcpy(title_line, word);
 j = strlen(word);
 if (j>0) word[j-1] = '\0';
 // printf("Title: '%s'\n", word);
 if (strstr(word,"Title:")==word) tmpstr = &(word[6]); else tmpstr = &(word[0]);
 k = strlen(tmpstr);	/* Pad to center if title is too short. */
 if (k<20)
  { for (j=0; j<(20-k)/2; j++) tmpstr2[k]=' '; tmpstr2[(20-k)/2] = '\0'; 
    strcpy(tmpstr3,tmpstr2); strcat(tmpstr3,tmpstr); strcpy(tmpstr,tmpstr3); strcat(tmpstr,tmpstr2);
  }
 Otk_Modify_Text( title_label, tmpstr );
 Otk_FitTextInPanel( title_label );

 kind = get_next_entry( word, 10000, &column, &linenum, infile );
 if (linenum > lastline) { lastline = linenum;  if (newentry) linecnt++;  newentry = 0; }
 while (!feof(infile))
  {
   if (column == 1) state = 0;
   if (verbose) printf("%d: state=%d: col=%d: lnum=%d:  '%s'\n", kind, state, column, linenum, word);
   switch (kind)
    {
     case VALUE_LABEL: 
	 if (state==0) 
	  {
	   if (verbose) printf(" LineLabel:	%s\n", word);
	   state = 1;
	   entrycnt = 0;
	   txline = new_taxline( word, linecnt );
	  }
	 else
	  {
	   if (verbose) printf(" Value:	%s\n", word);
	   if (strcasecmp(txline->linename, "Status") == 0)
	    {
	     new_list_item_value( VKIND_TEXT, txline, word, column, linecnt );
	     state = 0;
	    }
	   else
	    {
	     new_list_item_value( VKIND_TEXT, txline, word, column, linecnt );
	     entrycnt++;
	    }
	  }
	newentry++;
	break;
     case COMMENT: if (verbose) printf(" Comment:	%s\n", word);
	if (txline==0) txline = new_taxline("", linecnt);
	new_list_item_value( VKIND_COMMENT, txline, word, column, linecnt );
	newentry++;
	break;
     case SEMICOLON: if (verbose) printf(" End:	%s\n", word);
	  /* When line is labeled "Cap-Gains", and there are no entries,	  */
	  /* then produce extra boxes for date bought or sold.	  */
	  /* So far, this is only known to be needed on US-Fed form. */
	if ((txline != 0) && (strncasecmp(txline->linename, "Cap-Gains",8) == 0) && (entrycnt < 2))
	 {
	  new_list_item_value( VKIND_TEXT, txline, "", column, linecnt );
	  new_list_item_value( VKIND_TEXT, txline, "", column, linecnt++ );
	  new_list_item_value( VKIND_TEXT, txline, "", column, linecnt );
	  new_list_item_value( VKIND_TEXT, txline, "", column, linecnt++ );
	 }
	state = 0;
	new_list_item_value( VKIND_COLON, txline, word, column, linecnt );
	break;
    }
   column = column + strlen(word);
   kind = get_next_entry( word, 10000, &column, &linenum, infile );
   if (linenum > lastline) { lastline = linenum;  if (newentry) linecnt++;  newentry = 0; }
  }
 fclose(infile);

 /* Check for missing entries. */
 txline = taxlines_hd;
 while (txline!=0)
  {
   tmppt = txline->values_hd;  state = 0;
   while (tmppt!=0)
    {
     if ((tmppt->kind==VKIND_FLOAT) || (tmppt->kind==VKIND_TEXT) || (tmppt->kind==VKIND_INT)) state = 1;
     tmppt = tmppt->nxt;
    }
   if ((state==0) && (strlen(txline->linename)>0))	/* Place empty formbox on any line having no entries. */
    {
      oldtail = txline->values_tl;
      newitem = new_list_item_value( VKIND_TEXT, txline, "", 0, txline->linenum );
      if (newitem!=txline->values_hd)
       {
	newitem->nxt = txline->values_hd;
	txline->values_hd = newitem;
	txline->values_tl = oldtail;
	oldtail->nxt = 0;
       }
    }
   txline = txline->nxt;
  }

 DisplayTaxInfo();
}




void advance_panel( void *x )
{
 int pannum=0, direction;

 /* Determine the current panel. */
 while ((pannum < TabbedPanel->num) && (TabbedPanel->selection != TabbedPanel->selects[pannum].selection )) 
  pannum++;

 direction = (long)x;
 pannum = pannum + direction;
 if (pannum >= TabbedPanel->num) return;
 if (pannum < 0) return;

 /* Set to display the original page. */
 Otk_tabbed_panel_select( &(TabbedPanel->selects[pannum]) );
}



void check_comments()	/* Make sure every line has a comment field. */
{
 struct taxline_record *txline;
 struct value_list *tmppt, *npt, *tail;
 int ncomments;

 txline = taxlines_hd;
 while (txline!=0)
  {
   ncomments = 0;
   tmppt = txline->values_hd;
   while (tmppt!=0)
    {
     if (tmppt->kind==VKIND_COMMENT) ncomments++;
     if ((tmppt->nxt==0) || (tmppt->linenum != tmppt->nxt->linenum))
      {
       if (ncomments==0)
        {
         new_list_item_value( VKIND_COMMENT, txline, "", 50, tmppt->linenum);
	 if (tmppt->nxt != txline->values_tl)
	  {
	   npt = txline->values_tl;
	   tail = tmppt->nxt;
	   while (tail->nxt != npt) tail = tail->nxt;
	   tail->nxt = 0;
 	   txline->values_tl = tail;
	   npt->nxt = tmppt->nxt;
	   tmppt->nxt = npt;
	 }
        }
       ncomments = 0;
      }
     tmppt = tmppt->nxt;
    }
   txline = txline->nxt;
  }
}



void status_choice_S( void *x )
{ struct value_list *tmppt=(struct value_list *)x;
  Otk_Modify_Text( tmppt->box, "Single" );
}

void status_choice_MJ( void *x )
{ struct value_list *tmppt=(struct value_list *)x;
  Otk_Modify_Text( tmppt->box, "Married/Joint" );
}

void status_choice_MS( void *x )
{ struct value_list *tmppt=(struct value_list *)x;
  Otk_Modify_Text( tmppt->box, "Married/Sep" );
}

void status_choice_HH( void *x )
{ struct value_list *tmppt=(struct value_list *)x;
  Otk_Modify_Text( tmppt->box, "Head_of_Household" );
}

void status_choice_W( void *x )
{ struct value_list *tmppt=(struct value_list *)x;
  Otk_Modify_Text( tmppt->box, "Widow(er)" );
}



OtkWidget active_entry;


void set_included_file( char *filename )
{
 if (strstr( filename, " "))
  { /* If filename contains white-space, then add quotes around it. */
   char tmpfname[MaxFname]="\"";
   strcat( tmpfname, filename );
   strcat( tmpfname, "\"" );
   strcpy( filename, tmpfname );
  }
 strcpy( include_file_name, filename );
 Otk_Modify_Text( active_entry, filename );
}


void open_include_file( void *data )
{ char slsh[2]="/";
 active_entry = (OtkWidget)data;
 Otk_Get_Text( active_entry, include_file_name, MaxFname );
 // printf("\ninclude_file_name = '%s'\n", include_file_name );
 if (include_file_name[0] == '?') include_file_name[0] = '\0';  /* Erase place-holder. */
 if (include_file_name[0] != '\0') 
  { int j, k;
   strcpy( directory_incl, include_file_name );	
   j = strlen( directory_incl ) - 1;
   while ((j >= 0) && (directory_incl[j] !='/') && (directory_incl[j] !='\\')) j--;
   if (j < 0)
    strcpy( directory_incl, directory_dat );		/* Leaf-name only. */
   else
    {
     slsh[0] = directory_incl[j];
     strcpy( include_file_name, &(directory_incl[j+1]) );
     directory_incl[j+1] = '\0';
     if (directory_incl[0] == '.')
      {
	char tmpstr[MaxFname];
	strcpy( tmpstr, directory_dat );
	strcat( tmpstr, slsh );
	strcat( tmpstr, directory_incl );
	strcpy( directory_incl, tmpstr );
      }
    }
  }
 // printf("\ninclude_dir = '%s'\t", directory_incl );	
 // printf("\tinclude_file_name = '%s'\n", include_file_name );	
 Otk_Browse_Files( "File to Include:", MaxFname, directory_incl, wildcards_incl, include_file_name, set_included_file );
}


#define lines_per_page 10

/*************************************************************************/
/* Display the Tax Info - This routine constructs, lays-out and poulates */
/*  the panels.  Called after initial read-in and on updates.		 */
/*************************************************************************/
void DisplayTaxInfo()
{
 struct taxline_record *txline;
 struct value_list *entry;
 char **panelnames;
 OtkWidget pagept, label, button, cbutton, lastbox;
 OtkTabbedPanel *Panels;
 char messg[2048];
 int j, nlines=1, npanels, page, linecnt=0, nchars=14, linenum, pagenum, iscapgains;
 int lastline, olinecnt, olinenum, offset=0, tchars, capgtoggle, capcnt=0;
 float pos_x, pos_y, twidth, theight, x, leftmargin=0.0;
 float box_x=11.0, box_width=18.0, box_height=5.0;

 check_comments();

 /* First count the number of lines to display. */
  txline = taxlines_hd;
  while (txline!=0)
   {
    if (linecnt < txline->linenum + offset) linecnt = txline->linenum + offset;
    linenum = linecnt % lines_per_page;
    olinecnt = linecnt;
    entry = txline->values_hd;
    while (entry!=0)
     {
      if (linecnt < entry->linenum + offset) linecnt = entry->linenum + offset;
      entry = entry->nxt;
     }

    /* Don't start a new multi-line entry at bottom of a page. */
    if ((linenum==lines_per_page-1) && (linecnt > olinecnt))
     {
      txline->format_offset = 1;
      offset++;
      linecnt++;
     }
    else txline->format_offset = 0;

    txline = txline->nxt;
   }
 nlines = linecnt;

 /* Now create enough tabbed-panels to hold all the lines. */
  npanels = nlines / lines_per_page + 1;
  if (npanels < 2) npanels = 2;
  panelnames = (char **)malloc( (npanels+1) * sizeof(char *));
  for (j=0; j<npanels; j++) {panelnames[j] = (char *)malloc(30); sprintf( panelnames[j], "Page %d ", j+1); }
  Panels = Otk_Tabbed_Panel_New( main_panel, npanels, panelnames, Otk_LightGray, 1, 1, 98, 98, 5 );
  TabbedPanel = Panels;

 /* Now place the form-data onto the pages. */
 page = 0;  linecnt = 0;  pagept = Panels->panels[0];  offset = 0;
 Otk_SetBorderThickness( pagept, 0.2 );
 OtkMakeTextLabel( pagept, panelnames[page], Otk_Black, 1.6, 1.0, 45.0, 2.0 );
 txline = taxlines_hd;
 while (txline!=0)
  {
   offset = offset + txline->format_offset;
   pagenum = (txline->linenum + offset) / lines_per_page;
   linenum = (txline->linenum + offset) % lines_per_page;
   if (page < pagenum)	/* Check to turn page. */
    { 
     page++;
     if (page>=npanels) {printf("Too many pages!\n"); exit(0);}
     pagept = Panels->panels[page];
     Otk_SetBorderThickness( pagept, 0.2 );
     OtkMakeTextLabel( pagept, panelnames[page], Otk_Black, 1.6, 1.0, 45.0, 2.0 );
    }

   /* Place the line label. */
   pos_x = 2.0;
   pos_y = (float)linenum * 9.0 + 8.0;
   olinenum = linenum;
   // printf("%d: Label %s\n", linenum, txline->linename );
   label = OtkMakeTextLabel( pagept, txline->linename, Otk_Black, 1.0, 1.0, pos_x, pos_y );
   if (strlen(txline->linename) > 8)  /* Squeeze size if too long. */
    {
     x = 7.8 / (float)strlen(txline->linename);
     Otk_Modify_Text_Aspect( label, x*sqrt(x) );
     Otk_Modify_Text_Scale( label, sqrt(sqrt(x)) );
    }

  if (strncmp(txline->linename,"Cap-Gains",9) == 0)
   iscapgains = 1;
  else
   iscapgains = 0;

   entry = txline->values_hd;
   lastline = -1;  button = 0;  capgtoggle = 0;
   while (entry!=0)
    { /*entry*/

     pagenum = (entry->linenum + offset) / lines_per_page;
     linenum = (entry->linenum + offset) % lines_per_page;
     if (page < pagenum)	/* Check to turn page. */
      { 
       page++;
       if (page>=npanels) {printf("Too many pages!\n"); exit(0);}
       pagept = Panels->panels[page];
       Otk_SetBorderThickness( pagept, 0.2 );
       OtkMakeTextLabel( pagept, panelnames[page], Otk_Black, 1.6, 1.0, 45.0, 2.0 );
      }
     if (lastline != linenum) leftmargin = box_x;
     switch (entry->kind)
      {
       case VKIND_FLOAT:  // printf("Formbox: '%s'\n", messg);
		sprintf(messg, "%12.2f", entry->value ); 
		pos_x = leftmargin;
		leftmargin = leftmargin + box_width + 1.5;
		pos_y = (float)linenum * 9.0 + 6.5;
		entry->box = 
		 OtkMakeTextFormBox( pagept, messg, nchars, pos_x, pos_y, box_width, box_height, 0, 0 );
		lastbox = entry->box;
		button = OtkMakeButton( pagept, pos_x+box_width, pos_y+4.0, 1.5, 1.5, "+", add_new_box_item, entry );   /* Add another box - button */
		Otk_Modify_Text_Scale( button->children, 0.8 );
		Otk_Register_Hover_Box( button, "Add another entry box\nfor this line." );
		lastline = linenum;
		break;

       case VKIND_INT:  // printf("Int: %d	\n", (int)(entry->value) ); 
		break;

       case VKIND_TEXT:  // printf(">>>Text: '%s'\n", entry->text );
		pos_x = leftmargin;
		pos_y = (float)linenum * 9.0 + 6.5;
		twidth = box_width;
		tchars = nchars;

		if (iscapgains && (lastline == linenum))
		 { /* Cap-gains date-entry. */
		  twidth = 0.75 * box_width;  
		  tchars = 8;
		  pos_x = pos_x + 1.0;
		  if (button != 0) Otk_RemoveObject(button);  button = 0;
		  if (capgtoggle == 0)
		   {
		    OtkMakeTextLabel( pagept, "Buy Cost", Otk_Black, 0.75, 1.0, pos_x - box_width, pos_y-1.9 );
		    OtkMakeTextLabel( pagept, "Date Bought", Otk_Black, 0.75, 1.0, pos_x+1.0, pos_y-1.9 );
		   }
		  if (capgtoggle == 1)
		   {
		    OtkMakeTextLabel( pagept, "Sold For", Otk_Black, 0.75, 1.0, pos_x - box_width, pos_y-1.9 );
		    OtkMakeTextLabel( pagept, "Date Sold", Otk_Black, 0.75, 1.0, pos_x+2.0, pos_y-1.9 );
		   }
		  capgtoggle = !capgtoggle;
	          capcnt++;
		 }

		leftmargin = leftmargin + twidth + 1.5;
		entry->box = OtkMakeTextFormBox( pagept, entry->text, tchars, pos_x, pos_y, twidth, box_height, 0, 0 );
		lastbox = entry->box;

		if (strcmp(txline->linename,"Status") == 0)
		 {
		  button = OtkMakePanel( pagept, Otk_Raised, Otk_LightGray, pos_x+box_width, pos_y+3.5, 3.0, 2.5 );
		  button = Otk_Make_Menu( button, 10, 10, 80, 80, "??" );
		  Otk_Add_Menu_Item( button, "Single", status_choice_S, entry);
		  Otk_Add_Menu_Item( button, "Married/Joint", status_choice_MJ, entry);
		  Otk_Add_Menu_Item( button, "Married/Sep", status_choice_MS, entry);
		  Otk_Add_Menu_Item( button, "Head_of_Household", status_choice_HH, entry);
		  Otk_Add_Menu_Item( button, "Widow(er)", status_choice_W, entry);
		  Otk_Register_Hover_Box( button, "Click to select filing status\nfrom available choices." );
		 }
		else
		if (iscapgains)
		 {
		   if (capgtoggle == 0)
		    {
		     button = OtkMakeButton( pagept, pos_x+twidth, pos_y+4.0, 1.5, 1.5, "+", add_new_capgain_boxes, entry );   /* Add more boxes - button */
		     Otk_Modify_Text_Scale( button->children, 0.8 );
		     Otk_Register_Hover_Box( button, "Add another entry box\nfor this line." );
		    }
		 }
		else
		 {
		   button = OtkMakeButton( pagept, pos_x+twidth, pos_y+4.0, 1.5, 1.5, "+", add_new_box_item, entry );   /* Add another box - button */
		   Otk_Modify_Text_Scale( button->children, 0.8 );
		   Otk_Register_Hover_Box( button, "Add another entry box\nfor this line." );
		 }
		lastline = linenum;
		break;

       case VKIND_COMMENT: // printf("%d: Comment {%s}\n", linenum, entry->comment ); 
		pos_x = leftmargin;
		pos_y = (float)linenum * 9.0 + 8.0;
		label = OtkMakeTextLabel( pagept, entry->comment, Otk_Black, 1.0, 1.0, pos_x, pos_y );
		Otk_Get_Text_Size( label, &twidth, &theight );
		if (twidth > (100.0-leftmargin-3.5))	/* Shrink width to fit on page if too long. */
		 {
		  x = (100.0-leftmargin-3.5)/twidth;
		  Otk_Modify_Text_Aspect( label, x * x );
		 }
		/* Add edit_line_comment button */
		cbutton = OtkMakeButton( pagept, 97.0, (float)linenum * 9.0 + 10.5, 1.4, 1.4, "*", edit_line_comment, entry );
	   	Otk_Modify_Text_Scale( cbutton->children, 0.9 );
		Otk_Register_Hover_Box( cbutton, "Edit the comment for\nthis line." );

		if (strstr( entry->comment, "File-name") != 0)
		 {
		  cbutton = OtkMakeButton( pagept, 91.0, (float)linenum * 9.0 + 7.25, 8, 2.5, "Browse", open_include_file, lastbox );
		  Otk_Modify_Text_Scale( cbutton->children, 0.9 );
		  Otk_Modify_Text_Color( cbutton->children, Otk_Blue );
		  Otk_Register_Hover_Box( cbutton, "Browse for tax return\noutput file to reference." );
		 }
		break;
      }
     entry = entry->nxt;
    } /*entry*/

   txline = txline->nxt;
  }

 /* Add the 'advance' and go-back' paging buttons to bottom of each page. */
 for (page=0; page<npanels; page++)
  {
   pagept = Panels->panels[page];
   if (page>0)
    {
     button = OtkMakeButton( pagept, 1.5, 96.0, 5.5, 2.5, " ", advance_panel, (void *)-1 );
     Otk_Set_Button_Icon( button, left_arrow_image );
     Otk_SetBorderThickness( button, 2.5 );
    }
   if (page<npanels-1)
    {
     button = OtkMakeButton( pagept, 93.0, 96.0, 5.5, 2.5, " ", advance_panel, (void *)1 );
     Otk_Set_Button_Icon( button, right_arrow_image );
     Otk_SetBorderThickness( button, 2.5 );
    }
  }
}




void dump_taxinfo()
{
 struct taxline_record *txline;
 struct value_list *tmppt;

 printf("\n======================================\n");
 txline = taxlines_hd;
 while (txline!=0)
  {
   printf("\n%d: %s\n", txline->linenum, txline->linename );
   tmppt = txline->values_hd;
   while (tmppt!=0)
    {
     switch (tmppt->kind)
      {
       case VKIND_FLOAT:   printf("%d,F: %6.2f\n", tmppt->linenum, tmppt->value ); break;
       case VKIND_INT:     printf("%d,I: %d\n", tmppt->linenum, (int)(tmppt->value) ); break;
       case VKIND_TEXT:    printf("%d,T: %s\n", tmppt->linenum, tmppt->text ); break;
       case VKIND_COMMENT: printf("%d,C: {%s}\n", tmppt->linenum, tmppt->comment ); break;
      }
     tmppt = tmppt->nxt;
    }
   txline = txline->nxt;
  }
 printf("\n");
}



void Save_Tax_File( char *filename )
{
 struct taxline_record *txline;
 struct value_list *tmppt;
 int lastline=-1, semicolon;
 FILE *outfile;

 /* Update the data structure(s) by getting the form fields. */
 Update_box_info();

  working_file = strdup(filename);
  outfile = fopen(filename,"w");
  if (outfile==0) 
  {
   printf("ERROR: Output file '%s' could not be opened for writing.\n", filename);
   save_taxfile();	/* Re-open file-broswer. */
   return;
  }
 working_file = strdup(filename);
 fprintf(outfile,"%s\n", title_line);
 txline = taxlines_hd;
 while (txline!=0)
  {
   fprintf(outfile,"\n%s", txline->linename );
   semicolon = 0;
   lastline = txline->linenum;
   tmppt = txline->values_hd;
   while (tmppt!=0)
    {
     if (tmppt->linenum != lastline)
      { fprintf(outfile,"\n");  lastline = tmppt->linenum; }
     switch (tmppt->kind)
      {
       case VKIND_FLOAT:   fprintf(outfile,"	%6.2f	", tmppt->value ); break;
       case VKIND_INT:     fprintf(outfile,"	%d	", (int)(tmppt->value) ); break;
       case VKIND_TEXT:    fprintf(outfile,"	%s	", tmppt->text ); break;
       case VKIND_COMMENT: if (strlen(tmppt->comment)>0) fprintf(outfile," {%s}", tmppt->comment ); break;
       case VKIND_COLON:   semicolon = 1; break;
      }
     tmppt = tmppt->nxt;
    }
   if (semicolon) fprintf(outfile,"\n		;");
   txline = txline->nxt;
  }

 fclose(outfile);
 save_needed = 0;
 printf("\nWrote form-data to file %s\n.", filename);
}



void read_file( char *filename )
{
 infile = fopen(filename,"r");
 if (infile==0) 
  {
   printf("ERROR: Input file '%s' could not be opened.\n", filename);
   open_taxfile();
   return;
  }
 else
  {
   Read_Tax_File(filename);
  }
} 

void open_taxfile()
{
 Otk_Browse_Files( "File to Open:", MaxFname, directory_dat, wildcards_dat, filename_dat, read_file );
}


void save_taxfile()
{
 Otk_Browse_Files( "File to Save:", MaxFname, directory_dat, wildcards_dat, filename_dat, Save_Tax_File );
}


void predict_output_filename(char *indatafile, char *outfname)
{
 int j;
 /* Base name of output file on input file. */
 strcpy(outfname,indatafile);
 j = strlen(outfname)-1;
 while ((j>=0) && (outfname[j]!='.')) j--;
 if (j<0) strcat(outfname,"_out.txt"); else strcpy(&(outfname[j]),"_out.txt");
}



void settxslvcmd( char *filename )
{
 strcpy(taxsolvestrng,filename);
 selected_other = 0;
 taxsolve();
}

void findtscmd()
{
 if (warningpopup)
  Otk_RemoveObject(warningpopup);  
 warningpopup = 0;
 Otk_Browse_Files( "Select TaxSolver:", MaxFname, directory_bin, wildcards_bin, taxsolvestrng, settxslvcmd);
}

void canceltxslvr()
{
 if (warningpopup)
  Otk_RemoveObject(warningpopup);  
 warningpopup = 0;
}

void killedwarning( void *x )
{
 warningpopup = 0;
}


void open_taxfile_again( void *x )
{
 canceltxslvr( 0 );
 open_taxfile( x );
}


int missingfile=0;


void filter_tabs( char *line )
{ /* Replace tabs and <cr> with spaces for clean printouts. */
 char *ptr;

 ptr = strchr( line, '\r' );
 if (ptr != 0)
  ptr[0] ='\0';

 ptr = strchr( line, '\t' );
 while (ptr != 0)
  {
   ptr[0] = ' ';
   ptr = strchr( line, '\t' );
  }
}


void taxsolve()
{
 char cmd[MaxFname], outfname[MaxFname];
 int changed=0;
 OtkWidget viewwin;

 if (working_file==0) 
  {
   if (missingfile++ == 0)
    open_taxfile(0);
   else
    {
     printf("No tax file opened.\n");  
     canceltxslvr();
     warningpopup = OtkMakeWindow( Otk_Raised, Otk_Blue, OtkSetColor(0.8,0.8,0.8), 10.0, 10.0, 80.0, 30.0 );
     Otk_RegisterWindowKillEventFunction( warningpopup, killedwarning, 0 );
     OtkMakeTextLabel( warningpopup, "No tax file selected.", Otk_Red, /*scale=*/ 1.8, /*weight=*/ 2, /*x=*/ 3, /*y=*/ 10 );
     OtkMakeTextLabel( warningpopup, "First select a tax file.", Otk_Red, /*scale=*/ 1.8, /*weight=*/ 2, /*x=*/ 10, /*y=*/ 25 );
     OtkMakeButton( warningpopup, 20, 48.0, 60, 15, "Select Tax File Now", open_taxfile_again, 0 );	
     OtkMakeButton( warningpopup, 42.5, 75.0, 15, 15, " Cancel ", canceltxslvr, 0 );
    }
   return;
  }

 if (selected_other) { taxsolvestrng[0] = '\0';  taxsolvecmd = 0; }
 if (strlen(taxsolvestrng) > 0) taxsolvecmd = taxsolvestrng;
 if (taxsolvecmd==0) taxsolvecmd = getenv("taxsolvecmd");

 changed = Update_box_info();
 if (changed)
  {
   printf("Changes not saved !!\n");   
   canceltxslvr();
   warningpopup = OtkMakeWindow( Otk_Raised, Otk_Blue, OtkSetColor(0.8,0.8,0.8), 10.0, 10.0, 80.0, 30.0 );
   Otk_RegisterWindowKillEventFunction( warningpopup, killedwarning, 0 );
   OtkMakeTextLabel( warningpopup, "Changes not saved !!", Otk_Red, /*scale=*/ 1.8, /*weight=*/ 2, /*x=*/ 3, /*y=*/ 10 );
   OtkMakeTextLabel( warningpopup, "Save first, then compute.", Otk_Red, /*scale=*/ 1.8, /*weight=*/ 2, /*x=*/ 3, /*y=*/ 30 );
   OtkMakeButton( warningpopup, 42, 75.0, 10, 15, " OK ", canceltxslvr, 0 );
   return;   
  }

 if (taxsolvecmd==0)
  {
   printf("The 'taxsolvecmd' environment variable is not set.\n");   
   canceltxslvr();
   warningpopup = OtkMakeWindow( Otk_Raised, Otk_Blue, OtkSetColor(0.8,0.8,0.8), 10.0, 10.0, 80.0, 30.0 );
   Otk_RegisterWindowKillEventFunction( warningpopup, killedwarning, 0 );
   OtkMakeTextLabel( warningpopup, "The 'taxsolvecmd' environment", Otk_Red, /*scale=*/ 1.8, /*weight=*/ 2, /*x=*/ 3, /*y=*/ 10 );
   OtkMakeTextLabel( warningpopup, "variable is not set.", Otk_Red, /*scale=*/ 1.8, /*weight=*/ 2, /*x=*/ 10, /*y=*/ 25 );
   OtkMakeButton( warningpopup, 20, 48.0, 60, 15, "Select TaxSolver Now", findtscmd, 0 );	
   OtkMakeButton( warningpopup, 42.5, 75.0, 15, 15, " Cancel ", canceltxslvr, 0 );	
   return;   
  }
 if (PLATFORM_KIND==Posix_Platform)
  sprintf(cmd,"'%s' '%s' &", taxsolvecmd, working_file );
 else
  {
   if ((strlen(taxsolvecmd) > 2 ) && (taxsolvecmd[1] == ':') && (taxsolvecmd[2] != '"')) 
    { /*Insert quotes around file name for Microsoft, in case pathname has spaces in it.*/
     int j;
     char *tstr;
     j = strlen( taxsolvecmd ) + 10;
     tstr = (char *)malloc(j);
     for (j = 0; j < 2; j++) tstr[j] = taxsolvecmd[j];
     j = 2;	/* Leading quote must be inserted after drive letter for Microsoft OS's. */
     tstr[2] = '"';  j++;
     do { tstr[j] = taxsolvecmd[j-1];  j++; } while (tstr[j-1] != '\0');
     strcat( tstr, "\"" );
     taxsolvecmd = tstr;
    }
   sprintf(cmd,"%s \"%s\"", taxsolvecmd, working_file );
  }

 printf("Invoking '%s'\n", cmd );
 system(cmd);	/* Invoke the TaxSolver. */

 /* Make a popup message telling where the results are. */
 predict_output_filename(working_file, outfname);
 canceltxslvr();
 warningpopup = OtkMakeWindow( Otk_Raised, Otk_Blue, OtkSetColor(0.8,0.8,0.6), 10.0, 8.0, 80.0, 85.0 );
 Otk_RegisterWindowKillEventFunction( warningpopup, killedwarning, 0 );
 OtkMakeTextLabel( warningpopup, "Results written to file:", Otk_Black, /*scale=*/ 1.8, /*weight=*/ 2, /*x=*/ 3, /*y=*/ 5 );
 OtkMakeTextLabel( warningpopup, outfname, Otk_Blue, /*scale=*/ 1.2, /*weight=*/ 1.2, /*x=*/ 5, /*y=*/ 10 );
 OtkMakeTextLabel( warningpopup, "Preview:", Otk_Black, /*scale=*/ 1.2, /*weight=*/ 1.2, /*x=*/ 3, /*y=*/ 17 );
 viewwin = Otk_Make_Selection_List( warningpopup, 20, 50, 5, 20, 87, 67 );
 Otk_Force_Redraw();
 OtkUpdateCheck();
 otk_sleep( 0.5 );
 { FILE *viewfile;
   char vline[5000];
   viewfile = fopen( outfname, "rb" );
   if (viewfile == 0)
    {
     sprintf(vline,"Cannot open: %s", outfname);
     Otk_Add_Selection_Item( viewwin, vline, 0, 0 ); 
    }
   else
    {
     fgets( vline, 256, viewfile );
     while (!feof(viewfile))
      {
       filter_tabs( vline );
       Otk_Add_Selection_Item( viewwin, vline, 0, 0 );
       fgets( vline, 256, viewfile );
      }
    }
 }
 OtkMakeButton( warningpopup, 44, 92.0, 12, 4, " OK ", canceltxslvr, 0 );	
 Otk_Force_Redraw();
 Otk_Display_Changed++;
}



OtkWidget printpopup=0, printerformbox, printresultstog;
char printer_command[MaxFname], wrkingfname[MaxFname];
#if (PLATFORM_KIND==POSIX_PLATFORM)
 char base_printer_command[]="lpr ";
#else
 char base_printer_command[]="print ";
#endif

void cancelprintpopup0()
{ printpopup = 0; }

void cancelprintpopup()
{ 
 Otk_RemoveObject(printpopup); 
 printpopup = 0;
}

void togprntcmd_in(void *x)
{ 
 sprintf(printer_command,"%s \"%s\"", base_printer_command, wrkingfname);
 Otk_Modify_Text( printerformbox, printer_command );
}

void togprntcmd_out(void *x)
{ char tmpstr[MaxFname];
  int k;
 predict_output_filename(wrkingfname,tmpstr);
 sprintf(printer_command,"%s \"%s\"", base_printer_command, tmpstr);
 Otk_Modify_Text( printerformbox, printer_command );
}

void acceptprinter_command( void *data )
{
 Otk_Get_Text( printerformbox, printer_command, MaxFname );
 printf("Issuing: %s\n", printer_command);
 system(printer_command);
 cancelprintpopup();
 refresh();
}

void acceptprinter_command2( char *s, void *x ) { acceptcomment( x ); }

void printout( void *data )
{
 if (printpopup!=0) return;
 printpopup = OtkMakeWindow( Otk_Raised, Otk_Blue, OtkSetColor(0.8,0.8,0.8), 10.0, 10.0, 80.0, 35.0 );
 OtkMakeTextLabel( printpopup, "Print Return Data:", Otk_Black, /*scale=*/ 1.9, /*weight=*/ 1, /*x=*/ 2, /*y=*/ 3 );

 OtkMakeTextLabel( printpopup, "Print Input Data", Otk_Black, /*scale=*/ 1.5, /*weight=*/ 1, /*x=*/ 20, /*y=*/ 20.0 );
 printresultstog = OtkMakeRadioButton( printpopup, 53.0, 20.0, 10.0, 6.0, togprntcmd_in, 0 );
 OtkMakeTextLabel( printpopup, "Print Output Results", Otk_Black, /*scale=*/ 1.5, /*weight=*/ 1, /*x=*/ 20, /*y=*/ 35.0 );
 OtkMakeRadioButton( printresultstog, 53.0, 35.0, 10.0, 6.0, togprntcmd_out, 0 );
 Otk_Add_BoundingBox( printpopup, Otk_Blue, 1.0, 18.0, 16.0, 63.0, 47.0 );

 if (working_file==0) strcpy(wrkingfname,filename_dat); else strcpy(wrkingfname,working_file);
 sprintf(printer_command,"%s %s", base_printer_command, wrkingfname);

 OtkMakeTextLabel( printpopup, "Print Command:", Otk_Black, /*scale=*/ 1.5, /*weight=*/ 1, /*x=*/ 4, /*y=*/ 57 );
 printerformbox = OtkMakeTextFormBox( printpopup, printer_command, 60,  28.5, 55, 68, 18, acceptprinter_command2, 0 );

 OtkMakeButton( printpopup, 10, 80.0, 22, 12, " Print It", acceptprinter_command, 0 );	
 OtkMakeButton( printpopup, 80, 81.0, 14, 11, " Cancel ", cancelprintpopup, 0 );	
 Otk_RegisterWindowKillEventFunction( printpopup, cancelprintpopup0, 0 );
}





void change_highlighted_form_button()
{
 last_tog->object_subtype = Otk_subtype_plane;			/* Unselect the old button. */
 Otk_Set_Button_State(selected_button[ selected_form ], 0 );	/* Raise the new button. */
 last_tog = selected_button[ selected_form ];			/* Remember the new mode. */
}



char program_names[20][100] = 
	{
	 "taxsolve_US_1040_2014",		/* 0 */
	 "taxsolve_US_1040_Sched_C_2014",	/* 1 */
	 "taxsolve_CA_540_2014",		/* 2 */
	 "taxsolve_MA_1_2014",			/* 3 */
	 "taxsolve_NC_D400_2014",		/* 4 */
	 "taxsolve_NJ_1040_2014",		/* 5 */
	 "taxsolve_NY_IT201_2014",		/* 6 */
	 "taxsolve_OH_IT1040_2014",		/* 7 */
	 "taxsolve_PA_40_2014",			/* 8 */
	 "taxsolve_VA_760_2014",		/* 9 */
	 "Other",				/* 10 */
	 "taxsolve_US_8829"			/* 11 */
	};



void slcttxprog( void *data )
{
 char *sel=(char *)data;
 char strg[MaxFname], tmpstr[MaxFname];
 int k;

 if (sscanf(sel,"%d",&selected_form) != 1) printf("Internal error '%s'\n", sel );
 strcpy( strg, program_names[selected_form] );
 change_highlighted_form_button();
 printf("Selected tax program '%s'\n", strg);
 if (strcmp(strg,"Other")==0)
  {
   selected_other = 1;
   strcpy(tmpstr,invocation_path);
   k = strlen(tmpstr)-1;
   while ((k>0) && (tmpstr[k]!=slashchr)) k--;
   if (k>0) k--;
   while ((k>0) && (tmpstr[k]!=slashchr)) k--;
   if (tmpstr[k]==slashchr)  tmpstr[k+1] = '\0';
   else  {sprintf(tmpstr,".%c", slashchr);}
   sprintf(directory_dat,"%sexamples_and_templates%c", tmpstr, slashchr);
   Otk_Browse_Files( "Select TaxForm:", MaxFname, directory_dat, wildcards_dat, taxsolvestrng, open_taxfile );
   return;
  }
 else
  {
   selected_other = 0;
   sprintf(tmpstr,"%s%s", invocation_path, strg);
   printf("Setting Tax Program to be: '%s'\n", tmpstr);
   taxsolvecmd = strdup(tmpstr);
   strcpy(taxsolvestrng,tmpstr);

   strcpy(tmpstr,invocation_path);
   k = strlen(tmpstr)-1;
   while ((k>0) && (tmpstr[k]!=slashchr)) k--;
   if (k>0) k--;
   while ((k>0) && (tmpstr[k]!=slashchr)) k--;
   if (tmpstr[k]==slashchr)  tmpstr[k+1] = '\0';
   else  {sprintf(tmpstr,".%c", slashchr);}
   sprintf(directory_dat,"%sexamples_and_templates%c", tmpstr, slashchr);

   sel = strstr( strg, "_2014" );
   if (sel != 0)
     sel[0] = '\0';
   strcpy( tmpstr, strg );
   if (strlen(tmpstr) < 10) {printf("Internal error, name too short '%s'\n", tmpstr); return;}
   strcpy( strg, &(tmpstr[9]) );	/* Remove the prefix "taxsolve_". */
   strcat(directory_dat, strg );
   printf("Setting Tax Data Directory to be: '%s'\n", directory_dat);
  }
}


void radiobuttoncallback( void *x )
{
 char *sel=(char *)x;

 sscanf(sel,"%d",&selected_form);
 Otk_SetRadioButton( selected_tog[ selected_form ] );
 slcttxprog( x );
}


void Ok2Quit()
{
 printf("OTS Exiting.\n");
 exit(0);
}

void QuitCallback( void *x )
{
 popup = OtkMakeWindow( Otk_Raised, Otk_Blue, OtkSetColor(0.8,0.8,0.8), 20.0, 54.0, 65.0, 20.0 );
 OtkMakeTextLabel( popup, "Ok to QUIT ?", OtkSetColor(0.6,0.0,0.0), /*scale=*/ 3.0, /*weight=*/ 2, /*x=*/ 19, /*y=*/ 18 );
 Otk_Set_Default_Button_BorderThickness( 2.0 );
 OtkMakeButton( popup, 14, 70.0, 20, 18, " Ok Quit ", Ok2Quit, 0);	
 OtkMakeButton( popup, 72, 70.0, 16, 18, "Cancel ", cancelpopup, 0 );	
 Otk_Set_Default_Button_BorderThickness( 1.0 );
}


#include "logo_image.c"   /* <--- The image-data is included here, produced by "image2code". */
#include "decode_image_data.c"  /* Re-Usable Routines */



/*----------------------------------------------------------------------------*/
/* Main -								      */
/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[] )
{
 int argn, k, scrn_width, scrn_height, winwidth, winht;
 char vrsnmssg[50], tmpstr[MaxFname];
 OtkWidget subpanel, txprogstog;
 float x, y, dy, fontsz, aspect_ratio=0.88;
 FILE *testfile;
 struct Otk_image *logo_image;

 /* Decode any command-line arguments. */
 argn = 1;  k=1;
 while (argn < argc)
 {
  if (strcmp(argv[argn],"-verbose")==0)  { verbose = 1;  argv[argn][0] = '\0'; }
  else
  if (strstr(argv[argn],"-otk_") == argv[argn]) { ; }
  else
  if (strncmp(argv[argn],"-aspect=",8) == 0)  { aspect_ratio = atof(argv[argn]+8); }
  else
  if (k==1)
   {
    working_file = strdup(argv[argn]);
    infile = fopen(working_file,"r");
    if (infile==0) {printf("ERROR: Parameter file '%s' could not be opened.\n", argv[argn]); exit(1);}
    k = 2;
   }
  else
   {printf("Unknown command-line parameter '%s'\n", argv[argn]); /* exit(1); */ }
  argn = argn + 1;
 }
 sprintf(vrsnmssg,"OTS GUI v%1.2f", version);  printf("%s\n\n",vrsnmssg);
 invocation_path = strdup(argv[0]);
 k = strlen(invocation_path)-1;
 while ((k>0) && (invocation_path[k]!=slashchr)) k--;
 if (invocation_path[k]==slashchr) k++;
 invocation_path[k] = '\0';
 printf("Invocation path = '%s'\n", invocation_path);
 strcpy(directory_bin,invocation_path);

 Otk_Set_Window_Name( "OpenTaxSolver-GUI" );
 Otk_Get_Screen_Size( &scrn_width, &scrn_height );
 winht = 0.9 * scrn_height;
 if (winht > 950) winht = 950;
 winwidth = aspect_ratio * (float)winht;
 if (winwidth > scrn_width) winwidth = scrn_width - 50;
 OtkInitWindow( winwidth, winht, argc, argv );	 /* Open a window. */
 Otk_Set_Render_Quality_Hint( 2.0 );

 bckgrnd = OtkMakePanel( OtkOuterWindow, Otk_Flat, Otk_LightGray, 0, 0, 100, 100 );
 title_panel = 
 OtkMakePanel( bckgrnd, Otk_Raised, Otk_LightGray, 1, 0.75, 98, 6 );	/* Title Panel. */
 Otk_SetBorderThickness( title_panel, 0.7 );
 title_label = OtkMakeTextLabel( title_panel, "Open-Tax-Solver", Otk_Blue, /*scale=*/ 3, /*weight=*/ 2, /*x=*/ 27, /*y=*/ 10 );

 main_panel = 
 OtkMakePanel( bckgrnd, Otk_Raised, Otk_LightGray, 1, 7.5, 98, 87 );	/* Main Panel. */
 Otk_SetBorderThickness( main_panel, 0.25 );
 subpanel = OtkMakePanel( main_panel, Otk_Raised, Otk_LightGray, 18.5, 2, 63, 17 );
 Otk_SetBorderThickness( subpanel, 0.75 );

 /* Use self-contained image, to avoid needing to find external image-file. */
 logo_image = Otk_Make_Image_From_Matrix( "LogoImage0", 92, 512, decode_image( imgmtrx_N0, 92, 512, imgmtrxbytes0 ) );
 OtkMakeImagePanel_ImgPtr( subpanel, logo_image, 1.5, 5.0, 96.8, 89.85 );

 OtkMakeTextLabel( main_panel, "2014", Otk_DarkGray, /*scale=*/ 1.6, /*weight=*/ 1, /*x=*/ 47, /*y=*/ 20 );

 taxsolvecmd = getenv("taxsolvecmd");
 if (taxsolvecmd==0)
  {
   int  US_1040_enabled=1, US_1040_Sched_C_enabled=1, US_8829_enabled=1, State_CA_540_enabled=1, State_NC_DC400_enabled=1,
	State_OH_IT1040_enabled=1, State_NJ_1040_enabled=1, State_PA_40_enabled=1, State_VA_760_enabled=1,
	State_NY_IT201_enabled=1, State_MA_1_enabled=1;

   OtkMakeTextLabel( main_panel, "Select Tax Program:", Otk_Black, /*scale=*/ 2, /*weight=*/ 1, /*x=*/ 3, /*y=*/ 26 );
   x = 8.0;  y = 33.0;  
   dy = 6.0;		// 5.5;  
   fontsz = 1.45;	// 1.4
   Otk_Set_Default_Button_Color( 0.8, 0.8, 0.8 );
   Otk_Set_Button_Outline_Style( Otk_Flat );

   if (US_1040_enabled)
    {
     txprogstog = OtkMakeRadioButton( main_panel, x, y, 6.0, 3.0, slcttxprog, "0" );
     selected_tog[0] = txprogstog;
     selected_button[0] = OtkMakeButton( main_panel, x+5.4, y+0.2, 34.5, 4.2, "US 1040 (w/Scheds A,B,D)", radiobuttoncallback, "0" );
     last_tog = selected_button[0];
     slcttxprog("0");	 /* Set as default solver. */
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "US 1040 (w/Scheds A,B,D)", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;

   if (US_1040_Sched_C_enabled)
    {
     selected_tog[1] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "1" );
     selected_button[1] = OtkMakeButton( main_panel, x+6.0, y+0.2, 22.0, 4.2, "US 1040 Sched C ", radiobuttoncallback, "1" );
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "US 1040 Sched C", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;

   if (US_8829_enabled)
    {
     selected_tog[11] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "11" );
     selected_button[11] = OtkMakeButton( main_panel, x+6.0, y+0.2, 13.0, 4.2, " US 8829  ", radiobuttoncallback, "11" );
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "US 8829", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;

		/* Discontinuing California taxes, due to state issuing free auto-fill forms. */
   if (State_CA_540_enabled)
    {
     selected_tog[2] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "2" );
     selected_button[2] = OtkMakeButton( main_panel, x+6.0, y+0.2, 18.0, 4.2, "CA State 540", radiobuttoncallback, "2" );
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "CA State 540", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;


   if (State_NC_DC400_enabled)
    {
     selected_tog[4] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "4" );
     selected_button[4] = OtkMakeButton( main_panel, x+6.0, y+0.2, 21.0, 4.2, "NC State DC400 ", radiobuttoncallback, "4" );
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "NC State DC400", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;

   if (State_NJ_1040_enabled)
    {
     selected_tog[5] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "5" );
     selected_button[5] = OtkMakeButton( main_panel, x+6.0, y+0.2, 20.0, 4.2, "NJ State 1040", radiobuttoncallback, "5" );
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "NJ State 1040", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;

   x = 60.0;  y = 33.0;

   if (State_OH_IT1040_enabled)
    {
     selected_tog[7] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "7" );
     selected_button[7] = OtkMakeButton( main_panel, x+6.0, y+0.2, 22.0, 4.2, "OH State IT1040", radiobuttoncallback, "7" );
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "OH State IT1040", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;

   if (State_PA_40_enabled)
    {
     selected_tog[8] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "8" );
     selected_button[8] = OtkMakeButton( main_panel, x+6.0, y+0.2, 17.0, 4.2, "PA State 40", radiobuttoncallback, "8" );
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "PA STATE 40", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;

   if (State_VA_760_enabled)
    {
     selected_tog[9] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "9" );
     selected_button[9] = OtkMakeButton( main_panel, x+6.0, y+0.2, 18.5, 4.2, "VA State 760", radiobuttoncallback, "9" );
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "VA State 760", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;

   if (State_NY_IT201_enabled)
    {
     selected_tog[6] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "6" );
     selected_button[6] = OtkMakeButton( main_panel, x+6.0, y+0.2, 21.0, 4.2, "NY State IT201", radiobuttoncallback, "6" );
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "NY State IT201", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;

   if (State_MA_1_enabled)
    {
     selected_tog[3] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "3" );
     selected_button[3] = OtkMakeButton( main_panel, x+6.0, y+0.2, 15.0, 4.2, "MA State 1", radiobuttoncallback, "3" );
    }
   else
    {
     OtkMakePanel( main_panel, Otk_Recessed, Otk_LightGray, x+1.0, y+0.7, 3.0, 2.0 );
     OtkMakeTextLabel( main_panel, "MA State 1", Otk_Gray, /*scale=*/ fontsz, /*weight=*/ 1, /*x=*/ x+7.0, y+0.6 );
    }
   y = y + dy;

   selected_tog[10] = OtkMakeRadioButton( txprogstog, x, y, 6.0, 3.0, slcttxprog, "10" );
   selected_button[10] = OtkMakeButton( main_panel, x+6.0, y+0.2, 10.0, 4.2, "Other ", radiobuttoncallback, "10" );

   Otk_Set_Button_Outline_Style( Otk_Raised );
   Otk_Add_BoundingBox( main_panel, Otk_Blue, 1.0, 6.0, 31.5, 95.0, 69.5 );
  }
 Otk_Set_Default_Button_BorderThickness( 2.0 );
 OtkMakeButton( bckgrnd, 82, 95.3, 15, 4, "Exit", QuitCallback, 0 );	/* Exit Button. */
 Otk_Set_Default_Button_BorderThickness( 1.0 );
 Otk_RegisterWindowKillEventFunction( bckgrnd, QuitCallback, 0 );
 Otk_ignore_file( "README_", 0 );
 Otk_ignore_file( "_out.txt", 0 );

 if (infile==0) 
  {
   OtkMakeButton( main_panel, 10, 75, 80, 10, "Open a Tax File (or Template)",  open_taxfile, 0 );
   OtkMakeTextLabel( main_panel, vrsnmssg, Otk_Black, /*scale=*/ 1, /*weight=*/ 1, /*x=*/ 42, /*y=*/ 95 );
  }
 else
   Read_Tax_File(working_file);

 left_arrow_image = Otk_Make_Image_From_Matrix( "YourImage1", 14, 35, decode_image( imgmtrx_N1, 14, 35, imgmtrxbytes1 ) );
 right_arrow_image = Otk_Make_Image_From_Matrix( "YourImage2", 14, 35, decode_image( imgmtrx_N2, 14, 35, imgmtrxbytes2 ) );

 OtkMainLoop();
 return 0;
}
