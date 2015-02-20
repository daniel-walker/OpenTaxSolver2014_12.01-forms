/********************************************************/
/* OTS_GUI.c - OpenTaxSolver Graphical User Interface.	*/
/* Based on GimpToolKit (Gtk) widgets.			*/
/*							*/
/* This version is based on Gtk widgets.		*/
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
/* To compile this graphical program, you will need the */
/* Gtk library.						*/
/*							*/
/* Compile:						*/
/*  cc -O `pkg-config --cflags gtk+-2.0` ots_gui2.c  \
 	`pkg-config --libs gtk+-2.0`  -o ots_gui2	*/
/*							*/
/********************************************************/

float version=2.03;

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
/*	a line-label.  So all field values should be        */
/*	indented to use the GUI with them.		    */
/************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
// #include "backcompat.c"
#include "gtk_utils.c"		/* Include the graphics library. */

 GtkWidget *mpanel, *mpanel2, *warnwin=0, *popupwin=0, *resultswindow=0, *scrolledpane;
 int operating_mode=1, need_to_resize=0;
 double last_resize_time;

int verbose=0;
int winwidth=450, winht=480;
FILE *infile;
int ots_column=0, ots_line=0;	/* Input file position. */
#define MaxFname 4096
char wildcards_bin[MaxFname]="", filename_exe[MaxFname]="", *ots_path;
char directory_dat[MaxFname]=".", wildcards_dat[MaxFname]="*.txt", *filename_dat=0;
char directory_incl[MaxFname]="examples_and_templates", wildcards_incl[MaxFname]="*_out.txt";
char title_line[MaxFname], *current_working_filename=0, *invocation_path, *include_file_name=0;
char wildcards_out[MaxFname]="*_out.txt";
int fronty1, fronty2, computed=0, ok_slcttxprog=1;
char *yourfilename=0;

void pick_file( GtkWidget *wdg, void *data );	/* Prototype */
void Run_TaxSolver( GtkWidget *wdg, void *x );
void dump_taxinfo();
int warn_release=0;

int selected_form=11;
int selected_other=0;
void save_taxfile( GtkWidget *wdg, void *data );
void printout( GtkWidget *wdg, void *data );
void taxsolve();
char *taxsolvecmd=0, taxsolvestrng[MaxFname]="";

#define VKIND_FLOAT   0
#define VKIND_INT     1
#define VKIND_TEXT    2
#define VKIND_COMMENT 3
#define VKIND_COLON   4

#define VALUE_LABEL   0
#define COMMENT       1
#define SEMICOLON     2
#define NOTHING	     10
#define ENABLED       1
#define DISABLED      0


void dismiss_general_warning( GtkWidget *wdg, void *data )
{
 switch (warn_release)
  {
   case 1:  pick_file( 0, 0 );
	    warn_release = 0;
	    close_any_window( 0, data );
	break;
   case 2:  save_taxfile(0,0);	/* Re-open file-broswer. */
	    warn_release = 0;
	    close_any_window( 0, data );
	break;
   default:  close_any_window( 0, data );
  }
}


char wmsg[4096], *pending_message=0;

void GeneralWarning( char *mesg )       /* Used for one-line messages. */
{
 int xpos=20, ypos=20, winwdth, winhght=100;
 GtkWidget *winframe;
 printf("%s\n", mesg);
 winwdth = 90 + strlen( mesg ) * 8;
 winframe = new_window( winwdth, winhght, "Warning Message", &warnwin );
 make_label( winframe, xpos, ypos, mesg );
 make_button( winframe, winwdth/2 - 30, winhght - 40, "  Ok  ", dismiss_general_warning, &warnwin );
 gtk_window_set_keep_above( (GtkWindow *)warnwin, 1 );
 show_wind( warnwin );
}



struct value_list
 {
  int	    kind;	/* 0=float, 1=integer, 2=text, 3=comment. */
  float     value;
  char      *comment, *text;
  int       column, linenum;
  struct taxline_record *parent;
  GtkEntry  *box;
  GtkWidget *comment_label;
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

 tmppt = (struct taxline_record *)calloc( 1, sizeof(struct taxline_record) );
 tmppt->linename = strdup(linename);
 tmppt->linenum = linenum;
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
       get_formbox_text( tmppt->box, text, 1024 );	
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
 gtk_widget_destroy( mpanel2 );		/* Clear out panel. */
 /* Set up new panel with scrollbars for tax form data. */
 mpanel2 = gtk_fixed_new();
 scrolledpane = gtk_scrolled_window_new( 0, 0 );
 gtk_scrolled_window_set_policy( (GtkScrolledWindow *)scrolledpane, GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS );
 gtk_scrolled_window_add_with_viewport( (GtkScrolledWindow *)scrolledpane, mpanel2 );
 //   gtk_container_add( GTK_CONTAINER( mpanel ), scrolledpane );
 gtk_fixed_put( GTK_FIXED( mpanel ), scrolledpane, 0, 35 );
 gtk_widget_set_size_request( scrolledpane, winwidth, winht - 80 );
 DisplayTaxInfo();
 gtk_widget_show_all( outer_window );
}



/*--------------------------------------------------------------*/
/* Add_New_Boxes - Callback for "+" button on form-boxes.	*/
/*  Adds new form-box(s) to the line item.			*/
/*--------------------------------------------------------------*/
void add_new_boxes( void *data, int num )
{
 struct taxline_record *txline;
 struct value_list *item, *lineitem, *newitem1, *newitem2, *oldtail;
 double vpos;
 GtkAdjustment *adj;

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

 /* Prestore the scrolling position. */
 adj = gtk_scrolled_window_get_vadjustment( (GtkScrolledWindow *)scrolledpane );
 // printf("Adj = (%x), %g, (%g, %g), (%g, %g), %g\n", adj, adj->value, adj->lower, adj->upper, adj->step_increment, adj->page_increment, adj->page_size );
 vpos = gtk_adjustment_get_value( adj );
 // vpos = adj->value;

 refresh();

 /* Restore the scrolling position. */
 adj = gtk_scrolled_window_get_vadjustment( (GtkScrolledWindow *)scrolledpane );
 // gtk_adjustment_set_value( adj, vpos );	/* Isn't working because at this time upper and lower are 0 and 1. */
 adj->value = vpos;
 // gtk_scrolled_window_set_vadjustment( (GtkScrolledWindow *)scrolledpane, adj );  	/* Not needed. */
}


/*-------------------------------------------------------------------------*/
/* Add_new_capgain_boxes - Callback for "+" button on cap-gain form-boxes. */
/*  Adds a new form-boxes to the line item.				   */
/*-------------------------------------------------------------------------*/
void add_new_capgain_boxes( GtkWidget *wdg, void *data )
{
 add_new_boxes( data, 2 ); 
}

