#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>


/* ------------- Text-Label Routines ----------------- */
GtkWidget *make_label( GtkWidget *panel, int xpos, int ypos, char *text );

void modify_label( GtkWidget *label, char *newtext );
void set_widget_color( GtkWidget *label, char *color_value );    /* Specify colors as: "#rrggbb" in hex 00-ff. */
GtkWidget *make_sized_label( GtkWidget *panel, int xpos, int ypos, char *text, float fontsize );  /* Default = 10.0. */


/* ------------- Button Routines ----------------- */
GtkWidget *make_button( GtkWidget *panel, int xpos, int ypos, char *label, void callback(GtkWidget *, void *), void *data );

GtkWidget *make_button_wicon( GtkWidget *panel, int xpos, int ypos, char **icon, void callback(GtkWidget *, void *), void *data );
	/* Be sure to somewhere set:  g_object_set( gtk_settings_get_default(), "gtk-button-images", TRUE, NULL); */
	/* To use, "#include" an xpm image file of icon.  Make variable-name at top of XPM file match "icon" name */
	/*  called in this function.  (You call it with that variable name.) */


/* ------------- Radio Button Routines ----------------- */
GtkWidget *make_radio_button( GtkWidget *panel, GtkWidget *group, int xpos, int ypos, char *label, 
			      void callback(GtkWidget *, void *), void *data );

void set_radio_button( GtkWidget *radio_button );


/* ------------- Toggle Button Routines ----------------- */
GtkWidget *make_toggle_button( GtkWidget *panel, int xpos, int ypos, char *label, int state, void callback(GtkWidget *, void *), void *data );

void set_toggle_button( GtkWidget *tog_button, int state );


/* ------------- Form-Box (Text-Entry Box) Routines ----------------- */
GtkEntry *new_formbox( GtkWidget *panel, int xpos, int ypos, int nchars_wide, char *text, int maxlen, 
				 void callback(GtkWidget *, void *), void *data );

void modify_formbox( GtkEntry *formbox, char *text );

char *get_formbox( GtkEntry *formbox );		/* Returns string poiner, which must NOT be freed. */

void get_formbox_text( GtkEntry *formbox, char *rtrnstrng, int maxlen );

	/*--- Example Usage:
	 GtkEntry *formbox1;

	 SetFormBox_Routine()
	  {
	   int nchars;
	   nchars = 0.109 * frmbx_wdth_pix;	// Good rule of thumb.
	   formbox1 = new_formbox( panel, ypos, xpos, nchars, "0", 1024, action_routine, 0 );
	  }

	 Retreival_routine()
	  {
	   char *word;
	   word = get_formbox( formbox1 );
	   printf("Text = '%s'\n", word );
	  }
         ---*/

/* ------------- GTK Text-Edit Box Routines ----------------- */		/* Multi-line text-edit widget. */
GtkTextView *new_text_edit_box( GtkWidget *panel, int xpos, int ypos, int width, int height, char *text );

void modify_text_edit_box( GtkTextView *textview, char *text );

void get_text_edit_box( GtkTextView *textview, char *rtrnstrng, int maxlen );

	/*--- Example Usage:
	 GtkTextView *txtedt;

	 MakeTextEditWindow_Routine()
	  {
	   txtedt = new_text_edit_box( panel, 2, 2, 200, 100, "Initial text\n is here." );
	  }

	 Retreival_routine()
	  {
	   char word[1000];
	   get_text_edit_box( txtedt, word, 1000 );
	   printf("Text = '%s'\n", word );
	  }
	---*/



/* ------------- Menu Routines ----------------- */

GtkWidget *make_menu( GtkWidget *tool_bar, char *label );	/* For use on automatically spaced menu-bars. */

GtkWidget *make_menu_button( GtkWidget *panel, int xpos, int ypos, char *label );                /* For use anywhere on a window. */

GtkWidget *add_menu_item( GtkWidget *menu, char *label, void callback(GtkWidget *, void *), void *data );

GtkWidget *add_submenu( GtkWidget *menu, char *label );

void modify_menu_label( GtkWidget *menu_label, char *newtext );	 /* A 'menu_label' is what is returned by add_menu_item(). */



/* ------------- GTK Selection List Routines ----------------- */

GtkTreeStore *new_selection_list( GtkWidget *panel, int xpos, int ypos, int width, int height, char *column_titles, 
				  void callback(GtkWidget *, void *), void dclick_callback(GtkWidget *, void *), void *data );

void append_selection_list( GtkTreeStore *list, GtkTreeIter *iter, char *item );

char *get_selection_from_list( GtkWidget *selection );


	/* Example Usage:
		{
		 GtkTreeStore *mylist;
		 GtkTreeIter iter;
		 mylist = new_selection_list( panel, 10, 20, 80, 100, myfunction, 0, 0 );
		 for (j=0; j<10; j++)
		  append_selection_list( mylist, &iter, text[j] );
		}
	
	       void myfunction( GtkWidget *wdg, void *data )
		{
		 char *text;
		 text = get_selection_from_list( wdg );
		 printf("Selected: '%s'\n", text );
		}
	*/


/* ------------- GTK FormBoxWithSuggestor ------------------- */