void add_new_box_item( GtkWidget *wdg, void *data )
{
 add_new_boxes( data, 1 );
}



GtkEntry *commentbox;

void cancelpopup( GtkWidget *wdg, void *data )
{ gtk_widget_destroy( popupwin );  popupwin = 0; }


void acceptcomment( GtkWidget *wdg, void *data )
{
 char *comment;
 struct value_list *tmppt;

 tmppt = (struct value_list *)data;
 comment = get_formbox( commentbox );
 // printf("Prior comment was '%s', new comment is '%s'\n", tmppt->comment, comment );
 if (tmppt->comment != 0) 
  {
   if (strcmp( tmppt->comment, comment ) != 0) save_needed++;
   free( tmppt->comment );
  } else save_needed++;
 tmppt->comment = strdup( comment );
 modify_label( tmppt->comment_label, comment );
 cancelpopup(0,0);
 // refresh();
}


void edit_line_comment( GtkWidget *wdg, void *data )	/* Edit_comment. */
{
 struct value_list *tmppt;
 int winwidth=510, winht=80;
 GtkWidget *panel;

 tmppt = (struct value_list *)data;
 if (popupwin) gtk_widget_destroy( popupwin );
 panel = new_window( winwidth, winht, "Edit Comment", &popupwin );
 make_label( panel, 2, 2, "Edit Line Comment:" );
 commentbox = new_formbox_bypix( panel, 10, 25, winwidth - 40, tmppt->comment, 500, acceptcomment, tmppt );
 make_button( panel, 20, winht - 30, " Ok ", acceptcomment, tmppt );
 make_button( panel, winwidth - 60, winht - 30, "Cancel", cancelpopup, 0 );
 gtk_widget_show_all( popupwin );
}



void quit_wcheck( GtkWidget *wdg, void *x );		/* Protoyype */



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


#ifndef PLATFORM_KIND
 #define Posix_Platform  0 
 #define Mingw_Platform  1
 #define MsVisC_Platform 2
 #ifdef __CYGWIN32__
  #ifndef __CYGWIN__
   #define __CYGWIN__ __CYGWIN32__
  #endif
 #endif
 #if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MSYS__)
  #define PLATFORM_KIND Mingw_Platform /* MinGW or like platform */
 #elif defined(__WIN32) || defined(WIN32)
  #define PLATFORM_KIND MsVisC_Platform /* microsoft visual C */
 #else
  #define PLATFORM_KIND Posix_Platform    /* Posix/Linux/Unix */
 #endif
#endif


#if (PLATFORM_KIND != Posix_Platform) 
 char slashchr='\\';
 char slashstr[]="\\";
#else
 char slashchr='/';
 char slashstr[]="/";
#endif


char *taxform_name;


/***********************/
/* Read Tax Data File. */
/***********************/
void Read_Tax_File( char *fname )
{
 int j, k, kind, state=0, column=0, linenum=0, linecnt=0, lastline=0, newentry=0, entrycnt=0;
 char word[10000], *tmpstr, tmpstr2[100], tmpstr3[100];
 struct taxline_record *txline=0;
 struct value_list *tmppt, *newitem, *oldtail;

 /* Read the Tax Data Form File. */
 current_working_filename = strdup(fname);
 taxlines_hd = 0;
 /* Accept the form's Title line.  (Must be first line!) */
 fgets(word, 200, infile);
 strcpy(title_line, word);
 j = strlen(word);
 if (j>0) word[j-1] = '\0';
 // printf("Title: '%s'\n", word);
 if (strstr(word,"Title:")==word) tmpstr = &(word[6]); else tmpstr = &(word[0]);
 k = strlen(tmpstr);	/* Pad to center if title is too short. */
 if (k < 20)
  { for (j=0; j<(20-k)/2; j++) tmpstr2[k]=' '; tmpstr2[(20-k)/2] = '\0'; 
    strcpy(tmpstr3,tmpstr2); strcat(tmpstr3,tmpstr); strcpy(tmpstr,tmpstr3); strcat(tmpstr,tmpstr2);
  }
 taxform_name = strdup( tmpstr );

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
}



void Setup_Tax_Form_Page()	/* This is called whenever the form window needs to be redisplayed for any reason. */
{
 GtkWidget *button, *title_label;
 GtkRequisition actual;
 int x1=1;
 char *twrd;
 float fontsz;

 gtk_widget_destroy( mpanel );	/* Clear out panel. */
 mpanel = gtk_fixed_new();
 gtk_container_add( GTK_CONTAINER( outer_window ), mpanel );
 /* Set up new panel with scrollbars for tax form data. */
 mpanel2 = gtk_fixed_new();
 scrolledpane = gtk_scrolled_window_new( 0, 0 );
 gtk_scrolled_window_set_policy( (GtkScrolledWindow *)scrolledpane, GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS );
 gtk_scrolled_window_add_with_viewport( (GtkScrolledWindow *)scrolledpane, mpanel2 );
 //   gtk_container_add( GTK_CONTAINER( mpanel ), scrolledpane );
 gtk_fixed_put( GTK_FIXED( mpanel ), scrolledpane, 0, 35 );
 gtk_widget_set_size_request( scrolledpane, winwidth, winht - 80 );
 operating_mode = 2;

 button = make_button( mpanel, 25, winht - 35, "  Save  ", save_taxfile, 0 );	/* The "Save" button. */
 add_tool_tip( button, "Save your changes." );

 button = make_button( mpanel, winwidth/4 + 5, winht - 35, "Compute Tax", Run_TaxSolver, 0 );
 add_tool_tip( button, "Run TaxSolver." );

 button = make_button( mpanel, 3 * winwidth / 5, winht - 35, " Print ", printout, 0 );
 add_tool_tip( button, "Print results." );

 button = make_button( mpanel, winwidth - 60, winht - 35, " Exit ", quit_wcheck, 0 );
 add_tool_tip( button, "Leave this program." );

 twrd = (char *)malloc( strlen( taxform_name ) + 100 );
 strcpy( twrd, "<b>" );
 strcat( twrd, taxform_name );
 strcat( twrd, "</b>" );
 fontsz = 12.0;
 title_label = make_sized_label( mpanel, x1, 10, twrd, fontsz );   /* Temporarily make label to get its size. */
 gtk_widget_size_request( (GtkWidget *)title_label, &actual );
 if (actual.width > winwidth)
  fontsz = fontsz * (float)winwidth / (float)actual.width;
 else
  x1 = (winwidth - (actual.width + 20)) / 2;
 if (x1 < 0) x1 = 0;
 gtk_widget_destroy( title_label );
 title_label = make_sized_label( mpanel, x1, 10, twrd, fontsz );    /* Remake label in centered position. */
 set_widget_color( title_label, "#0000ff" );
 free( twrd );

 DisplayTaxInfo();
 gtk_widget_show_all( outer_window );
}


void Get_Tax_Form_Page( char *fname )		/* This is only called once, to bring up the initial form. */
{
 Read_Tax_File( fname );
 Setup_Tax_Form_Page();
}


void Get_New_Tax_Form_Page( char *fname )	/* Transition from initial opening window to tax-form window. */
{
 winwidth = 680;
 winht = 700;
 gtk_window_resize( (GtkWindow *)outer_window, winwidth, winht );
 Get_Tax_Form_Page( fname );
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



void status_choice_S( GtkWidget *wdg, void *x )
{ struct value_list *tmppt=(struct value_list *)x;
  modify_formbox( tmppt->box, "Single" );
}

void status_choice_MJ( GtkWidget *wdg, void *x )
{ struct value_list *tmppt=(struct value_list *)x;
  modify_formbox( tmppt->box, "Married/Joint" );
}

void status_choice_MS( GtkWidget *wdg, void *x )
{ struct value_list *tmppt=(struct value_list *)x;
  modify_formbox( tmppt->box, "Married/Sep" );
}

void status_choice_HH( GtkWidget *wdg, void *x )
{ struct value_list *tmppt=(struct value_list *)x;
  modify_formbox( tmppt->box, "Head_of_Household" );
}

void status_choice_W( GtkWidget *wdg, void *x )
{ struct value_list *tmppt=(struct value_list *)x;
  modify_formbox( tmppt->box, "Widow(er)" );
}





GtkEntry *active_entry;

void set_included_file( GtkWidget *wdg, void *fs )
{
 if (include_file_name != 0) free( include_file_name );
 include_file_name = strdup( gtk_file_selection_get_filename( (GtkFileSelection *)fs ) );
 if (strstr( include_file_name, " "))
  { /* If filename contains white-space, then add quotes around it. */
   char tmpfname[MaxFname]="\"";
   strcat( tmpfname, include_file_name );
   strcat( tmpfname, "\"" );
   free( include_file_name );
   include_file_name = strdup( tmpfname );
  }
 modify_formbox( active_entry, include_file_name );
 gtk_widget_destroy( fs );
}


void open_include_file( GtkWidget *wdg, void *data )
{ char *filename;
  struct value_list *eb=(struct value_list *)data;
  active_entry = eb->box;
  filename = get_formbox( active_entry );
  if (filename[0] == '?') filename[0] = '\0';  /* Erase place-holder. */
  file_browser_popup( filename, "File to Include:", set_included_file );
}




/*************************************************************************/
/* Display the Tax Info - This routine constructs, lays-out and populates */
/*  the panels.  Called after initial read-in and on updates.		 */
/*************************************************************************/
void DisplayTaxInfo()
{
 struct taxline_record *txline;
 struct value_list *entry, *previous_entry=0;
 GtkWidget *label, *button, *cbutton, *menu;
 GtkRequisition req;
 GtkEntry *lastbox;
 char messg[2048];
 int linenum, iscapgains;
 int lastline, offset=0, capgtoggle, capcnt=0;
 int x1, x2, x2a, x3, x4, x5, y1, y1a, yoffset=4, y2, dy, xx, xx2, status_line; 
 int entry_box_width=0, entry_box_height=0, extra_dy;

 check_comments();
 x1 = 2;  x2 = 100;  x3 = 220;  x4 = winwidth - 50;
 y1 = 5;  y1a = y1 + yoffset;  dy = 50;
 xx2 = (x2 - x1)/2 + x1;
 // dump_taxinfo();

 /* Now place the form-data onto the pages. */
 txline = taxlines_hd;
 while (txline != 0)
  {
   offset = offset + txline->format_offset;

   /* Place the line label. */
   // printf("%d: Label %s\n", linenum, txline->linename );
   label = make_label( mpanel2, x1, y1a, txline->linename );
   gtk_widget_size_request( label, &req );	/* First find the label's size. */
   gtk_widget_destroy( label );			/* Remove it, then re-place it at best position. */
   xx = x2 - req.width - 4;
   if (xx > xx2) xx = xx2;
   if (xx < 0) xx = 0;
   label = make_label( mpanel2, xx, y1a, txline->linename );

   if (strncmp(txline->linename,"Cap-Gains",9) == 0)
    iscapgains = 1;
   else
    iscapgains = 0;
   status_line = 0;

   linenum = txline->linenum;
   entry = txline->values_hd;
   lastline = -1;  button = 0;  capgtoggle = 0;  lastbox = 0;  extra_dy = 0;
   while (entry != 0)
    { /*entry*/
     if (linenum != entry->linenum)
      {
	y1 = y1 + dy + extra_dy;
	y1a = y1 + yoffset;
	linenum = entry->linenum;
	lastbox = 0;   extra_dy = 0;
      }
     switch (entry->kind)
      {
       case VKIND_FLOAT:  // printf("Formbox: '%s'\n", messg);
		sprintf(messg, "%12.2f", entry->value ); 
		entry->box = new_formbox( mpanel2, x2, y1, 12, messg, 500, 0, 0 );
		lastbox = entry->box;
		y2 = y1 + entry_box_height - 1;
		button = make_button_wsizedcolor_text( mpanel2, x4, y2, "+", 5.0, "#000000", add_new_box_item, entry );   /* Add another box - button */
		add_tool_tip( button, "Add another entry box\nfor this line." );
		lastline = linenum;
		break;

       case VKIND_INT:  // printf("Int: %d	\n", (int)(entry->value) ); 
		break;

       case VKIND_TEXT:  // printf(">>>Text: '%s'\n", entry->text );
		if (iscapgains && (lastline == linenum))
		 { /* Cap-gains date-entry. */
		  if (button != 0) { gtk_widget_destroy( button );  button = 0; }
		  x2a = x2 + entry_box_width + 10;
		  if (capgtoggle == 0)
		   {
		    make_label( mpanel2, x2 + 20, y1 - 14, "Buy Cost" );
		    make_label( mpanel2, x2a + 10, y1 - 14, "Date Bought" );
		   }
		  else
		   {
		    make_label( mpanel2, x2 + 20, y1 - 14, "Sold For" );
		    make_label( mpanel2, x2a + 10, y1 - 14, "Date Sold" );
		   }
		  capgtoggle = !capgtoggle;
	          capcnt++;
		 }
		else
		 x2a = x2;

		entry->box = new_formbox( mpanel2, x2a, y1, 12, entry->text, 500, 0, 0 );
		previous_entry = entry;
		lastbox = entry->box;
		if (entry_box_width == 0)
		 {
		  gtk_widget_size_request( (GtkWidget *)(entry->box), &req );
		  entry_box_width = req.width;
		  entry_box_height = req.height;
		  x4 = x2a + entry_box_width - 15;	/* Set position for "+" button. */
		 }

		if (strcmp(txline->linename,"Status") == 0)
		 {
		  xx = x2 + entry_box_width;
		  menu = make_menu_button( mpanel2, xx, y1a-2, "*" );
		  add_tool_tip( most_recent_menu, "Click to select filing status\nfrom available choices." );
		  add_menu_item( menu, "Single", status_choice_S, entry );
		  add_menu_item( menu, "Married/Joint", status_choice_MJ, entry );
		  add_menu_item( menu, "Married/Sep", status_choice_MS, entry );
		  add_menu_item( menu, "Head_of_Household", status_choice_HH, entry );
		  add_menu_item( menu, "Widow(er)", status_choice_W, entry );
		  status_line = 1;
		 }
		else
		if (iscapgains)
		 {
		   if (capgtoggle == 0)
		    {
		     y2 = y1 + entry_box_height - 1;
		     x5 = x4 + entry_box_width + 10;
		     button = make_button_wsizedcolor_text( mpanel2, x5, y2, "+", 5.0, "#000000", add_new_capgain_boxes, entry );   /* Add more boxes - button */
		     add_tool_tip( button, "Add another set of entry\nboxes for another\ncap-gains entry." );
		     extra_dy = 23;
		    }
		   else
		    extra_dy = 0;
		 }
		else
		 {
		   y2 = y1 + entry_box_height - 1;
		   button = make_button_wsizedcolor_text( mpanel2, x4, y2, "+", 5.0, "#000000", add_new_box_item, entry );   /* Add another box - button */
		   add_tool_tip( button, "Add another entry box\nfor this line." );
		 }
		lastline = linenum;
		break;

       case VKIND_COMMENT: // printf("%d: Comment {%s}\n", linenum, entry->comment ); 
		if (status_line)
		 {
		  gtk_widget_size_request( most_recent_menu, &req );
		  xx = xx + req.width + 2;
		  if (xx < x3) xx = x3;
		 }
		else
		if (lastbox == 0)
		 xx = x2;	/* Otherwise empty line, so move comment left to center it. */
		else
		if (iscapgains)
	         xx = x3 + entry_box_width;	/* Cap-gains line, so move comment right to make room for date box. */
		else
	         xx = x3;	/* Normal. */
		label = make_label( mpanel2, xx, y1a, entry->comment );
		entry->comment_label = label;

		/* Add edit_line_comment button */
		if (entry_box_height != 0)
		 {
		  y2 = y1 + entry_box_height - 1;
		  x5 = winwidth - 40;
		  cbutton = make_button_wsizedcolor_text( mpanel2, x5, y2 - 4, "*", 5.0, "#000000", edit_line_comment, entry );
		  add_tool_tip( cbutton, "Edit the comment for\nthis line." );
		  if ((strstr( entry->comment, "File-name") != 0) && (previous_entry != 0))
		   {
		    cbutton = make_button_wsizedcolor_text( mpanel2, xx + 40, y2 - 4, "Browse", 7.0, "#0000ff", open_include_file, previous_entry );
		    add_tool_tip( cbutton, "Browse for tax return\noutput file to reference." );
		   }
		 }
		if (strstr( entry->comment, "\n" ))	/* Add extra line spacing for multi-line comments. */
		 { int j=0;
		   while ( entry->comment[j] != 0) { if (entry->comment[j] == '\n') y1 = y1 + 0.2 * dy;  j++; }
		 }
		break;
      }
     entry = entry->nxt;
    } /*entry*/

   y1 = y1 + dy;
   y1a = y1 + yoffset;
   txline = txline->nxt;
  }
}




void dump_taxinfo()
{
 struct taxline_record *txline;
 struct value_list *tmppt;

 printf("\n======================================\n");
 txline = taxlines_hd;
 while (txline != 0)
  {
   printf("\n%d: %s\n", txline->linenum, txline->linename );
   tmppt = txline->values_hd;
   while (tmppt != 0)
    {
     switch (tmppt->kind)
      {
       case VKIND_FLOAT:   printf("%d,F: %6.2f\n", tmppt->linenum, tmppt->value ); break;
       case VKIND_INT:     printf("%d,I: %d\n", tmppt->linenum, (int)(tmppt->value) ); break;
       case VKIND_TEXT:    printf("%d,T: %s\n", tmppt->linenum, tmppt->text ); break;
       case VKIND_COMMENT: printf("%d,C: {%s}\n", tmppt->linenum, tmppt->comment ); break;
       case VKIND_COLON:   printf("%d,s:\n", tmppt->linenum ); break;
       default:		   printf("%d,U: \n", tmppt->linenum ); break;
      }
     tmppt = tmppt->nxt;
    }
   txline = txline->nxt;
  }
 printf("\n");
}



char *my_strcasestr( char *line, char *srchstr )
{ /* Define portable version of "non-std" C-extension function that is not (yet?) available on all platforms. */
  int j=0, k;
  while (line[j] != '\0')
   {
    k = 0;
    while ((line[j+k] != '\0') && (srchstr[k] != '\0') && (toupper( line[j+k] ) == toupper( srchstr[k])))
	k++;
    if (srchstr[k] == '\0') { return &(line[j]); }
    j++;
   }
 return 0; 
}



void Save_Tax_File( GtkWidget *wdg, void *fs )
{
 struct taxline_record *txline;
 struct value_list *tmppt;
 int lastline=-1, semicolon;
 char *suffix, *tmpstr;
 FILE *outfile;

 if (current_working_filename != 0) free( current_working_filename );
 current_working_filename = strdup( gtk_file_selection_get_filename( (GtkFileSelection *)fs ) );
 gtk_widget_destroy( fs );

 /* Update the data structure(s) by getting the form fields. */
 Update_box_info();

 suffix = my_strcasestr( current_working_filename, ".txt" );
 if ((suffix == 0) || (strcasecmp( suffix, ".txt" ) != 0))
  {
   tmpstr = (char *)malloc( strlen( current_working_filename ) + 10 );
   strcpy( tmpstr, current_working_filename );
   strcat( tmpstr, ".txt" );
   current_working_filename = tmpstr;
  }

 suffix = my_strcasestr( current_working_filename, "_out.txt" );
 if ((suffix != 0) && (strcasecmp( suffix, "_out.txt" ) == 0))
  {
   warn_release = 2;
   GeneralWarning( "Your are saving an 'input-file', but the file name you picked looks like an output file." );
   return;
  }

 outfile = fopen(current_working_filename, "w");
 if (outfile==0) 
  {
   sprintf(wmsg,"ERROR: Output file '%s' could not be opened for writing.", current_working_filename );
   warn_release = 2;
   GeneralWarning( wmsg );
   return;
  }
 if (yourfilename != 0) free( yourfilename );
 yourfilename = strdup( current_working_filename );
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
 printf("\nWrote form-data to file %s\n.", yourfilename );
}



void open_taxfile( char *filename )
{
 infile = fopen(filename,"r");
 if (infile==0) 
  {
   printf("ERROR: Input file '%s' could not be opened.\n", filename);
   GeneralWarning("Error: Tax file could not be opened.");
   return;
  }
 else
  {
   Get_New_Tax_Form_Page( filename );
  }
} 



void save_taxfile( GtkWidget *wdg, void *data )
{ 
 if (verbose) printf("File-Save Dialog at: '%s'\n", directory_dat );
 file_browser_popup( yourfilename, "File to Save As:", Save_Tax_File );
}


void predict_output_filename(char *indatafile, char *outfname)
{
 int j;
 /* Base name of output file on input file. */
 strcpy( outfname, indatafile );
 j = strlen(outfname) - 1;
 while ((j >= 0) && (outfname[j] != '.')) j--;
 if (j < 0) strcat( outfname, "_out.txt" ); else strcpy( &(outfname[j]), "_out.txt" );
}



void set_tax_solver( GtkWidget *wdg, void *fs )
{
 taxsolvecmd = strdup( gtk_file_selection_get_filename( (GtkFileSelection *)fs ) );
 strcpy( taxsolvestrng, taxsolvecmd );
 gtk_widget_destroy( fs );
}


void canceltxslvr( GtkWidget *wdg, void *data )
{
 gtk_widget_destroy( resultswindow );
 resultswindow = 0;
}



int missingfile=0;


void filter_tabs( char *line )
{ /* Replace tabs and <cr> with spaces for clean printouts. */
 char *ptr;

 ptr = strchr( line, '\r' );
 if (ptr != 0)
  ptr[0] ='\0';

 ptr = strchr( line, '\n' );
 if (ptr != 0)
  ptr[0] ='\0';

 ptr = strchr( line, '\t' );
 while (ptr != 0)
  {
   ptr[0] = ' ';
   ptr = strchr( line, '\t' );
  }
}


void set_invocation_path( char *toolpath )
{
 char tmpstr[MaxFname];
 int k;
 strcpy(tmpstr, invocation_path);
 k = strlen(tmpstr)-1;
 while ((k > 0) && (tmpstr[k] != slashchr)) k--;
 if (k > 0) k--;
 while ((k > 0) && (tmpstr[k] != slashchr)) k--;
 if (tmpstr[k] == slashchr)
  tmpstr[k+1] = '\0';
 else 
#if (PLATFORM_KIND==POSIX_PLATFORM)
   {sprintf(tmpstr,".%c", slashchr);}
  if (strstr( invocation_path, "bin" ) != 0)
   sprintf( toolpath, "%sbin%c", tmpstr, slashchr);
  else
   strcpy( toolpath, "./" );   
 #else
   tmpstr[k] = '\0';
  sprintf( toolpath, "%smsbin%c", tmpstr, slashchr);
 #endif
}


void quote_file_name( char *fname )	/* Place quotes around a file name.  With special care on Microsoft systems. */
{					/* Enables proper operation when files or pathnames have spaces in them. */
 char *tmpstr;
 if ((fname[0] == '\0') || (strstr( fname, "\"" ) != 0)) return;
 tmpstr = (char *)malloc( strlen(fname) + 10 );
 #if (PLATFORM_KIND == Posix_Platform)
   strcpy( tmpstr, "\"" );
   strcat( tmpstr, fname );
 #else
   if (fname[1] == ':')
    { /* Leading quote must be inserted after drive letter for Microsoft OS's. */
     int j;
     tmpstr[0] = fname[0];
     tmpstr[1] = fname[1];
     tmpstr[2] = '"';
     j = 2;
     do { tmpstr[j+1] = fname[j];  j++; } while (tmpstr[j] != '\0');
    }
   else
    {
     strcpy( tmpstr, "\"" );
     strcat( tmpstr, fname );
    }
 #endif
 strcpy( fname, tmpstr );
 strcat( fname, "\"" );
 free(tmpstr);
}


void taxsolve()				/* "Compute" the taxes. Run_TaxSolver. */
{
 char cmd[MaxFname], outfname[MaxFname];
 GtkWidget *panel, *label;
 GtkTreeStore *mylist;
 GtkTreeIter iter;
 FILE *viewfile;
 char vline[5000];
 int wd, ht;

 if (current_working_filename == 0) 
  {
   GeneralWarning( "No tax file selected." );
   return;
  }
 if (strlen(taxsolvestrng) > 0) 
  taxsolvecmd = taxsolvestrng;
 if (taxsolvecmd == 0) 
  taxsolvecmd = getenv("taxsolvecmd");
 if (taxsolvecmd == 0)
  {
   char toolpath[MaxFname];
   set_invocation_path( toolpath );
   file_browser_popup( toolpath, "Select Tax Program to Use:", set_tax_solver );
   place_window_atmouse();	/* Temporarily change the new window position policy. */
   GeneralWarning( "No tax solver selected.  Re-try after selecting." );
   place_window_center();	/* Restore the normal window position policy. */
   return;
  }

 #if (PLATFORM_KIND == Posix_Platform)
  sprintf(cmd,"'%s' '%s' &", taxsolvecmd, current_working_filename );
 #else
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
  sprintf(cmd,"%s \"%s\"", taxsolvecmd, current_working_filename );
 #endif

 printf("Invoking '%s'\n", cmd );
 system(cmd);		/* Run the TaxSolver. */

 /* Make a popup window telling where the results are, and showing them. */
 predict_output_filename( current_working_filename, outfname );
 wd = 620;  ht = 550;
 panel = new_window( wd, ht, "Results", &resultswindow );
 make_sized_label( panel, 1, 1, "Results written to file:", 12 );
 label = make_sized_label( panel, 30, 25, outfname, 8 );
 set_widget_color( label, "#0000ff" );
 make_button( panel, wd/2 - 15, ht - 35, "  OK  ", canceltxslvr, 0 ); 
 show_wind( resultswindow );
 UpdateCheck();
 Sleep_seconds( 0.25 );
 UpdateCheck();
 Sleep_seconds( 0.25 );
 mylist = new_selection_list( panel, 5, 50, wd - 10, ht - 50 - 50, "Results Preview:", 0, 0, 0 );
 viewfile = fopen( outfname, "rb" );
 if (viewfile == 0)
  {
   sprintf(vline,"Cannot open: %s", outfname);
   printf("%s\n", vline );
   append_selection_list( mylist, &iter, vline );
  }
 else
  {
   fgets( vline, 256, viewfile );
   while (!feof(viewfile))
    {
     filter_tabs( vline );
     append_selection_list( mylist, &iter, vline );
     fgets( vline, 256, viewfile );
    }
  }
 show_wind( resultswindow );
 computed = 1;
}



void Run_TaxSolver( GtkWidget *wdg, void *x )
{
 Update_box_info();
 if (save_needed)
  {
   GeneralWarning( "Change(s) not saved.  You must save before computing." );
   return;
  }
 taxsolve( 0, 0 );
}





GtkWidget *printpopup=0, *print_label, *print_button;
GtkEntry *printerformbox;
char printer_command[MaxFname], wrkingfname[MaxFname];
int printdialogsetup;
int print_mode=0;
#if (PLATFORM_KIND==POSIX_PLATFORM)
 char base_printer_command[]="lpr ";
#else
 char base_printer_command[]="print ";
#endif
void acceptprinter_command( GtkWidget *wdg, void *data );



void cancelprintpopup( GtkWidget *wdg, void *data )
{ gtk_widget_destroy( printpopup );  printpopup = 0; }


void togprntcmd_in( GtkWidget *wdg, void *data )
{ 
 if (!printdialogsetup) return;
 sprintf(printer_command,"%s \"%s\"", base_printer_command, current_working_filename );
 modify_formbox( printerformbox, printer_command );
 if (print_mode == 2)
  {
   modify_label( print_label, "Print Command" );
   gtk_button_set_label( GTK_BUTTON( print_button ), "  Print It  " );
  }
 print_mode = 0;
}


void togprntcmd_out( GtkWidget *wdg, void *data )
{
 if (!printdialogsetup) return;
 predict_output_filename( current_working_filename, wrkingfname );
 sprintf(printer_command,"%s \"%s\"", base_printer_command, wrkingfname );
 modify_formbox( printerformbox, printer_command );
 if (print_mode == 2) 
  {
   modify_label( print_label, "Print Command" );
   gtk_button_set_label( GTK_BUTTON( print_button ), "  Print It  " );
  }
 print_mode = 1;
}


void acceptprinter_command( GtkWidget *wdg, void *data )
{
   get_formbox_text( printerformbox, printer_command, MaxFname );
   printf("Issuing: %s\n", printer_command);
   system( printer_command );
   cancelprintpopup( 0, 0 );
}





void printout( GtkWidget *wdg, void *data )
{
 GtkWidget *rad, *panel; 
 int x1=70, x2, y1=30, dy=20, wd=750, ht=190, printht;

 Update_box_info();
 if (save_needed)
  {
   if (!computed)
    GeneralWarning( "Change(s) not saved.  You should Save input file before printing it." );
   else
    GeneralWarning( "Change(s) not saved.  You should Save + Re-Compute before printing." );
   return;
  }
 print_mode = 0;
 panel = new_window( wd, ht, "Print Return Data", &printpopup );
 make_sized_label( panel, 1, 1, "Print Return Data:", 12 );
 rad = make_radio_button( panel, 0, x1, y1, "Print Input Data", togprntcmd_in, 0 );

 x2 = x1 + 160;
 y1 = y1 + dy;
 rad = make_radio_button( panel, rad, x1, y1, "Print Output Data", togprntcmd_out, 0 );
 make_rectangular_separator( panel, x1 - 10, y1 - dy - 10, x2, y1 + dy );
 y1 = y1 + dy + 15;
 print_label = make_label( panel, 10, y1 + 4, "Print Command:" );
 y1 = y1 + dy;
 printdialogsetup = 0;
 strcpy( printer_command, base_printer_command );
 strcat( printer_command, " " );
 if (computed)
  {
   set_radio_button( rad );
   predict_output_filename( current_working_filename, wrkingfname );
   strcat( printer_command, wrkingfname );
   print_mode = 1;
  }
 else
  strcat( printer_command, current_working_filename );
 x1 = 20;
 printerformbox = new_formbox_bypix( panel, x1, y1, wd - x1 - 10, printer_command, MaxFname, acceptprinter_command, 0 ); 
 printht = ht - 35;
 print_button = make_button( panel, 50, printht, "  Print It  ", acceptprinter_command, 0 );
 make_button( panel, wd - 95, printht, "Cancel", cancelprintpopup, 0 );
 show_wind( printpopup );
 printdialogsetup = 1;
}







char program_names[30][100] = 
	{
	 "taxsolve_US_1040_2014",		/* 0 */
	 "taxsolve_US_1040_Sched_C_2014",	/* 1 */
	 "taxsolve_US_8829",			/* 2 */
	 "taxsolve_CA_540_2014",		/* 3 */
	 "taxsolve_NC_D400_2014",		/* 4 */
	 "taxsolve_NJ_1040_2014",		/* 5 */
	 "taxsolve_OH_IT1040_2014",		/* 6 */
	 "taxsolve_PA_40_2014",			/* 7 */
	 "taxsolve_VA_760_2014",		/* 8 */
	 "taxsolve_NY_IT201_2014",		/* 9 */
	 "taxsolve_MA_1_2014",			/* 10 */
	 "Other",				/* 11 */
	};



void slcttxprog( GtkWidget *wdg, void *data )
{
 char *sel=(char *)data;
 char strg[MaxFname], tmpstr[MaxFname], toolpath[MaxFname];
 int k, prev;

 if (!ok_slcttxprog) return;
 prev = selected_form;
 if (sscanf(sel,"%d", &selected_form) != 1) printf("Internal error '%s'\n", sel );
 if (selected_form == prev) return;
 strcpy( strg, program_names[selected_form] );
 if (strcmp(strg,"Other")==0)
  {
   selected_other = 1;
   if (verbose) printf("invocation_path = '%s'\n", invocation_path );
   set_invocation_path( toolpath );
   file_browser_popup( toolpath, "Select Tax Program to Use:", set_tax_solver );

   strcpy(tmpstr, invocation_path);
   k = strlen(tmpstr)-1;
   while ((k > 0) && (tmpstr[k] != slashchr)) k--;
   if (k > 0) k--;
   while ((k > 0) && (tmpstr[k] != slashchr)) k--;
   if (tmpstr[k] == slashchr)  tmpstr[k+1] = '\0';
   else  {sprintf(tmpstr,".%c", slashchr);}
   sprintf(directory_dat, "%sexamples_and_templates%c", tmpstr, slashchr);

   return;
  }
 else
  {
   selected_other = 0;
   sprintf(tmpstr,"%s%s", invocation_path, strg);
   printf("Setting Tax Program to be: '%s'\n", tmpstr);
   taxsolvecmd = strdup(tmpstr);
   strcpy(taxsolvestrng, tmpstr);

   strcpy(tmpstr, invocation_path);
   k = strlen(tmpstr)-1;
   while ((k > 0) && (tmpstr[k] != slashchr)) k--;
   if (k > 0) k--;
   while ((k > 0) && (tmpstr[k] != slashchr)) k--;
   if (tmpstr[k] == slashchr)  tmpstr[k+1] = '\0';
   else  {sprintf(tmpstr,".%c", slashchr);}
   sprintf(directory_dat, "%sexamples_and_templates%c", tmpstr, slashchr);

   sel = strstr( strg, "_2014" );
   if (sel != 0)
     sel[0] = '\0';
   strcpy( tmpstr, strg );
   if (strlen(tmpstr) < 10) {printf("Internal error, name too short '%s'\n", tmpstr); return;}
   strcpy( strg, &(tmpstr[9]) );	/* Remove the prefix "taxsolve_". */
   strcat( directory_dat, strg );
   printf("Setting Tax Data Directory to be: '%s'\n", directory_dat);
  }
}



#include "logo_image.c"   /* <--- The image-data is included here, produced by "data2code". */


void quit( GtkWidget *wdg, void *x )
{
 printf("OTS Exiting.\n");
 exit(0);
}


void warn_about_save_needed()
{
 int xpos=20, ypos=20, winwdth, winhght=100;
 GtkWidget *winframe, *label;
 winwdth = 300;
 winframe = new_window( winwdth, winhght, "Warning Message", &warnwin );
 label = make_sized_label( winframe, xpos, ypos, "<b>Change(s) not saved !!</b>", 12 );
 set_widget_color( label, "#ff0000" );
 make_button( winframe, 10, winhght - 40, "Exit anyway, without saving", quit, &warnwin );
 make_button( winframe, winwdth - 60, winhght - 40, "Cancel", dismiss_general_warning, &warnwin );
 gtk_window_set_keep_above( (GtkWindow *)warnwin, 1 );
 show_wind( warnwin );
}


void quit_wcheck( GtkWidget *wdg, void *x )
{
 Update_box_info();
 if (save_needed)
  {
   warn_about_save_needed();
   return;
  }
 quit( 0, 0 );
}



static gboolean on_expose_event( GtkWidget *widget, GdkEventExpose *event, gpointer data )
{
 unsigned char *logodata;
 int imgwd, imght, x1, y1, x2, y2, new_width, new_height;

 if (operating_mode == 1)
  { /*mode1*/
   logodata = cdti_convert_data_to_image( data_0, data_0_size, &imgwd, &imght );
   x1 = (winwidth - imgwd) / 2;
   y1 = 10;
   x2 = x1 + imgwd;
   y2 = y1 + imght;
   place_image( mpanel, imgwd, imght, x1, y1, logodata );
   free( logodata );
   // make_rectangular_separator( mpanel, x1 - 2, y1 - 2, x2 - 2, y2 + 2 );
   { /* Place a border around the logo image. */
    cairo_t *canvas=0;
    canvas = gdk_cairo_create( mpanel->window );
    cairo_set_line_width( canvas, 2.0 );
    cairo_set_source_rgb( canvas, 0.1, 0.1, 0.2 );
    x1 = x1 - 2;  y1 = y1 - 2;
    x2 = x2 + 2;  y2 = y2 + 2;
    cairo_move_to( canvas, x1, y1 );
    cairo_line_to( canvas, x2, y1 );
    cairo_line_to( canvas, x2, y2 );
    cairo_line_to( canvas, x1, y2 );
    cairo_line_to( canvas, x1, y1 );
    cairo_stroke( canvas );
    x1 = 20;   	   	x2 = winwidth - 20;
    y1 = fronty1 - 5;	y2 = fronty2;
    cairo_set_line_width( canvas, 1.0 );
    cairo_set_source_rgb( canvas, 0.1, 0.1, 0.5 );
    cairo_move_to( canvas, x1, y1 );
    cairo_line_to( canvas, x2, y1 );
    cairo_line_to( canvas, x2, y2 );
    cairo_line_to( canvas, x1, y2 );
    cairo_line_to( canvas, x1, y1 );
    cairo_stroke( canvas );
    cairo_destroy(canvas);
   }
  } /*mode1*/
 else
  { /*mode2*/
   gtk_window_get_size( GTK_WINDOW( outer_window ), &new_width, &new_height );
   if ((new_width != winwidth) || (new_height != winht))
    {
     winwidth = new_width;
     winht = new_height;
     need_to_resize = 1;
     last_resize_time = Report_Time();
    }
  } /*mode2*/
 return FALSE;
}



int myfilterfunc( char *word )
{
 if (my_strcasestr( word, ".txt") != 0)
  {
   if (my_strcasestr( word, "_out.txt" ) != 0) return 0;
   if (my_strcasestr( word, "README_" ) != 0) return 0;
   return 1; 
  }
 else return 0;
}


//gboolean filterfunc( const GtkFileFilterInfo *finfo, gpointer data )
//{
 // printf("Got '%s'\n", finfo->filename );
// return myfilterfunc( finfo->filename );
//}


void receive_filename( GtkWidget *wdg, void *fs )
{
 if (yourfilename != 0) free( yourfilename );
 yourfilename = strdup( gtk_file_selection_get_filename( (GtkFileSelection *)fs ) );
 printf("You picked '%s'\n", yourfilename );
 gtk_widget_destroy( fs );
 if (myfilterfunc( yourfilename ) == 0)
  {
   warn_release = 1;
   GeneralWarning("File name does not look like a tax input file.");
  }
 else
  {
   open_taxfile( yourfilename );
  }
}


void pick_file( GtkWidget *wdg, void *data )
{ int j;
  GtkFileFilter *rule;

printf("Selected_Form = %d\n", selected_form );
  rule = gtk_file_filter_new();
  gtk_file_filter_add_pattern( rule, "*.txt" );
  // gtk_file_filter_add_custom( rule, 15, filterfunc, 0, 0 );

  j = strlen( directory_dat );
  if ((j > 0) && (directory_dat[j-1] != slashchr))
   strcat( directory_dat, slashstr );
  if (verbose) printf("BrowsePath = '%s'\n", directory_dat );
  file_browser_popup( directory_dat, "Select File", receive_filename );
}


void set_ots_path()
{ /* Expect invocation path to end with "bin" or "msbin", and remove that part. */
 int j;
 ots_path = strdup( invocation_path );
 j = strlen( ots_path ) - 1;
 while ((j >= 0) && (strstr( &(ots_path[j]), "bin" ) != &(ots_path[j]))) j--;
 if (j < 0) { ots_path = strdup( "../" );  ots_path[2] = slashchr; }
 else
 if ((j > 1) && (ots_path[j-1] == 's')) ots_path[j-2] = '\0';
 else ots_path[j] = '\0';
}



/*----------------------------------------------------------------------------*/
/* Main -								      */
/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[] )
{
 int argn, k;
 char vrsnmssg[50], tmpstr[MaxFname];
 float x, y, dy, y1, y2;
 GtkWidget *txprogstog, *button;

 sprintf(vrsnmssg,"OTS GUI v%1.2f", version);  printf("%s\n\n",vrsnmssg);
 invocation_path = strdup(argv[0]);
 k = strlen(invocation_path)-1;
 while ((k>0) && (invocation_path[k]!=slashchr)) k--;
 if (invocation_path[k]==slashchr) k++;
 invocation_path[k] = '\0';
 // printf("Invocation path = '%s'\n", invocation_path);
 set_ots_path();

 /* Decode any command-line arguments. */
 argn = 1;  k=1;
 while (argn < argc)
 {
  if (strcmp(argv[argn],"-verbose")==0)
   { 
    verbose = 1;
   }
  else
  if (strcmp(argv[argn],"-help")==0)
   { 
    printf("OTS GUI v%1.2f:\n", version);
    printf(" Command-line Options:\n");
    printf("  -verbose          - Show status messages.\n");
    printf("  -taxsolver xx     - Set path and name of the tax-solver executable.\n");
    printf("  {file-name}.txt   - Set path and name of the tax data input file.\n\n");
    exit(0);
   }
  else
  if (strcmp(argv[argn],"-taxsolver")==0)
   { int kx;
    argn++;
    if (argn == argc) { printf("Missing entry after '-taxsolver'.\n");  exit(1); }
    taxsolvecmd = strdup( argv[argn] );
    strcpy( taxsolvestrng, taxsolvecmd );
    selected_other = 1;
    if (verbose) printf("invocation_path = '%s'\n", invocation_path );

    strcpy(tmpstr, invocation_path);
    kx = strlen(tmpstr)-1;
    while ((kx > 0) && (tmpstr[kx] != slashchr)) kx--;
    if (kx > 0) kx--;
    while ((kx > 0) && (tmpstr[kx] != slashchr)) kx--;
    if (tmpstr[kx] == slashchr)  tmpstr[kx+1] = '\0';
    else  {sprintf(tmpstr,".%c", slashchr);}
    sprintf(directory_dat, "%sexamples_and_templates%c", tmpstr, slashchr);
    selected_form = 11;
    ok_slcttxprog = 0;
   }
  else
  if (k==1)
   {
    current_working_filename = strdup(argv[argn]);
    infile = fopen(current_working_filename,"r");
    if (infile==0) {printf("ERROR: Parameter file '%s' could not be opened.\n", argv[argn]); exit(1);}
    k = 2;
    ok_slcttxprog = 0;
   }
  else
   {
    printf("Unknown command-line parameter '%s'\n", argv[argn]); 
    /* exit(1); */ 
   }
  argn = argn + 1;
 }

 mpanel = init_top_outer_window( &argc, &argv, winwidth, winht, "OpenTaxSolver-GUI", 0, 0 );
 // make_sized_label( mpanel, 180, 10, "Open-Tax-Solver", 20.0 );

 gtk_widget_set_app_paintable( outer_window, TRUE );
 g_signal_connect( outer_window, "expose-event", G_CALLBACK(on_expose_event), NULL);

 make_rectangular_separator( mpanel, 59, 6, 387, 102 );

 y = 105;
 make_sized_label( mpanel, winwidth / 2 - 20, y, "2014", 11.0 );
 y = y + 35;
 make_sized_label( mpanel, 10, 135, "Select Tax Program:", 12.0 );

 x = 30;
 y = y + 25;
 y1 = y;
 dy = ((winht - 120) - y) / 6;
 txprogstog = make_radio_button( mpanel, 0, x, y, "US 1040 (w/Scheds A,B,D)", slcttxprog, "0" );
 add_tool_tip( txprogstog, "Also does the 8949 forms." );
 y = y + dy;
 make_radio_button( mpanel, txprogstog, x, y, "US 1040 Sched C", slcttxprog, "1" );
 y = y + dy;
 make_radio_button( mpanel, txprogstog, x, y, "CA State 540", slcttxprog, "3" );
 y = y + dy;
 make_radio_button( mpanel, txprogstog, x, y, "NC State DC400", slcttxprog, "4" );
 y = y + dy;
 make_radio_button( mpanel, txprogstog, x, y, "NJ State 1040", slcttxprog, "5" );

 y = y1;
 x = winwidth/2 + 40;
 make_radio_button( mpanel, txprogstog, x, y, "OH State IT1040", slcttxprog, "6" );
 y = y + dy;
 make_radio_button( mpanel, txprogstog, x, y, "PA State 40", slcttxprog, "7" );
 y = y + dy;
 make_radio_button( mpanel, txprogstog, x, y, "VA State 760", slcttxprog, "8" );
 y = y + dy;
 make_radio_button( mpanel, txprogstog, x, y, "NY State IT201", slcttxprog, "9" );
 y = y + dy;
 make_radio_button( mpanel, txprogstog, x, y, "MA State 1", slcttxprog, "10" );
 y = y + dy;
 txprogstog = make_radio_button( mpanel, txprogstog, x, y, "Other", slcttxprog, "11" );
 y2 = y + dy;

 if (selected_other) set_radio_button( txprogstog );

 fronty1 = y1;
 fronty2 = y2;
 // make_rectangular_separator( mpanel, 20, y1-5, winwidth - 20, y2 );

 slcttxprog( 0, "0" );	/* Set default tax program. */

 if (infile == 0) 
  {
   button = make_button_wsizedcolor_text( mpanel, 80, winht - 95, "Open a Tax File (or Template)", 14.0, "#000000", pick_file, 0 );
   add_tool_tip( button, "Find tax data files matching the selected tax form." );
   make_label( mpanel, winwidth / 2 - 50, winht - 30, vrsnmssg  );
  }
 else
   Get_New_Tax_Form_Page( current_working_filename );

 button = make_button( mpanel, winwidth - 60, winht - 35, " Quit ", quit, 0 );
 add_tool_tip( button, "Leave this program." );

 ok_slcttxprog = 1;
 gtk_widget_show_all( outer_window );
 while (1)	 // gtk_main();
  {
   UpdateCheck();       	/* Check for, and serve, any pending GTK window/interaction events. */
   Sleep_seconds( 0.05 );       /* No need to spin faster than ~20 Hz update rate. */
   if ((need_to_resize) && (Report_Time() - last_resize_time > 0.35))
    {
	if (verbose) printf("	Resizing to (%d, %d)\n", winwidth, winht );
	Update_box_info();
	Setup_Tax_Form_Page();
	need_to_resize = 0;
    }
  }
 return 0;
}