GtkSpinButton *make_FormBoxWithSuggestor( GtkWidget *panel, int xpos, int ypos, float min, float initval, float max, float step, int ndigits );

float get_FormBoxWithSuggestor_Value( GtkSpinButton *spinbox );

void set_FormBoxWithSuggestor_value( GtkSpinButton *spinbox, float value );


/* ------------- Separator Routines ----------------- */

GtkWidget *make_vseparator( GtkWidget *panel, int xpos, int y1, int y2 );
GtkWidget *make_hseparator( GtkWidget *panel, int x1, int x2, int ypos );
GtkWidget *make_rectangular_separator( GtkWidget *panel, int x1, int y1, int x2, int y2 );



/* ------------- GTK Slider Routines ----------------- */

GtkWidget *make_slider( GtkWidget *panel, int xpos, int ypos, int size, char orien, double min, double initval, double max,
			void callback(GtkWidget *, void *), void *data );

	/*--- Example Usage:
	void slide_adj( GtkWidget *widget, gpointer data )
	{
	 double x;
	 x = gtk_range_get_value( GTK_RANGE(widget) );
	 printf("Slider Adjusted '%g'\n", x );
	}

	...
	make_slider( panel, xpos, ypos, size, 'h', minv, initval, maxval, slide_adj, 0 );
	---*/


/* ------------- Sub-Window or (Detachable) Pop-up Window Making Routines ----------------- */

GtkWidget *new_window( int width, int height, char *title, GtkWidget *(*winptr) );

        /* Create new window and register user's own killed callback.  It must return 0. */
GtkWidget *new_window_wkill( int width, int height, char *title, GtkWidget *(*winptr), int callback(GtkWidget *, void *) );

        /* Like above, but set horz/vert-scroll to 1 or 0, to enable or disable respective scroll. */
GtkWidget *new_scrolled_window_wkill( int width, int height, char *title, GtkWidget *(*winptr), 
                                      int horzscroll, int vertscroll, int callback(GtkWidget *, void *) );

	/* Initial call to set up outermost window. */
GtkWidget *init_top_outer_window( int *argc, char ***argv, int winwidth, int winhght, char *title,
                                  int horzscroll, int vertscroll );

void close_any_window( GtkWidget *widget, gpointer data );

void place_window_center();
void place_window_atmouse();
void show_wind( GtkWidget *wind );


     /****	---- Example window making function. ----
	GtkWidget *win1=0;

	void respond_to_some_event( GtkWidget *widget, gpointer data )
	{
	 GtkWidget *panel;

	 panel = new_window( 400, 150, "Test Win", &win1 );

	 ---- Place all window items here .... ----
	 make_button( panel, 20, 20, " OK ", close_any_window, &win1 );

	 show_wind( win1 );
	}

    ****/




/* ------------- GTK Image Routines ----------------- */

/*------------------------------------------------------------------------------*/
/* place_image - This routine displays an image on the named panel, at the 	*/
/*	specified (x,y) position.  Image pixels:  image_width x image_height	*/
/*	Allocate and initialize image as:					*/
/*		imagedata = (unsigned char *)malloc( 4 * image_pixels );	*/
/*		imagedata[ 4 * (image_width * row + col) ]     = Blue;		*/
/*		imagedata[ 4 * (image_width * row + col) + 1 ] = Green;		*/
/*		imagedata[ 4 * (image_width * row + col) + 2 ] = Red;		*/
/*	Where values are 0=dark to 255=bright. 					*/
/*	You can de-allocate or re-use image array after calling this function.	*/
/*------------------------------------------------------------------------------*/
void place_image( GtkWidget *window, int image_width, int image_height, int pos_x, int pos_y, unsigned char *imagedata );

/* Convert Data to Image (cdti) - Retreives image-data from pre-initialized arrays stored within program. */
/*     (The pre-stored array text can be created from the "data2code.c" utility.)   */
unsigned char *cdti_convert_data_to_image( char *data, int dimension, int *imgwd, int *imght );



/* ------------- Tool Tips / Hover Notes ----------------- */
GtkTooltips *add_tool_tip( GtkWidget *wdg, char *text );         /* Adds tool-tip to button, etc. */


/* ------------- File Browser ----------------- */
GtkWidget *file_browser_popup( char *dir, char *text, void callback(GtkWidget *, void *) );

	/* Example usage: 
	   void receive_filename( GtkWidget *wdg, void *fs )
	    {
	     char *yourfilename=0;
	     yourfilename = strdup( gtk_file_selection_get_filename( (GtkFileSelection *)fs ) );
	     gtk_widget_destroy( fs );		-- Closes file-browser window. --
	     printf("Absolute path = '%s'\n", yourfilename );
	     free( yourfilename );
	    }

	   file_browser_popup( ".", "Select File", receive_filename );
	*/


/* ------------- General Stuff ----------------- */

void UpdateCheck();   /* Process any pending window events. */

void strcpy_safe( char *dst, const char *src, int maxlen );

double Report_Time();	/* Reports time in seconds, accurate to millisecs, for checking time differences. */

void Sleep_seconds( float dt_seconds );

/* gtk_widget_destroy( wdg );	*/
