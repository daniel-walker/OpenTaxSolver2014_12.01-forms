/****************************************************************************************/
/* Otk_lib.c - A general purpose GUI environment based on OpenGL and GLUT or X-Windows.	*/
/*  (GUI = Graphical User Interface.)							*/
/*											*/
/* For Documentation and Usage Notes, see:    http://otk.sourceforge.net/		*/
/*											*/
/* To Compile on Unix/Linux:								*/
/*   cc -c -I/usr/X11R6/include otk_lib.c -o otk.o					*/
/* Link with:    -lGLU -lGL -lXmu -lXext -lX11						*/
/*                                                                              	*/
/* To Compile on Microsoft using MinGW compiler:					*/
/*   gcc -c otk_lib.c -o otk.o								*/
/* Link with:    -lglu32 -lopengl32 -lwinmm -lgdi32 					*/
/*                                                                              	*/
/* Copyright (C) 2002-2009: Carl Kindman						*/
/*                                                                              	*/
/* Lessor GNU Public License - LGPL:							*/
/* This program is free software; you can redistribute it and/or			*/
/* modify it under the terms of the GNU General Public License as			*/
/* published by the Free Software Foundation; either version 2 of the			*/
/* License, or (at your option) any later version.					*/
/*											*/
/* This program is distributed in the hope that it will be useful,			*/
/* but WITHOUT ANY WARRANTY; without even the implied warranty of			*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU			*/
/* General Public License for more details.						*/
/*											*/
/* You should have received a copy of the Lessor GNU General Public License		*/
/* along with this program; if not, write to the Free Software				*/
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA				*/
/* 02111-1307 USA									*/
/*											*/
/* Carl Kindman 8-1-2006     carlkindman@yahoo.com					*/
/* Carl Kindman 9-7-2007     Updated file browser.					*/
/* Carl Kindman 4-21-2008    Updated image handling.					*/
/* Carl Kindman 6-21-2008    Added automatic test capture+playback.			*/
/* Carl Kindman 8-10-2008    Added hover-boxes.						*/
/* Carl Kindman 9-2-2008 v0.74  Enhanced MS-Win timer.  Enabled slider-slot clicking.	*/
/* Carl Kindman 4-22-2009 v0.84  Activated text-selection highlighting in form-boxes.	*/
/* Carl Kindman 1-1-2012 v0.95  Added Otk_Set_Timer.					*/
/* (See release notes for complete revision history.)					*/
/****************************************************************************************/

#define Otk_version 0.96
#define Otk_MAJOR_VERSION 0
#define Otk_MINOR_VERSION 96
#define Otk_MINOR_REVNUM 0

/* Set debug switch. 1=verbose.  0=quiet. */
#define DEBUG if (0)

#include "otk_lib.h"

// #include "scz_compress/scz.h"              /* Optionally, include self-contained decompression library. */
// #include "scz_compress/scz_compress_lib.c"
// #include "scz_compress/scz_decompress_lib.c"
// #include "scz_compress/scz_streams.c"

int Otk_verbose=0;
int OtkWindowSizeX=-1, OtkWindowSizeY=-1;
float Otk_AspectAngle=1.0;
int Otk_showkey=0;
int Otk_MouseButtonState[9]={0,0,0,0,0,0,0,0,0};
int Otk_Clicked_MouseButton=0;
int Otk_MousePixX=-1, Otk_MousePixY=-1, 	/* Current mouse position in pixels, on click or drag. */
    Otk_MouseLastX, Otk_MouseLastY;		/* Last mouse position while dragging. */
float Otk_MouseX=-1.0, Otk_MouseY=-1.0;		/* Current mouse position in % window coords, on click or during drag. */
float Otk_ClickedX=-1.0;			/* Position clicked on down-click or start of drag, in % window coords. */
float Otk_ClickedY=-1.0;

float Otk_DZ=2.0;				/* Layer Z spacing.  Nominally=1.0. */
int otk_native_window=1;
int Otk_Display_Changed = 4, Otk_nondraws=0, Otk_Always_Update=0;
float Otk_sqrtaspect=1.0, otk_render_quality_hint=1.0;
char *otk_window_name="Otk";
FILE *otk_test_file=0;
int otk_test_playback=0, otk_test_capnum=0;
int otk_highest_qual_tried=0, otk_render_qual_level=0;	/* Quality levels span 0 to 4. */
int allow_cpu_antialiasing=1;
int otk_got_accumbuf=1;
double otk_last_redraw_time, OTK_FRAME_PERIOD=0.0;

#if (PLATFORM_KIND==MsVisC_Platform)
 #ifndef calloc
  #define calloc(n,s) (memset(malloc(n*s),0,(n*s)))
 #endif
 #ifndef strdup
  #define strdup(s) (strcpy(malloc(strlen(s)+1), s))
 #endif

  int strcasecmp( char *str1, char *str2 )
  {
   int j=-1, cmp;
   do 
    {
      j++;
      cmp = tolower( str1[j] ) - tolower( str2[j] );
    }
   while ((cmp == 0) && (str1[j] != '\0') && (str2[j] != '\0'));
   return cmp;
  }
#endif


#include "letter2vector2.c"


 struct OtkObjectInstance *OtkRootObject=0, *OtkOuterWindow=0, *Otk_keyboard_focus=0, *Otk_OpenMenu=0;

 /* Modal parameters. */
 float Otk_border_thickness = 1.0;

 struct Otkkeyboardstatus
  {
   int shiftkey;
   int ctrlkey;
   int keypad_numlock;
   int textform_insertion_column, mouse_select_start_column, mouse_select_end_column, mouse_select_old_column;
   int textform_insertion_row;
   int cursor_blink_count;
  } Otk_Keyboard_state;

 struct 
  {
    float x, y, z;
    float pointx, pointy, pointz;
    int moved;
    float fov, minZ, maxZ;
  } OtkCameraPosition;
 
 OtkWidget Otk_ClickedObject=0, otk_highlighted_text=0, otk_last_scrollable=0;
 struct Otk_image *Otk_image_list=0;
 int OtkTextureNumber=0, otk_current_mouse_cursor_style=0;

/*=============================================v= Otk =v=============================================*/

/* Otk variables. */

void OtkSetBorderThickness( float thickness )	/* Deprecated in favor of Otk_Set_Default_Border_Thickness(...) */
{ Otk_border_thickness = thickness; }

void Otk_Set_Default_Border_Thickness( float thickness )	/* Replaces OtkSetBorderThickness(...) */
{ Otk_border_thickness = thickness; }


void Otk_Set_Object_Border_Thickness( OtkWidget obj, float thickness )
{ obj->thickness = thickness; }


float Otk_BLACK[4] = {0.0, 0.0, 0.0, 1.0};
OtkColor Otk_Default_Button_Color;
int Otk_Default_Button_Outline_Style=1;

OtkColor OtkSetColor( float r, float g, float b )
{
 OtkColor tmpc;

 tmpc.r = r;
 tmpc.g = g;
 tmpc.b = b;
 return tmpc;
} 

void OtkSetColorVector( float r, float g, float b, OtkColorVector tmpc )
{
 tmpc[0] = r;
 tmpc[1] = g;
 tmpc[2] = b;
 tmpc[3] = 1.0;
}

void OtkTranslateColor( OtkColor Cin, OtkColorVector tmpc )
{
 tmpc[0] = Cin.r;
 tmpc[1] = Cin.g;
 tmpc[2] = Cin.b;
 tmpc[3] = 1.0;
}

void OtkCopyColor( OtkColorVector dst, OtkColorVector src )
{ int j;
  for (j=0; j < 4; j++) dst[j] = src[j];
}


int  Otk_snifferrors( int ernum )
{ int error, ecnt=0; 
  while ((error = glGetError()) != GL_NO_ERROR)  
   {ecnt++; printf("GL error %d: %s\n", ernum, gluErrorString(error) );} 
  return ecnt; 
}


void OtkInitLighting()
{
 glEnable( GL_CULL_FACE );    /* Save performance! */
 glEnable( GL_DEPTH_TEST );     /* Depth buffering. */
}


void OtkSetViewWindow( float xmin, float xmax, float ymin, float ymax, float Zmin, float Zmax )
{
 glMatrixMode( GL_PROJECTION );
 glLoadIdentity();
 glOrtho( xmin, xmax, ymin, ymax, Zmin, Zmax  );
}


void OtkSetCameraPosition( float CamX, float CamY, float CamZ,
			  float PointX, float PointY, float PointZ,
			  float OrienX, float OrienY, float OrienZ
			)
{
 glMatrixMode( GL_MODELVIEW );
 gluLookAt(  CamX, CamY, CamZ, PointX, PointY, PointZ, OrienX, OrienY, OrienZ );
}


void Otk_Set_Camera( float minx, float maxx, float miny, float maxy, float minZ, float maxZ, float cx, float cy, float cz )
{ /*set_camera*/
      OtkCameraPosition.x = cx;
      OtkCameraPosition.y = cy;
      OtkCameraPosition.z = cz;
      OtkCameraPosition.minZ = minZ;
      OtkCameraPosition.maxZ = maxZ;
      OtkSetViewWindow( minx, maxx, miny, maxy, minZ, maxZ );  	/* minx, max, miny, maxy, minZ, maxZ. */
      /* Camera location (x,y,z), Pointed at (x,y,z), Up direction (x,y,z). */
      OtkCameraPosition.pointx = 0.0;
      OtkCameraPosition.pointy = 0.0;
      OtkCameraPosition.pointz = 0.0;
      OtkSetCameraPosition( OtkCameraPosition.x, OtkCameraPosition.y, OtkCameraPosition.z, OtkCameraPosition.pointx, OtkCameraPosition.pointy, OtkCameraPosition.pointz, 0.0, 1.0,  0.0 );
} /*set_camera*/


void Otk_Set_Window_Name( char *name )
{
 otk_window_name = strdup( name );
}


/* FONTS begin */
void Otk_glShear( float xy, float xz, float yx, float yz, float zx, float zy )
{
 float m[16] = { 1, xy, xz, 0,  yx, 1, yz, 0,  zx, zy, 1, 0,  0, 0, 0, 1 };
 glMultMatrixf( m );
}
/* FONTS end */

#define mouse_groove_click 6
#if (WinGLEnabled==0)
 Bool OtkMultiSample=0;
#else
 int OtkMultiSample=0;
#endif

/* Prototypes */
OtkWidget OtkMakePanel( OtkWidget container, int panel_subtype, OtkColor panel_color, float left, float top, float horiz_size, float vert_size );
void otk_update_draw_object( OtkWidget objpt );	/* Redraw just the given object.  For rapid updates when */
void Otk_Set_Default_Button_Color( float r, float g, float b );
void Otk_swap_buffers();
void Otk_fb_clear_last_selected();
void otk_check_timer();
int Otk_handle_mouse_click( int state, int button_number );
int BLEND=0;
double otk_started_time;
int otk_realize_image=1;	/* Flag to control display of read image, or to get image-data only. */

void otk_set_realize_image( int j ) { otk_realize_image = j; }


void Otk_Get_Screen_Size( int *scrn_width, int *scrn_height )	/* Passes back screen size in pixels. */
{
 #if (PLATFORM_KIND == Mingw_Platform)
  *scrn_width = GetSystemMetrics(SM_CXSCREEN);
  *scrn_height = GetSystemMetrics(SM_CYSCREEN);
 #else
  Display *Otkdpy;
  int screen_number;
  Otkdpy = XOpenDisplay(NULL);
  screen_number = DefaultScreen( Otkdpy);
  *scrn_width = DisplayWidth( Otkdpy, screen_number );
  *scrn_height = DisplayHeight( Otkdpy, screen_number );
  XCloseDisplay(Otkdpy);
 #endif
}


void OtkMakeOuterWindow()
{
 otk_started_time = otk_report_time();
 OtkInitLighting();
 if (Otk_snifferrors(300)) printf("OGL Errors on initial read-in.\n");
 glClearColor( 0.8, 0.8, 0.85, 1.0 );
 glClear( GL_COLOR_BUFFER_BIT );
 Otk_swap_buffers();
 Otk_Set_Camera( 0.0, 100.0,  -100.0, 0.0,  5.0, 510.0,   0.0, 0.0, 505.0 );  /* minx, maxx, miny, maxy, minZ, maxZ, camera_x_position, camera_y_position, camera_z_position. */
 /* Make the outer window. */
 OtkMakePanel( 0, Otk_subtype_plane, OtkSetColor( 0.75, 0.75, 0.75 ), 0.0, 0.0, 100.0, 100.0 );
 glDisable( GL_LIGHTING );

 /* For transparency. */
 glBlendFunc( GL_SRC_ALPHA, GL_DST_ALPHA );

 if (BLEND)
  {
   if (OtkMultiSample)
    {
     #if (WinGLEnabled==0)
      if (Otk_verbose) printf("Enabling MultiSampling\n");
      glEnable(GL_MULTISAMPLE);
      glHint( GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST );
     #endif
    }
   glEnable(GL_LINE_SMOOTH);
   glEnable(GL_BLEND);
   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
   glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );	// GL_DONT_CARE 
  }

 Otk_Set_Default_Button_Color( 0.75, 0.75, 0.75 );
 Otk_Set_HoverBox_Style( OtkSetColor( 1.0, 1.0, 0.75 ), Otk_Black, 1.0 );
}



void Otk_AcceptCommandLineArguments( int argc, char **argv )
{
 int k=1;

 printf("\nUsing OTK - V%1.2f\n\n", Otk_version);

 k = 1;
 while (k<argc)
  {
   if (argv[k][0]=='-')
    {
     if (strcmp(argv[k],"-otk_v")==0)
      {
	k++;  if (argc <= k) {printf("ERROR: Missing verbosity level.\n"); exit(0);}
	if (sscanf(argv[k],"%d",&Otk_verbose)!=1) {printf("ERROR: Bad verbosity level '%s'.\n",argv[k]); exit(0);}
	printf(" Setting verbosity to %d\n", Otk_verbose);
      }
     else
     if (strcmp(argv[k],"-windowsz_x")==0)
      {
	k++;  if (argc <= k) {printf("ERROR: Missing windowsz_x value.\n"); exit(0);}
	if (sscanf(argv[k],"%d",&OtkWindowSizeX)!=1) {printf("ERROR: Bad windowsz_x '%s'.\n",argv[k]); exit(0);}
	printf(" Setting windowsz_x to %d\n", OtkWindowSizeX);
      }
     else
     if (strcmp(argv[k],"-windowsz_y")==0)
      {
	k++;  if (argc <= k) {printf("ERROR: Missing windowsz_y value.\n"); exit(0);}
	if (sscanf(argv[k],"%d",&OtkWindowSizeY)!=1) {printf("ERROR: Bad windowsz_y '%s'.\n",argv[k]); exit(0);}
	printf(" Setting windowsz_y to %d\n", OtkWindowSizeY);
      }
     else
     if (strcmp(argv[k],"-aspect_ratio")==0)
      {
	k++;  if (argc <= k) {printf("ERROR: Missing Aspect Ratio value.\n"); exit(0);}
	if (sscanf(argv[k],"%f",&Otk_AspectAngle)!=1) {printf("ERROR: Bad Aspect Ratio '%s'.\n",argv[k]); exit(0);}
	printf(" Setting Aspect Ratio to %g\n", Otk_AspectAngle);
      }
     else
     if (strcmp(argv[k],"-dz")==0)
      {
	k++;  if (argc <= k) {printf("ERROR: Missing dz value.\n"); exit(0);}
	if (sscanf(argv[k],"%f",&Otk_DZ)!=1) {printf("ERROR: Bad dz '%s'.\n",argv[k]); exit(0);}
	printf(" Setting DZ to %g\n", Otk_DZ);
      }
     else
     if (strcmp(argv[k],"-blend")==0)
      {
	BLEND = 1;
	printf(" Setting BLEND\n");
      }
     else
     if (strcmp(argv[k],"-otk_test_capture")==0)
      {
	Otk_Start_Capturing_TestFile();
      }
     else
     if (strcmp(argv[k],"-otk_test_play")==0)
      {
	otk_test_playback = 1;
      }
     else
     if (strcmp(argv[k],"-otk_make_movie")==0)
      {
	Otk_SnapShot_Size(2);		/* Set snap-shoot to half-sized, for better speed. */
	Otk_MakeMovie(1);		/* Turn movie frame capture on. */
      }
     else
     if (strcmp(argv[k],"-h")==0)
      {
	printf("\nOtk Option Summary:\n");
	printf(" -v xx		 : Set verbosity. Default = 0.\n");
	printf(" -windowsz_x	 : Initial window width.\n");
	printf(" -windowsz_y	 : Initial window heigth.\n");
	printf(" -aspect_ratio	 : Aspect ratio.\n");
	printf(" -otk_test_capture : Record test file.\n");
	printf(" -otk_test_play	 : Read test file.\n");
	printf(" -h		 : Show options.\n");
      }
     // else printf("Unknown option '%s'\n", argv[k]);
    }
   k++;
  }
 Otk_Keyboard_state.shiftkey = 0;
}


float Otk_Set_Render_Quality_Hint( float q )	/* Sets render-quality-hint.  Returns previous value. */
{
 float oldq=1.0/(fabs(otk_render_quality_hint)+1e-12);
 otk_render_quality_hint = 1.0 / (fabs(q) + 1.0e-12);	/* Prevent divide-by-zero. */
 return oldq;
}



void Otk_Dispose_Image( char *filename )
{ /* Frees all resources associated with named image. */
  struct Otk_image *image, *prv=0;

  #if (PLATFORM_KIND != MsVisC_Platform)
  image = Otk_image_list;         /* Check to find image in list. */
  while ((image!=0) && (strcmp(image->filename,filename)!=0))
   { prv = image;  image = image->nxt; }
  if (image == 0) { printf("Otk_Dispose_Image: '%s' not presently loaded.\n", filename ); return;  /* Not in list. */ }
  if (prv == 0) Otk_image_list = image->nxt;	/* Remove from list. */
  else prv->nxt = image->nxt;
  glDeleteTextures( 1, &(image->texturename) );	/* Remove resources. */
  free( image->image );
  free( image->textureimage );
  free (image );
  #endif
}


struct Otk_image *Otk_Read_Image_File( char *fname )
{
 struct Otk_image *image;
 #if (PLATFORM_KIND != MsVisC_Platform)
 FILE *infile;
 char buf[2048], capsfname[2048], *ofname;
 int ispipe=0, istmp=0;
 int magic[2], ch;
 int nrows, ncols, maxval, row, col=0, r, g, b, maximgsz;
 unsigned char rc, bc, gc;

 image = Otk_image_list;	 /* Check to see if this image file was already read. */
 while ((image!=0) && (strcmp(image->filename,fname)!=0)) image = image->nxt;
 /* If already read, then return pointer to image. */
 if (image!=0) { return image; }

 ofname = fname;
 do {capsfname[col] = toupper(fname[col]); col++; } while (fname[col-1]!='\0');
 #ifdef SCZ_DEFS
  {
   struct Otk_image *Otk_Read_SCZ_Image_File( char *fname, char *capsfname );
   if ( ((strstr(capsfname,".SCZ")!=0) && (strlen(strstr(capsfname,".SCZ"))==4))
	|| ((strstr(capsfname,".ISCZ")!=0) && (strlen(strstr(capsfname,".ISCZ"))==5)) )
    { return Otk_Read_SCZ_Image_File( fname, capsfname ); }
  }
 #endif

  /* Check if image needs to be converted. */
  if ((strstr(capsfname,".JPG")!=0) || (strstr(capsfname,".JPEG")!=0))
   {
    sprintf(buf,"jpegtopnm %s > tmpimg.ppm", fname);
    system(buf);  fname = strdup("tmpimg.ppm");   istmp = 1;
   }
  else
  if (strstr(capsfname,".GIF")!=0) 
   {
    sprintf(buf,"giftopnm %s > tmpimg.ppm", fname);
    system(buf);  fname = strdup("tmpimg.ppm");   istmp = 1;
  }
 else
 if (strstr(capsfname,".PNG")!=0) 
  {
   sprintf(buf,"pngtopnm %s > tmpimg.ppm", fname);
   system(buf);  fname = strdup("tmpimg.ppm");    istmp = 1;
  }

 infile = fopen(fname,"rb");
 if (infile==0) { printf("ERROR: Could not open '%s'.\n", fname);  return 0; }
 if (Otk_verbose) printf("READING IMAGE %s\n",fname);

 magic[0] = getc(infile);  /* Read magic numerals. */
 magic[1] = getc(infile);
 if ( magic[0] != 'P' || magic[1] < '1' || magic[1] > '6' ) 
  {
   fclose( infile );
   snprintf( buf, 2048, "anytopnm %s", fname );
   infile = popen( buf, "rb" );
   if (infile==0) { printf("OTK ERROR: Could not read or convert '%s'.\n", fname);  return 0; }
   ispipe = 1;
   magic[0] = getc(infile);
   magic[1] = getc(infile);
  }

 switch ( magic[1] )
  {
   case '1': /* pbm bitmap */
   case '4': /* binary */
      printf("ERROR: Bad PBM image file '%s'.\n", fname);
      return 0;
    break;
   case '2': /* pgm greyscale */
   case '5': /* binary */
   case '3': /* ppm pixmap */
   case '6': /* binary */
    while ( (ch = getc(infile)) && isspace(ch) );  /* Chew whitespace. */
    while ( ch == '#' ) 
     {
      while ( getc(infile) != '\n' );	/* Eat comment line. */
      ch = getc(infile);
     }
    ungetc( ch, infile );
    fscanf( infile, "%d", &ncols );	/* Number of columns */
    while ( (ch = getc(infile)) && isspace(ch) ); /* chew whitespace */
    while ( ch == '#' )
     {
      while ( getc(infile) != '\n' );	/* Eat comment line. */
      ch = getc(infile);
     }
    ungetc( ch, infile );
    fscanf( infile, "%d", &nrows ); 	/* Number of rows. */
    while ( (ch = getc(infile)) && isspace(ch) ); /* chew whitespace */
    while ( ch == '#' )
     {
      while ( getc(infile) != '\n' );	/* Eat comment line. */
      ch = getc(infile);
     }
    ungetc( ch, infile );
    fscanf( infile, "%d", &maxval );	/* Maximum value of a pixel. */
    while (getc(infile)!='\n');

    if ((ncols * nrows < 0) || (ncols * nrows > 25000000))
     { printf("Otk Error: Image size %d x %d out of range.\n", ncols, nrows );  return 0; }

    /* Get the data. */
    image = (struct Otk_image *)malloc( sizeof(struct Otk_image) );
    image->filename = (char *)strdup(ofname);
    image->image = (struct Otk_image_rec *)malloc( (ncols * nrows) * sizeof(struct Otk_image_rec) );
    if (image->image == 0) {printf("Otk Error: Could not allocate image memory.\n");  return 0; }
    image->native_cols = ncols;   image->native_rows = nrows;
    image->cols = ncols;   image->rows = nrows;
    if (nrows<=32) image->texturerows = 32; else
    if (nrows<=64) image->texturerows = 64; else
    if (nrows<=128) image->texturerows = 128; else
    if (nrows<=256) image->texturerows = 256; else
    if (nrows<=512) image->texturerows = 512; else
    if (nrows<=1024) image->texturerows = 1024; else
    if (nrows<=2048) image->texturerows = 2048; else image->texturerows = 4096;
    if (ncols<=32) image->texturecols = 32; else
    if (ncols<=64) image->texturecols = 64; else
    if (ncols<=128) image->texturecols = 128; else
    if (ncols<=256) image->texturecols = 256; else
    if (ncols<=512) image->texturecols = 512; else
    if (ncols<=1024) image->texturecols = 1024; else
    if (ncols<=2048) image->texturecols = 2048; else image->texturecols = 4096;
    image->texturesize = image->texturerows * image->texturecols;
    image->textureimage = (GLubyte *)malloc( 4 * image->texturesize * sizeof(GLubyte) );
    if (image->textureimage == 0) {printf("Otk Error: Could not allocate image memory.\n");  return 0; }
    image->texturename = OtkTextureNumber++;

    if ( magic[1] == '6' ) 
     { /* binary */
      int k=0;
      for (row=0;  row != nrows;  row++)
       {
        for (col=0;  col != ncols;  col++)
         {
          fread( &rc, 1, 1, infile );
          fread( &gc, 1, 1, infile );
          fread( &bc, 1, 1, infile );
          image->image[k].r = rc;
          image->image[k].b = bc;
          image->image[k].g = gc;
	  k++;
         }
       }
     }
    else
    if ( magic[1] == '5' ) 
     { /* binary */
      int k=0;
      for (row=0;  row != nrows;  row++)
        for (col=0;  col != ncols;  col++)
        {
         fread( &rc, 1, 1, infile );
         image->image[k].r = rc;
         image->image[k].b = rc;
         image->image[k].g = rc;
	 k++;
        }
     }
    else
     { /* ascii */
      int k=0;
      for (row=0;  row != nrows;  row++)
       for (col=0;  col != ncols;  col++)
        {
         fscanf(infile,"%d",&r);    fscanf(infile,"%d",&g);     fscanf(infile,"%d",&b);
         image->image[k].r = r;
         image->image[k].b = b;
         image->image[k++].g = g;
        }
     }
    break;
  }

 if( ispipe ) pclose( infile );
 else    fclose( infile );
 // printf("nrows (columns) = %d,  ncols (rows) = %d\n", nrows, ncols );

 if (istmp) { remove("tmpimg.ppm");  free(fname); }
 if (otk_realize_image == 0) { return image; }

 glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maximgsz );

 /* Force nrows to equal ncols and be power of 2 for OGL. */
 if ((nrows != ncols) || (image->texturerows != image->rows) || (image->texturerows > maximgsz))
  {
   int new_nrows, new_ncols, j, k=2, m;
   float tmpfc, tmpfr;
   struct Otk_image_rec *tmpimage;

   if (nrows > ncols)
    { new_nrows = nrows;  new_ncols = nrows; }
   else
    { new_nrows = ncols;  new_ncols = ncols; }
   while (k < new_nrows) k = 2 * k;
   if (k > maximgsz) k = maximgsz;	/* Limit size if larger than current display capability. */
   new_nrows = k;  new_ncols = k;
   tmpimage = (struct Otk_image_rec *)malloc( (new_nrows * new_ncols) * sizeof(struct Otk_image_rec) );
   if (tmpimage == 0) {printf("Otk Error: Could not allocate image memory.\n");  return 0; }
   m = 0;
   tmpfc = (float)ncols / (float)new_ncols;
   tmpfr = (float)nrows / (float)new_nrows;
   for (row=0; row<new_nrows; row++)
    {
     j = (float)row * tmpfr;
     j = j * ncols;
     for (col=0; col<new_ncols; col++)
      {
       k = j + (float)col * tmpfc;
       tmpimage[m].r = image->image[k].r;
       tmpimage[m].g = image->image[k].g;
       tmpimage[m].b = image->image[k].b;
       m++;
      }
    }
   free( image->image );
   image->image = tmpimage;
   image->cols = new_nrows;   image->rows = new_ncols;
   image->texturerows = new_nrows;
   image->texturecols = new_nrows;
   image->texturesize = image->texturerows * image->texturecols;
   free(image->textureimage);
   image->textureimage = (GLubyte *)malloc( 4 * image->texturesize * sizeof(GLubyte) );
   if (image->textureimage == 0) {printf("Otk Error: Could not allocate image memory.\n");  return 0; }
  }

 {
  int p = 0, k, m = 0;
  for (row=0;  row < image->texturerows;  row++)
   {
    k = p;
    for (col=0;  col < image->texturecols;  col++)
     { 
       image->textureimage[m++] = image->image[k].r;
       image->textureimage[m++] = image->image[k].g;
       image->textureimage[m++] = image->image[k].b;
       image->textureimage[m++] = 255;
       k++;
     }
    p = p + image->cols;
   }
 }

 image->nxt = Otk_image_list;
 Otk_image_list = image;

 image->calllist_num = glGenLists(1);

 glDisable( GL_TEXTURE_2D );
 glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
 glGenTextures( 1, &(image->texturename) );
 glBindTexture( GL_TEXTURE_2D, image->texturename );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
 glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image->texturerows, image->texturecols, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->textureimage );

 glNewList( image->calllist_num, GL_COMPILE );
 glBindTexture( GL_TEXTURE_2D, image->texturename );
 glEndList();

 #else
  image = 0;	/* Unsupported. */
 #endif
 return image;
}



#ifdef SCZ_DEFS
struct Otk_image *Otk_Read_SCZ_Image_File( char *fname, char *capsfname )
{
 struct Otk_image *image;
 #if (PLATFORM_KIND != MsVisC_Platform)
 int magic[2], ch;
 int nrows, ncols, maxval, row, col=0, r, g, b, maximgsz;
 unsigned char rc, bc, gc;
 char *buffer;
 int bufindx=0, buflen;

 if (Otk_verbose) printf("READING IMAGE %s\n",fname);

 if (Scz_Decompress_File2Buffer( fname, &buffer, &buflen )!=1) 
   { printf("SCZ Error: Unable to decompress file '%s'.\n", fname);  return 0; }

 magic[0] = buffer[bufindx++];  /* Read magic numerals. */
 magic[1] = buffer[bufindx++];
 if ( magic[0] != 'P' || magic[1] < '1' || magic[1] > '6' ) 
  {
   printf("OTK ERROR: Could not read or convert '%s'.\n", fname);
   free(buffer);
   return 0;
  }

 switch ( magic[1] )
  {
   case '1': /* pbm bitmap */
   case '4': /* binary */
      printf("ERROR: Bad PBM image file '%s'.\n", fname);
      free(buffer);
      return 0;
    break;
   case '2': /* pgm greyscale */
   case '5': /* binary */
   case '3': /* ppm pixmap */
   case '6': /* binary */
    do
     {
      while ( buffer[bufindx++] != '\n' );	/* Advance to next line. */
      while ( isspace(buffer[bufindx]) ) bufindx++;
     }
    while (buffer[bufindx]=='#');    
    ncols = atoi( &(buffer[bufindx]) );	/* Number of columns */
    while ( ! isspace(buffer[bufindx]) ) bufindx++;	/* Advance to next item. */
    while ( isspace(buffer[bufindx]) ) bufindx++;
    while (buffer[bufindx]=='#')
     {
      while ( buffer[bufindx++] != '\n' );      /* Advance to next line. */
      while ( isspace(buffer[bufindx]) ) bufindx++;
     }
    nrows = atoi( &(buffer[bufindx]) ); 	/* Number of rows. */
    while ( ! isspace(buffer[bufindx]) ) bufindx++;	/* Advance to next item. */
    while ( isspace(buffer[bufindx]) ) bufindx++;
    while (buffer[bufindx]=='#')
     {
      while ( buffer[bufindx++] != '\n' );      /* Advance to next line. */
      while ( isspace(buffer[bufindx]) ) bufindx++;
     }
    maxval = atoi( &(buffer[bufindx]) );	/* Maximum value of a pixel. */
    while (buffer[bufindx++]!='\n');
    if ((nrows * ncols) > 25000000) {printf("Image too large.\n"); exit(0);}

    if ((ncols * nrows < 0) || (ncols * nrows > 25000000))
     { printf("Otk Error: Image size %d x %d out of range.\n", ncols, nrows );  return 0; }

    /* Get the data. */
    image = (struct Otk_image *)malloc( sizeof(struct Otk_image) );
    image->filename = (char *)strdup(fname);
    image->image = (struct Otk_image_rec *)malloc( (ncols * nrows) * sizeof(struct Otk_image_rec) );
    if (image->image == 0) {printf("Otk Error: Could not allocate image memory.\n");  free(buffer);  return 0; }
    image->native_cols = ncols;   image->native_rows = nrows;
    image->cols = ncols;   image->rows = nrows;
    if (nrows<=32) image->texturerows = 32; else
    if (nrows<=64) image->texturerows = 64; else
    if (nrows<=128) image->texturerows = 128; else
    if (nrows<=256) image->texturerows = 256; else
    if (nrows<=512) image->texturerows = 512; else
    if (nrows<=1024) image->texturerows = 1024; else
    if (nrows<=2048) image->texturerows = 2048; else image->texturerows = 4096;
    if (ncols<=32) image->texturecols = 32; else
    if (ncols<=64) image->texturecols = 64; else
    if (ncols<=128) image->texturecols = 128; else
    if (ncols<=256) image->texturecols = 256; else
    if (ncols<=512) image->texturecols = 512; else
    if (ncols<=1024) image->texturecols = 1024; else
    if (ncols<=2048) image->texturecols = 2048; else image->texturecols = 4096;
    image->texturesize = image->texturerows * image->texturecols;
    image->textureimage = (GLubyte *)malloc( 4 * image->texturesize * sizeof(GLubyte) );
    if (image->textureimage == 0) {printf("Otk Error: Could not allocate image memory.\n");  free(buffer);  return 0; }
    image->texturename = OtkTextureNumber++;

    if ( magic[1] == '6' ) 
     { /* binary */
      int j, k=0;
      if (buflen < 3 * nrows * ncols + bufindx) {printf("Otk ERROR: Image Buffer too small reading '%s'. buffer=%d < image= %d\n", fname, buflen, 3 * nrows * ncols + bufindx); exit(0);}
      for (row=0; row!=nrows; row++)
       {
        for (col=0; col!=ncols; col++)
         {
          rc = buffer[bufindx++];
          gc = buffer[bufindx++];
          bc = buffer[bufindx++];
	  j = k + col;
          image->image[j].r = rc;
          image->image[j].b = bc;
          image->image[j].g = gc;
         }
	k = k + ncols;
       }
     }
    else
    if ( magic[1] == '5' ) 
     { /* binary */
      if (buflen < 3 * nrows * ncols + bufindx) {printf("Image Buffer too small. %d<%d\n", buflen, 3 * nrows * ncols + bufindx); exit(0);}
      for (row=0; row!=nrows; row++)
        for (col=0; col!=ncols; col++)
        {
         rc = buffer[bufindx++];
         image->image[row*ncols+col].r = rc;
         image->image[row*ncols+col].b = rc;
         image->image[row*ncols+col].g = rc;
        }
     }
    else
     { /* ascii */
      for (row=0; row!=nrows; row++)
        for (col=0; col!=ncols; col++)
        {
         r = atoi( &(buffer[bufindx]) );	while (!isspace(buffer[bufindx++]));
	 if (bufindx>buflen) {printf("Image Buffer too small.\n"); exit(0);}
	 g = atoi( &(buffer[bufindx]) );		while (!isspace(buffer[bufindx++]));
	 if (bufindx>buflen) {printf("Image Buffer too small.\n"); exit(0);}
	 b = atoi( &(buffer[bufindx]) );	while (!isspace(buffer[bufindx++]));
	 if (bufindx>buflen) {printf("Image Buffer too small.\n"); exit(0);}
         image->image[row*ncols+col].r = r;
         image->image[row*ncols+col].b = b;
         image->image[row*ncols+col].g = g;
        }
     }
    break;
  }

 if (Otk_verbose>1) printf("	%d rows by %d columns.\n", nrows, ncols );
 glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maximgsz );

  if ((strstr(capsfname,".ISCZ")!=0) && (strlen(strstr(capsfname,".ISCZ"))==5))
   { /*iscz*/
     struct Otk_image_rec imel, last_imel;
     int k=0, m, tmpv;
     /* Integrate the image, to convert it from differential encoding. */
     for (row=0; row < nrows; row++)
      {
       last_imel = image->image[k];
       for (col=1; col < ncols; col++)
        {
	 m = k + col;
         imel = image->image[ m ];

         tmpv = imel.r + last_imel.r - 127.0;
         if ((int)(tmpv) < 0) tmpv = 0;  else
         if ((int)(tmpv) > 255) tmpv = 255;
	 image->image[m].r = tmpv;

         tmpv = imel.g + last_imel.g - 127.0;
         if ((int)(tmpv) < 0) tmpv = 0;  else
         if ((int)(tmpv) > 255) tmpv = 255;
	 image->image[m].g = tmpv;

         tmpv = imel.b + last_imel.b - 127.0;
         if ((int)(tmpv) < 0) tmpv = 0;  else
         if ((int)(tmpv) > 255) tmpv = 255;
	 image->image[m].b = tmpv;

         last_imel = image->image[ m ];
        }
       k = k + ncols;
      }
   } /*iscz*/

 /* Force nrows to equal ncols and be power of 2 for OGL. */
 glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maximgsz );
 if ((nrows != ncols) || (image->texturerows != image->rows) || (image->texturerows > maximgsz))
  {
   int new_nrows, new_ncols, j, k=2, m1, m2;
   float tmpfr, tmpfc;
   struct Otk_image_rec *tmpimage;

   if (nrows > ncols)
    { new_nrows = nrows;  new_ncols = nrows; }
   else
    { new_nrows = ncols;  new_ncols = ncols; }
   while (k < new_nrows) k = 2 * k;
   if (k > maximgsz) k = maximgsz;
   new_nrows = k;  new_ncols = k;
   tmpimage = (struct Otk_image_rec *)malloc( (new_nrows * new_ncols) * sizeof(struct Otk_image_rec) );
   if (tmpimage == 0) {printf("Otk Error: Could not allocate image memory.\n");  free(buffer);  return 0; }
   m1 = 0;
   tmpfc = (float)ncols / (float)new_ncols;
   tmpfr = (float)nrows / (float)new_nrows;
   for (row=0; row<new_nrows; row++)
    {
     j = (float)row * tmpfr;
     m2 = j * ncols;
     for (col=0; col<new_ncols; col++)
      {
       k = m2 + (int)((float)col * tmpfc);
       tmpimage[m1].r = image->image[k].r;
       tmpimage[m1].g = image->image[k].g;
       tmpimage[m1].b = image->image[k].b;
       m1++;
      }
    }
   if (Otk_verbose>1) printf("	Forcing resize to %d rows by %d columns.\n", new_nrows, new_ncols );
   free( image->image );
   image->image = tmpimage;
   image->cols = new_nrows;   image->rows = new_ncols;
   image->texturerows = new_nrows;
   image->texturecols = new_nrows;
   image->texturesize = image->texturerows * image->texturecols;
   free(image->textureimage);
   image->textureimage = (GLubyte *)malloc( 4 * image->texturesize * sizeof(GLubyte) );
   if (image->textureimage == 0) {printf("Otk Error: Could not allocate image memory.\n");  free(buffer);  return 0; }
  }

 {
  int p = 0, k, m = 0;
  for (row=0;  row < image->texturerows;  row++)
   {
    k = p;
    for (col=0;  col < image->texturecols;  col++)
     { 
       image->textureimage[m++] = image->image[k].r;
       image->textureimage[m++] = image->image[k].g;
       image->textureimage[m++] = image->image[k].b;
       image->textureimage[m++] = 255;
       k++;
     }
    p = p + image->cols;
   }
 }

 image->nxt = Otk_image_list;
 Otk_image_list = image;

 image->calllist_num = glGenLists(1);

 glDisable( GL_TEXTURE_2D );
 glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
 glGenTextures( 1, &(image->texturename) );
 glBindTexture( GL_TEXTURE_2D, image->texturename );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
 glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image->texturerows, image->texturecols, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->textureimage );

 glNewList( image->calllist_num, GL_COMPILE );
 glBindTexture( GL_TEXTURE_2D, image->texturename );
 glEndList();
 free(buffer);
 #else
  image = 0;	/* Unsupported. */
 #endif
 return image;
}

#endif	/*SCZ_DEFS*/


struct Otk_image *Otk_Make_Image_From_Matrix( char *name, int nrows, int ncols, struct Otk_image_rec *newimage )
{
 struct Otk_image *image;
 #if (PLATFORM_KIND != MsVisC_Platform)
 int row, col, maximgsz, mm;

 image = Otk_image_list;	 /* Check to see if this image file was already read. */
 while ((image!=0) && (strcmp(image->filename,name)!=0)) image = image->nxt;

 /* Get the data. */
 if (image==0)
  {
   image = (struct Otk_image *)malloc( sizeof(struct Otk_image) );
   image->filename = (char *)strdup(name);
   image->nxt = Otk_image_list;
   Otk_image_list = image;
   image->calllist_num = glGenLists(1);
  }
 else free(image->image);

 if ((ncols * nrows < 0) || (ncols * nrows > 25000000))
  { printf("Otk Error: Image size %d x %d out of range.\n", ncols, nrows );  return 0; }

 image->image = (struct Otk_image_rec *)malloc( (ncols * nrows) * sizeof(struct Otk_image_rec) );
 if (image->image == 0) {printf("Otk Error: Could not allocate image memory.\n");  return 0; }
 image->native_cols = ncols;   image->native_rows = nrows;
 image->cols = ncols;   image->rows = nrows;
 if (nrows<=32) image->texturerows = 32; else
 if (nrows<=64) image->texturerows = 64; else
 if (nrows<=128) image->texturerows = 128; else
 if (nrows<=256) image->texturerows = 256; else
 if (nrows<=512) image->texturerows = 512; else
 if (nrows<=1024) image->texturerows = 1024; else
 if (nrows<=2048) image->texturerows = 2048; else image->texturerows = 4096;
 if (ncols<=32) image->texturecols = 32; else
 if (ncols<=64) image->texturecols = 64; else
 if (ncols<=128) image->texturecols = 128; else
 if (ncols<=256) image->texturecols = 256; else
 if (ncols<=512) image->texturecols = 512; else
 if (ncols<=1024) image->texturecols = 1024; else
 if (ncols<=2048) image->texturecols = 2048; else image->texturecols = 4096;
 image->texturesize = image->texturerows * image->texturecols;
 image->textureimage = (GLubyte *)malloc( 4 * image->texturesize * sizeof(GLubyte) );
 if (image->textureimage == 0) {printf("Otk Error: Could not allocate image memory.\n");  return 0; }
 image->texturename = OtkTextureNumber++;

 mm = 0;
 for (row=0; row!=nrows; row++)
  for (col=0; col!=ncols; col++)
   {
    image->image[mm] = newimage[mm];
    mm++;
   }

 /* Force nrows to equal ncols and be power of 2 for OGL. */
 glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maximgsz );
 if ((nrows != ncols) || (image->texturerows != image->rows) || (image->texturerows > maximgsz))
  {
   int new_nrows, new_ncols, j, k=2, m;
   float tmpfc, tmpfr;
   struct Otk_image_rec *tmpimage;

   // printf("RESIZING\n");
   if (nrows >= ncols) { new_nrows = nrows;  new_ncols = nrows; }
   if (nrows < ncols) { new_nrows = ncols;  new_ncols = ncols; }
   while (k < new_nrows) k = 2 * k;
   if (k > 2048) k = 2048;
   new_nrows = k;  new_ncols = k;
   tmpimage = (struct Otk_image_rec *)malloc( (new_nrows * new_ncols) * sizeof(struct Otk_image_rec) );
   if (tmpimage == 0) {printf("Otk Error: Could not allocate image memory.\n");  return 0; }
   m = 0;
   tmpfc = (float)ncols / (float)new_ncols;
   tmpfr = (float)nrows / (float)new_nrows;
   for (row=0;  row < new_nrows;  row++)
    {
     j = (float)row * tmpfr;
     j = j * ncols;
     for (col=0;  col < new_ncols;  col++)
      {
       k = j + (float)col * tmpfc;
       tmpimage[m].r = image->image[k].r;
       tmpimage[m].g = image->image[k].g;
       tmpimage[m].b = image->image[k].b;
       m++;
      }
    }
   free( image->image );
   image->image = tmpimage;
   image->cols = new_nrows;   image->rows = new_ncols;
   image->texturerows = new_nrows;
   image->texturecols = new_nrows;
   image->texturesize = image->texturerows * image->texturecols;
   free(image->textureimage);
   image->textureimage = (GLubyte *)malloc( 4 * image->texturesize * sizeof(GLubyte) );
   if (image->textureimage == 0) {printf("Otk Error: Could not allocate image memory.\n");  return 0; }
  }

 for (row=0; row < image->texturerows; row++)
  for (col=0; col < image->texturecols; col++)
   { int k, m;
    k = row;  // (image->texturerows - row - 1) % image->rows;
    m = col;  // col % image->cols;
    image->textureimage[4 * (image->texturecols * row + col) + 0] = image->image[k * image->cols + m ].r;
    image->textureimage[4 * (image->texturecols * row + col) + 1] = image->image[k * image->cols + m ].g;
    image->textureimage[4 * (image->texturecols * row + col) + 2] = image->image[k * image->cols + m ].b;
    image->textureimage[4 * (image->texturecols * row + col) + 3] = 255;
   }

 glDisable( GL_TEXTURE_2D );
 glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
 glGenTextures( 1, &(image->texturename) );
 glBindTexture( GL_TEXTURE_2D, image->texturename );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
 glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
 glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image->texturerows, image->texturecols, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->textureimage );
 glNewList( image->calllist_num, GL_COMPILE );
 glBindTexture( GL_TEXTURE_2D, image->texturename );
 glEndList();

 #else
  image = 0;	/* Unsupported. */
 #endif
 return image;
}


void otk_set_not_native_window()
{ otk_native_window = 0; }




#if (GlutEnabled==0)
#if (WinGLEnabled==0)

Display *Otkdpy;
Window  Otkwin;
Bool OtkDoubleBuffer=1;
XSizeHints OtkSizeHints={0};
int OtkConfiguration[] = { GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DEPTH_SIZE, 12, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1, None,      None, None };

Atom wmDeleteWindow;
Cursor otk_mouse_cursor_font[4];
XVisualInfo *Otkvi;


void otk_set_multisamp( int value )	/* Set or remove multi-samp request. */
{ 				/* Multi-sampling provides sharper graphics, if GPU supports it. */
 int j=0;			/* Defaults to Off. To set On, call this before calling OtkInitWindow(...). */
 while ((OtkConfiguration[j] != 0) && (OtkConfiguration[j] != GLX_SAMPLES)) j++;
 if (OtkConfiguration[j] != 0)
  {
   if (value == 0)
    {
     OtkConfiguration[j] = 0; 
     OtkMultiSample = 0;
    }
   else
    { 
     OtkConfiguration[j+1] = value;
     OtkMultiSample = 1;
    }
  }
 else
 if (value != 0)
  {
   OtkConfiguration[j++] = GLX_SAMPLES;
   OtkConfiguration[j++] = value;
   OtkConfiguration[j] = None;
   OtkMultiSample = 1;
  }
}

void otk_deactivate_cpu_antialiasing()
{
 otk_render_qual_level = 0;
 allow_cpu_antialiasing = 0;
}


Colormap OtkGetShareableColormap( XVisualInfo *vi )
{
 Status status;
 XStandardColormap *standardCmaps;
 Colormap cmap;
 int i, numCmaps;

 if (vi->class != TrueColor)  { printf("TrueColor visual required\n"); exit(0); }
 status = XmuLookupStandardColormap( Otkdpy, vi->screen, vi->visualid, vi->depth, XA_RGB_DEFAULT_MAP, False, True );
 if (status == 1)
  {
   status = XGetRGBColormaps( Otkdpy, RootWindow(Otkdpy, vi->screen), &standardCmaps, &numCmaps, XA_RGB_DEFAULT_MAP );
   if (status == 1)
    for (i=0; i < numCmaps; i++)
     if (standardCmaps[i].visualid == vi->visualid)
      { cmap = standardCmaps[i].colormap;  XFree( standardCmaps );  return cmap; }
  }
 cmap = XCreateColormap( Otkdpy, RootWindow(Otkdpy, vi->screen), vi->visual, AllocNone );
 return cmap;
}


void OtkInitWindow( int WinWidth, int WinHeight, int argc, char **argv )
{
 XVisualInfo *vi;
 Colormap cmap;
 XSetWindowAttributes swa;
 XWMHints *wmHints;
 GLXContext cx;
 char *geometry=0;
 int flags, j, x, y, width, height;

 Otk_AcceptCommandLineArguments( argc, argv );
 if (OtkWindowSizeX>0) WinWidth  = OtkWindowSizeX; else OtkWindowSizeX = WinWidth;
 if (OtkWindowSizeY>0) WinHeight = OtkWindowSizeY; else OtkWindowSizeY = WinHeight;

 /* 1. Open connection to X-server. */
 Otkdpy = XOpenDisplay(NULL);
 if (Otkdpy==NULL) {printf("Error: Could not open display.\n"); exit(0);}

 /* 2. Get GLX. */
 if (! glXQueryExtension( Otkdpy, 0, 0 )) {printf("X-server has no OpenGL GLX extnsion!\n"); exit(0);}

 /* 3. Get OpenGL visual. */
 vi = glXChooseVisual( Otkdpy, DefaultScreen(Otkdpy), OtkConfiguration );	/* First ask for everything (double-buff + multi-samp). */
 if (vi==0)
  { /* Cannot get everything, so reduce multi-samp request, and ask again. */
   if (OtkMultiSample)
    { /*ReduceMultisample*/
      j = 0;
      while ((OtkConfiguration[j] != 0) && (OtkConfiguration[j] != GLX_SAMPLES)) j++;
      while ((vi == 0) && (OtkConfiguration[j+1] > 2))
       {
   	OtkConfiguration[j+1] = OtkConfiguration[j+1] / 2;
   	vi = glXChooseVisual( Otkdpy, DefaultScreen(Otkdpy), OtkConfiguration );
       }
      if (vi)
       {
	if (Otk_verbose) printf(" MultiSample reduced to max supported of %d\n", OtkConfiguration[j+1] );
       }
      else
       {
	if (Otk_verbose) printf("Multi-sample DENIED.\n");
	otk_set_multisamp( 0 );
	vi = glXChooseVisual( Otkdpy, DefaultScreen(Otkdpy), OtkConfiguration );	/* Ask again with reduced resources. */
       }
    } /*ReduceMultisample*/
   if (vi==0)	/* If cannot get requested resources for any reason above, then remove double-buffer request, and ask again. */
    { /*nomultisamp*/
       printf("Double-Buffering DENIED.\n");
       vi = glXChooseVisual( Otkdpy, DefaultScreen(Otkdpy), &OtkConfiguration[1] );	/* Ask for absolute minimum. */
       if (vi==NULL) { printf("Error: No RGB visual with depth buffer.\n");  exit(1); } /* If still can't get it, must exit. */
       OtkDoubleBuffer = False;
    } /*nomultisamp*/
  }
 // if (OtkMultiSample) otk_deactivate_cpu_antialiasing();  	/* No need for CPU anti-aliasing, if GPU supports "multi-sampling" mode. */
 if (OtkMultiSample) otk_render_quality_hint = 2.0 * otk_render_quality_hint;	/* Less need for CPU anti-aliasing, if GPU supports "multi-sampling" mode. */
 Otkvi = vi;
 glXGetConfig( Otkdpy, Otkvi, GLX_ACCUM_RED_SIZE, &otk_got_accumbuf );
 // printf("Got %d bits accum buffer.\n", otk_got_accumbuf);
 cmap = OtkGetShareableColormap(vi);

 if (otk_native_window)
  { /*otk_native_window*/
   /* 4. Create OpenGL rendering context. */
   cx = glXCreateContext( Otkdpy, vi, None, True );
   if (cx==NULL)  {printf("could not create rendering context.\n"); exit(0);}

   /* 5. Create X window. */
   flags = XParseGeometry( geometry, &x, &y, (unsigned int *)&width, (unsigned int *)&height );
   if (WidthValue & flags) { OtkSizeHints.flags = USSize | OtkSizeHints.flags;  OtkSizeHints.width = width; }
   if (HeightValue & flags) { OtkSizeHints.flags = USSize | OtkSizeHints.flags; OtkSizeHints.height = height; }
   if (XValue & flags) 
    { if (XNegative & flags) x = DisplayWidth( Otkdpy, DefaultScreen(Otkdpy) ) + x - OtkSizeHints.width; OtkSizeHints.flags = USPosition | OtkSizeHints.flags; OtkSizeHints.x = x; }
   if (YValue & flags) 
    { if (YNegative & flags) x = DisplayHeight( Otkdpy, DefaultScreen(Otkdpy) ) + y - OtkSizeHints.height; OtkSizeHints.flags = USPosition | OtkSizeHints.flags; OtkSizeHints.y = y; }
   swa.colormap = cmap;
   swa.border_pixel = 0;
   swa.event_mask = ExposureMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyPressMask | KeyReleaseMask | PointerMotionMask;
   Otkwin = XCreateWindow( Otkdpy, RootWindow(Otkdpy, vi->screen), OtkSizeHints.x, OtkSizeHints.y, WinWidth, WinHeight, 
			0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa );

   XSetStandardProperties( Otkdpy, Otkwin, otk_window_name, "glxOtk", None, argv, argc, &OtkSizeHints );
   wmHints = XAllocWMHints();
   wmHints->initial_state = NormalState;
   wmHints->flags = StateHint;
   XSetWMHints( Otkdpy, Otkwin, wmHints );
   wmDeleteWindow = XInternAtom( Otkdpy, "WM_DELETE_WINDOW", False );
   XSetWMProtocols( Otkdpy, Otkwin, &wmDeleteWindow, 1 );

   /* 6. Bind the window. */
   glXMakeCurrent( Otkdpy, Otkwin, cx );
  } /*otk_native_window*/

 /* Set mouse cursors. */
 otk_mouse_cursor_font[Otk_MouseCursor_Arrow] = XCreateFontCursor( Otkdpy, XC_arrow );
 otk_mouse_cursor_font[Otk_MouseCursor_Cross] = XCreateFontCursor( Otkdpy, XC_cross );
 otk_mouse_cursor_font[Otk_MouseCursor_Hand] =  XCreateFontCursor( Otkdpy, XC_hand2 );
 otk_mouse_cursor_font[Otk_MouseCursor_Wait] =  XCreateFontCursor( Otkdpy, XC_watch );

 OtkMakeOuterWindow();
}

#else 

/* WinGLEnabled */

HDC Otkdpy;
HWND  Otkwin;
int OtkDoubleBuffer=1;
HCURSOR otk_mouse_cursor_font[4];

struct
{
  int width;
  int height;
} OtkSizeHints={0,0};

LRESULT	CALLBACK Otk_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
 switch( uMsg )
  { /*case*/
   case WM_SIZE:
    if (GET_X_LPARAM(lParam) * GET_Y_LPARAM(lParam) < OtkWindowSizeX * OtkWindowSizeY) { otk_highest_qual_tried = 0; }
    OtkWindowSizeX = GET_X_LPARAM(lParam);	/* Set to new window size. */
    OtkWindowSizeY = GET_Y_LPARAM(lParam);
    if (Otk_verbose) printf("resized window %d %d\n", OtkWindowSizeX, OtkWindowSizeY );
    glViewport( 0, 0, OtkWindowSizeX , OtkWindowSizeY );
    Otk_Display_Changed++;
    //glGetIntegerv( GL_VIEWPORT, Otk->viewport );
    break;
   case WM_CLOSE:
    printf("Forced Exit by Window Manager!\n");
    exit(0);
    break;
   case WM_PAINT:
    Otk_Display_Changed++;
   break;
  }
 return( DefWindowProc( hWnd, uMsg, wParam, lParam ) );
}

PIXELFORMATDESCRIPTOR Otkconfiguration = {
  sizeof(PIXELFORMATDESCRIPTOR),			     /* Size Of This Pixel Format Descriptor */
  1,							     /* Version Number */
  PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, /* Support Window, OpenGL, Double Buffering */
  PFD_TYPE_RGBA,					     /* Request An RGBA Format */
  24,  							     /* Select Our Color Depth */
  0, 0, 0, 0, 0, 0,					     /* Color Bits Ignored */
  0,							     /* No Alpha Buffer */
  0,							     /* Shift Bit Ignored */
  16,							     /* Accumulation Buffer Bits */
  0, 0, 0, 0,						     /* Accumulation Bits Ignored */
  0,							     /* 16Bit Z-Buffer (Depth Buffer)   */
  0,							     /* No Stencil Buffer */
  0,							     /* No Auxiliary Buffer */
  PFD_MAIN_PLANE,					     /* Main Drawing Layer */
  0,							     /* Reserved */
  0, 0, 0						     /* Layer Masks Ignored */
};

void otk_set_multisamp( int value )	/* Set or remove multi-samp request. */
{ ; }					/*  (Stub-out here since not implemented on microsoft.) */

void OtkInitWindow( int WinWidth, int WinHeight, int argc, char **argv )
{
 HGLRC cx = NULL;     /* Permanent Rendering Context */
 HINSTANCE hInstance; /* Holds The Instance Of The Application */
 GLuint	PixelFormat;  /* Holds The Results After Searching For A Match */
 WNDCLASS wc; 	  /* Windows Class Structure */
 DWORD dwExStyle; /* Window Extended Style */
 DWORD dwStyle;   /* Window Style */
 RECT WindowRect; /* Grabs Rectangle Upper Left / Lower Right Values */
 PIXELFORMATDESCRIPTOR pfd;

 //pfd.cColorBits = 24; /* Number Of Bits To Use For Color (8/16/24/32) */

 Otk_AcceptCommandLineArguments( argc, argv );
 if (OtkWindowSizeX > 0) WinWidth  = OtkWindowSizeX; else OtkWindowSizeX = WinWidth;
 if (OtkWindowSizeY > 0) WinHeight = OtkWindowSizeY; else OtkWindowSizeY = WinHeight;
 OtkSizeHints.width = WinWidth;
 OtkSizeHints.height = WinHeight;

 if (otk_native_window)
  { /*otk_native_window*/
    WindowRect.left = (long)0;
    WindowRect.right = (long)OtkSizeHints.width;
    WindowRect.top = (long)0;
    WindowRect.bottom = (long)OtkSizeHints.height;

    hInstance = GetModuleHandle( NULL ); /* Grab An Instance For Our Window */
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; /* Redraw On Size, And Own DC For Window. */
    wc.lpfnWndProc = (WNDPROC) Otk_WndProc; /* WndProc Handles Events */
    wc.cbClsExtra = 0; /* No Extra Window Data */
    wc.cbWndExtra = 0; /* No Extra Window Data */
    wc.hInstance = hInstance; /* Set The Instance */
    wc.hIcon = LoadIcon( NULL, IDI_WINLOGO ); /* Load The Default Icon */
    wc.hCursor = NULL; /* Load The Arrow Pointer */
    wc.hbrBackground = NULL; //(HBRUSH)(COLOR_WINDOW+1); /* No Background Required For GL */
    wc.lpszMenuName = "wglOtk"; /* We Don't Want A Menu */
    wc.lpszClassName = "wglOtk"; /* Set The Class Name */

    /* 1. Open connection to window-server. */
    if (!RegisterClass(&wc)) /* Attempt To Register The Window Class */
     {
      MessageBox( NULL, "Failed To Register The Window Class.", "ERROR", MB_OK|MB_ICONEXCLAMATION );
      exit(-1);
     }

    /* 5. Create the window. */
    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE; /* Window Extended Style */
    dwStyle = WS_OVERLAPPEDWINDOW; /* Windows Style */
    AdjustWindowRectEx( &WindowRect, dwStyle, FALSE, dwExStyle ); /* Adjust Window To True Requested Size */
    OtkSizeHints.width = WindowRect.right - WindowRect.left;
    OtkSizeHints.height = WindowRect.bottom - WindowRect.top;
    if (!(Otkwin = CreateWindow( "wglOtk", otk_window_name,
                             WS_OVERLAPPEDWINDOW|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
			     CW_USEDEFAULT, CW_USEDEFAULT, OtkSizeHints.width, OtkSizeHints.height,
			     NULL, NULL, /* No Parent Window, No Menu */
			     hInstance, NULL )))
     {
      MessageBox( NULL, "Window Creation Error.", "ERROR", MB_OK|MB_ICONEXCLAMATION );
      exit(-1);
     }
    if (!(Otkdpy=GetDC(Otkwin))) /* Did We Get A Display Context? */
     {
      MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      exit(-1);
     }

    /* 3. Get OpneGL visual. */
    PixelFormat = ChoosePixelFormat( Otkdpy, &Otkconfiguration );
    if (PixelFormat == 0) /* Did Windows Find A Matching Pixel Format? */
     {
      MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      exit(-1);
     }
    if(!SetPixelFormat(Otkdpy, PixelFormat, &Otkconfiguration)) /* Are We Able To Set The Pixel Format? */
     {
      MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      exit(-1);
     }

    /* 4. Create OpenGL rendering context. */
    if (!(cx = wglCreateContext( Otkdpy ))) /* Are We Able To Get A Rendering Context? */
     {
      MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      exit(-1);
     }

    /* 6. Bind the window. */
    if(!wglMakeCurrent( Otkdpy, cx )) /* Try To Activate The Rendering Context */
     {
      MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
      exit(-1);
     }
    ShowWindow( Otkwin, SW_SHOW ); /* Show The Window */
    SetForegroundWindow( Otkwin ); /* Slightly Higher Priority */
    SetFocus( Otkwin );            /* Sets Keyboard Focus To The Window */
    // printf("SETTING WINDOW to %d x %d\n", WinWidth, WinHeight );
  } /*otk_native_window*/

 OtkMakeOuterWindow();

 PixelFormat = GetPixelFormat( wglGetCurrentDC() );
 DescribePixelFormat( wglGetCurrentDC(), PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd ); 
 otk_got_accumbuf = pfd.cAccumBits;

 /* Set mouser cursors. */
 otk_mouse_cursor_font[Otk_MouseCursor_Arrow] = LoadCursor( NULL, IDC_ARROW );
 otk_mouse_cursor_font[Otk_MouseCursor_Cross] = LoadCursor( NULL, IDC_CROSS );
 otk_mouse_cursor_font[Otk_MouseCursor_Hand] =  LoadCursor( NULL, IDC_HAND );
 otk_mouse_cursor_font[Otk_MouseCursor_Wait] =  LoadCursor( NULL, IDC_WAIT );

 SetCursor( otk_mouse_cursor_font[Otk_MouseCursor_Arrow] );
} /*OtkInitWindow*/

#endif /* WinGLEnabled */

#else /* GlutEnabled */

void OtkInitWindow( int WinWidth, int WinHeight, int argc, char **argv )
{

  Otk_AcceptCommandLineArguments( argc, argv );
  if (OtkWindowSizeX>0) WinWidth  = OtkWindowSizeX; else OtkWindowSizeX = WinWidth;
  if (OtkWindowSizeY>0) WinHeight = OtkWindowSizeY; else OtkWindowSizeY = WinHeight;

 if (otk_native_window)
  { /*otk_native_window*/
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_ACCUM );
    glutInitWindowSize( WinWidth, WinHeight );
    glutInitWindowPosition( 50, 50 );
    glutCreateWindow( otk_window_name );
  } /*otk_native_window*/

  OtkMakeOuterWindow();
}

#endif



void OtkUpdateWindow( int WinWidth, int WinHeight )
{
  OtkWindowSizeX = WinWidth;
  OtkWindowSizeY = WinHeight;
}


void otk_sleep( float dt )
{
   #if (WinGLEnabled==0)
    usleep( (int)(dt * 1e6) );
   #else
    Sleep((int)(dt * 1e3) );	
   #endif
}


#define OTKMAXSTR 999


OtkWidget otk_playtest_pause_window=0;

 void Otk_Resume_Playback( void *data )
{
printf("Resuming.\n");
 otk_started_time = otk_report_time();
 Otk_RemoveObject( otk_playtest_pause_window ); 
 otk_playtest_pause_window = 0;
}


void Otk_Start_Capturing_TestFile()
{
  if (otk_test_file)
   printf("Otk Warning:  Attempt to open test-file, when already open.\n");
  else
   {
    otk_test_file = fopen("otk_test_capture.xml","wb");
    if (otk_test_file == 0)
     printf("Otk Error:  Could not open 'otk_test_capture.xml' for writing.\n");
    else
     printf("Capturing Otk interactions to 'otk_test_capture.xml'.\n");
   }
}


void Otk_Close_TestFile()
{
 if (otk_test_file)
  { fclose(otk_test_file);  otk_test_file = 0; }
 else 
  printf("Otk Warning:  Attempt to close test-file, when not open.\n");
}



void Otk_Apply_Test_Stimuli( char *filename )
{
 char tag[OTKMAXSTR]="", tagname[OTKMAXSTR], attrname[OTKMAXSTR], value[OTKMAXSTR];
 int linenum=0, key, state, idx, idy, k;
 double t2, dt, lastscrnupdate;
 float x, y, dx, dy, mousesz=1.0, arrowsz=0.5;
 FILE *infile;
 OtkWidget mousesprite[3];

 if (otk_test_playback == 2) { printf("Apply_Test_Stimuli multiply called.\n"); return; }
 otk_test_playback = 2;
 otk_started_time = otk_report_time();
 OtkUpdateCheck();
 lastscrnupdate = otk_report_time();
 infile = fopen(filename,"r");    
 if (infile==0) {printf("Error: Cannot open input file '%s'.\n", filename); return;} 
 printf("Applying automatic OTK test stimulus from file '%s'.\n", filename);
 mousesprite[0] = Otk_Add_Line( OtkOuterWindow, Otk_Black, 4.0, 50.0-mousesz, 50.0-mousesz, 50.0+mousesz, 50.0+mousesz );
 mousesprite[0]->z = mousesprite[0]->z + 399.0;
 mousesprite[1] = Otk_Add_Line( OtkOuterWindow, Otk_Black, 4.0, 50.0-mousesz, 50.0-mousesz, 50.0, 50.0-arrowsz );
 mousesprite[1]->z = mousesprite[1]->z + 399.0;
 mousesprite[2] = Otk_Add_Line( OtkOuterWindow, Otk_Black, 4.0, 50.0-mousesz, 50.0-mousesz, 50.0-arrowsz, 50.0 );
 mousesprite[2]->z = mousesprite[2]->z + 399.0;
 while (!feof(infile))
  { /*tag_line*/	/* Simple xml parser, 'cause it's not worth bringing full-blown one into project. */
   if (otk_report_time() - lastscrnupdate > 0.1)
    {
     OtkUpdateCheck();
     lastscrnupdate = otk_report_time();
    }

   if (tag[0] == '\0')
    {
     do { fgets( tag, OTKMAXSTR, infile );  linenum++; }
     while ((!feof(infile)) && (strstr(tag,"<") == 0));   /* Handles blank lines. */
    }
   Otk_next_word( tag, tagname, " \t<>=\"");
   // printf("tagname = '%s'\n", tagname);

   while ((!strcmp(tagname,"!--")) && (!feof(infile)))
    { /*xml_comment*/
      do
       { Otk_next_word( tag, tagname, " \t<>=\"\n\r");
	 if (tagname[0] == '\0') { fgets( tag, OTKMAXSTR, infile );  linenum++; }
       } 
      while ((!feof(infile)) && (strcmp(tagname,"--") != 0));
      Otk_next_word( tag, tagname, " \t<>=\"\n\r");
    } /*xml_comment*/
   if (feof(infile)) { tagname[0] = '\0'; }

   Otk_next_word( tag, attrname, " \t<>=\"");
   Otk_next_word( tag, value, " \t<>=\"");
   if (!strcmp(tagname,"key"))
    {
     // printf("	key\n");
     if (sscanf(value,"%d", &key) != 1) printf("Error: bad test key value '%s' on line %d.\n", value, linenum);
     if (!strcmp(attrname,"press")) Otk_handle_key_input( key );
     else
     if (!strcmp(attrname,"release")) Otk_handle_key_release( key );
    }
   else
   if (!strcmp(tagname,"mouse_click"))
    { int button_number=1;
     // printf("	click\n");
     if (strcmp(attrname,"x")) printf("Error: expected 'x' attribute but found '%s' on line %d.\n", attrname, linenum);
     if (sscanf(value,"%f", &x) != 1) printf("Error: bad mouse x value '%s' on line %d.\n", value, linenum);
     Otk_next_word( tag, attrname, " \t<>=\"");  Otk_next_word( tag, value, " \t<>=\"");
     if (strcmp(attrname,"y")) printf("Error: expected 'y' attribute but found '%s' on line %d.\n", attrname, linenum);
     if (sscanf(value,"%f", &y) != 1) printf("Error: bad test mouse y value '%s' on line %d.\n", value, linenum);
     Otk_next_word( tag, attrname, " \t<>=\"");  Otk_next_word( tag, value, " \t<>=\"");
     if (strcmp(attrname,"state")) printf("Error: expected 'state' attribute but found '%s' on line %d.\n", attrname, linenum);
     if (sscanf(value,"%d", &state) != 1) printf("Error: bad test mouse state value '%s' on line %d.\n", value, linenum);
     Otk_next_word( tag, attrname, " \t<>=\"");  Otk_next_word( tag, value, " \t<>=\"");
     if (strcmp(attrname,"button")==0) 
      if (sscanf(value,"%d", &button_number) != 1) printf("Error: bad test mouse button value '%s' on line %d.\n", value, linenum);
     Otk_MousePixX = (int)(0.01 * x * (float)OtkWindowSizeX + 0.5);
     Otk_MousePixY = (int)(0.01 * y * (float)OtkWindowSizeY + 0.5);
     Otk_handle_mouse_click( state, button_number );   /* State:  0=mouse-button-down-press, 1=mouse-button-release. */
    }
   else
   if (!strcmp(tagname,"mouse_move"))
    {
     // printf("	move\n");
     if (strcmp(attrname,"x")) printf("Error: expected 'x' attribute but found '%s' on line %d.\n", attrname, linenum);
     if (sscanf(value,"%f", &x) != 1) printf("Error: bad test mouse value '%s' on line %d.\n", value, linenum);
     Otk_next_word( tag, attrname, " \t<>=\"");  Otk_next_word( tag, value, " \t<>=\"");
     if (strcmp(attrname,"y")) printf("Error: expected 'y' attribute but found '%s' on line %d.\n", attrname, linenum);
     if (sscanf(value,"%f", &y) != 1) printf("Error: bad test mouse value '%s' on line %d.\n", value, linenum);
     Otk_next_word( tag, attrname, " \t<>=\"");  Otk_next_word( tag, value, " \t<>=\"");
     if (strcmp(attrname,"dx")) printf("Error: expected 'dx' attribute but found '%s' on line %d.\n", attrname, linenum);
     if (sscanf(value,"%f", &dx) != 1) printf("Error: bad test mouse value '%s' on line %d.\n", value, linenum);
     Otk_next_word( tag, attrname, " \t<>=\"");  Otk_next_word( tag, value, " \t<>=\"");
     if (strcmp(attrname,"dy")) printf("Error: expected 'dy' attribute but found '%s' on line %d.\n", attrname, linenum);
     if (sscanf(value,"%f", &dy) != 1) printf("Error: bad test mouse value '%s' on line %d.\n", value, linenum);
     Otk_MousePixX = (int)(0.01 * x * (float)OtkWindowSizeX + 0.5);
     Otk_MousePixY = (int)(0.01 * y * (float)OtkWindowSizeY + 0.5);
     idx = (int)(0.01 * dx * (float)OtkWindowSizeX  + 0.5);
     idy = (int)(0.01 * dy * (float)OtkWindowSizeY  + 0.5);
     mousesprite[0]->xleft = x - mousesz;  mousesprite[0]->ytop = y - mousesz;      mousesprite[0]->xright = x + arrowsz;  mousesprite[0]->ybottom = y + arrowsz;
     mousesprite[1]->xleft = x - mousesz;  mousesprite[1]->ytop = y - mousesz;      mousesprite[1]->xright = x;  	   mousesprite[1]->ybottom = y - arrowsz;
     mousesprite[2]->xleft = x - mousesz;  mousesprite[2]->ytop = y - mousesz;      mousesprite[2]->xright = x - arrowsz;  mousesprite[2]->ybottom = y;
     Otk_Display_Changed++;
     Otk_handle_mouse_move( idx, idy );
    }
   else
   if (strcmp(tagname,"t")==0)
    {
     if (sscanf(value,"%lf",&t2) != 1) printf("Error: bad test time value '%s' on line %d.\n", value, linenum);
     dt = t2 - otk_report_relative_time();
     while (dt > 0.0)
      {
       if (dt < 0.1) { otk_sleep( dt );  dt = 0.0; }
       else { otk_sleep( 0.1 );  OtkUpdateCheck();  dt = t2 - otk_report_relative_time(); }
      }
    }
   else
   if (!strcmp(tagname,"pause"))
    {
     printf("Script Pausing\n");
     otk_playtest_pause_window = OtkMakeWindow( Otk_Raised, Otk_Blue, Otk_LightGray, 20, 20, 50, 20 );
     OtkMakeTextLabel( otk_playtest_pause_window, "Script Paused.", Otk_Black, 3.0, 1.0, 2.0, 10.0 );
     // OtkMakeTextLabel( otk_playtest_pause_window, "(Click below to Resume.)", Otk_Black, 2.0, 1.0, 4.0, 30.0 );
     OtkMakeButton( otk_playtest_pause_window, 36, 60, 28, 27.5, "Resume  ", Otk_Resume_Playback, 0 );
     do { otk_sleep( 0.1 );  OtkUpdateCheck(); } while (otk_playtest_pause_window);
    }
   else
   if (!strcmp(tagname,"!--"))
    { /*xml_comment*/
      do
       { Otk_next_word( tag, tagname, " \t<>=\"");	/* Absorb comment, until end-of-comment. */
	 if (tagname[0] == '\0') { fgets( tag, OTKMAXSTR, infile );  linenum++; }
       } 
      while ((!feof(infile)) && (strcmp(tagname,"--") != 0));
    }
   else
   if (tagname[0] != '\0')
    printf("Error: Unknown tag '%s' in line %d.\n", tagname, linenum );
   Otk_next_word( tag, tagname, "<");	/* Pull off end of last tag, if any still on this line. */
  } /*tag_line*/
 fclose(infile);
 for (k=0; k < 3; k++) Otk_RemoveObject( mousesprite[k] ); 
 otk_test_playback = 0;
 OtkUpdateCheck();
 printf("Ended applying automatic OTK test stimulus.\n");
}


void Otk_Set_Mouse_Cursor( int cursor_style )
{
 if ((cursor_style < 0) || (cursor_style > 4))
  { printf("Otk_Set_Mouse_Cursor: Bad style %d\n", cursor_style );  return; }
 #if (WinGLEnabled==0)
  XDefineCursor( Otkdpy, Otkwin, otk_mouse_cursor_font[ cursor_style ] );
 #else	/*WinGLEnabled*/
  SetCursor( otk_mouse_cursor_font[ cursor_style ] );   
 #endif
 otk_current_mouse_cursor_style = cursor_style;
}


/*=============================================^= Otk =^=============================================*/

/* Otk object maintenance. */

#if(0)	/* Example (or template for) object-hierarchy traversal. */
	void otk_object_list_traverse( OtkWidget top )
	{
	 OtkWidget ptr;
	
	 ptr = top;
	 while( ptr )
	  {
	   /* Operate on each object, here. */
	   /* ... */

	   /* Move to next item. */
	   if( ptr->children ) ptr = ptr->children;			 /* Descend, if children. */
	   else
	    {
	     while ((ptr->nxt == 0) && (ptr != top)) ptr = ptr->parent;	 /* Go up, if end of siblings on any level. */
	     if (ptr!=top) ptr = ptr->nxt;	 /* Advance to next item, unless done. */
	    }
	  }
	}
#endif


void Otk_object_attach( OtkWidget parent, OtkWidget child )	/* Replaces old funct.:  Otk_AddOnObjectList() */
{
 if ( parent )
  {
   child->nxt = parent->children;	/* Adds to head of parent's child list. */
   parent->children = child;
   if (child->nxt == 0) parent->child_tail = child;
  }
 else
  {
   child->nxt = OtkRootObject;
   OtkRootObject = child;
   if (OtkOuterWindow==0) OtkOuterWindow = child;
   if (child->nxt == 0) OtkRootObject->child_tail = child;
  }
 child->parent = parent;
}


void Otk_object_attach_at_end( OtkWidget parent, OtkWidget child )
{
 if ( parent != 0)
  {
    if (parent->children == 0)
     parent->children = child;
   else
    { OtkWidget ptr;
      ptr = parent->children;
      while( ptr->nxt ) ptr = ptr->nxt;
      ptr->nxt = child;
    }
   child->nxt = 0;
   parent->child_tail = child;
  }
 else
  {
   child->nxt = OtkRootObject;
   OtkRootObject = child;
   if (OtkOuterWindow==0) OtkOuterWindow = child;
   if (child->nxt == 0) OtkRootObject->child_tail = child;
  }
 child->parent = parent;
}


void Otk_object_attach_hidden( OtkWidget parent, OtkWidget child )
{
 OtkWidget ptr;

 if ( ! parent->hidden_children )
   parent->hidden_children = child;
 else
  {	/* Attach to end of list. */
   ptr = parent->hidden_children;
   while( ptr->nxt ) ptr = ptr->nxt;
   ptr->nxt = child;
   // parent->hidden_tail->nxt = child;
  }
 child->nxt = NULL;
 child->parent = parent;
 parent->hidden_tail = child;
}


void Otk_object_quick_attach_hidden( OtkWidget parent, OtkWidget child )
{
 if ( ! parent->hidden_children ) parent->hidden_tail = child;
 child->nxt = parent->hidden_children;
 parent->hidden_children = child;
 child->parent = parent;
}

void Otk_object_attach_hidden_at_front(OtkWidget parent, OtkWidget child )
{ Otk_object_quick_attach_hidden( parent, child ); }


void Otk_object_detach( OtkWidget child )	/* Replaces old funct.:  Otk_RemoveFromObjectList() */
{
 OtkWidget tmpobj;

 if (child->parent != 0)	/* Then find this object's predecessor. */
  {
   if (child->parent->children == child) child->parent->children = child->nxt;	  /* Was first-child. */
   else
    {
     tmpobj = child->parent->children;		/* Find previous sibling. */
     if (tmpobj ==0) {printf("Otk Warning: Nothing to detach.\n");  return;}
     while ((tmpobj->nxt != child) && (tmpobj->nxt != 0)) tmpobj = tmpobj->nxt;	  /* Second condition and next statement can be removed when code becomes stable. */
     if (tmpobj->nxt==0) printf("Unexpected ERROR, child not on list.\n");
     tmpobj->nxt = child->nxt;
    }
  }
 else	/* No parent, must be a top-level object. */
  {
   if( OtkRootObject == child )  OtkRootObject = child->nxt;
   else	/* Find previous object. */
    {
     tmpobj = OtkRootObject;
     while ((tmpobj->nxt != child) && (tmpobj->nxt != 0)) tmpobj = tmpobj->nxt; 
     if (tmpobj->nxt==0) printf("Unexpected ERROR, object not on list.\n");
     tmpobj->nxt = child->nxt;
    }
  }
 child->parent = NULL;
 child->nxt = NULL;
}


void Otk_object_detach_hidden( OtkWidget child )
{
 OtkWidget tmpobj;

 if (child->parent != 0) /* Then find this object's predecessor. */
  {
   if (child->parent->hidden_children == child) child->parent->hidden_children = child->nxt;	  /* Was first-child. */
   else
    {
     tmpobj = child->parent->hidden_children;		/* Find previous sibling. */
     while ((tmpobj->nxt != child) && (tmpobj->nxt != 0)) tmpobj = tmpobj->nxt;	  /* Second condition and next statement can be removed when code becomes stable. */
     if (tmpobj->nxt==0) printf("Unexpected ERROR, child not on hidden-children list.\n");
     tmpobj->nxt = child->nxt;
    }
  }
 else	/* No parent, must be a top-level object. */
  printf("Unexpected ERROR, hidden children must have parents.\n");
 child->parent = NULL;
 child->nxt = NULL;
}


void Otk_object_detach_any( OtkWidget child )
{
 OtkWidget tmppt;

 if( child->parent )
  {
   tmppt = child->parent->hidden_children;
   while( tmppt && tmppt != child ) tmppt = tmppt->nxt;
   if( tmppt )
     Otk_object_detach_hidden( child );
   else
     Otk_object_detach( child );
  }
 else
   Otk_object_detach( child );
}


void Otk_object_correct_position( OtkWidget obj, int descend )	/* Re-calculates an object's absolute display coordinates. */
{
 OtkWidget tmpobj;
float dx, dy;

 if (obj->parent!=0)
  {
   dx = 0.01 * (obj->parent->xright - obj->parent->xleft);
   dy = 0.01 * (obj->parent->ybottom - obj->parent->ytop);
   obj->xleft =   obj->parent->xleft + obj->x1 * dx;
   obj->ytop =    obj->parent->ytop + obj->y1 * dy;
   obj->xright =  obj->parent->xleft + obj->x2 * dx;
   obj->ybottom = obj->parent->ytop + obj->y2 * dy;
  }
 else
  {
   obj->xleft =  obj->x1;
   obj->ytop =   obj->y1;
   obj->xright = obj->x2;
   obj->ybottom = obj->y2;
  }
  
 if( descend && obj->children )
  {
   tmpobj = obj->children;
   while (tmpobj && tmpobj!=obj)
    {
     tmpobj->xleft =  tmpobj->parent->xleft + tmpobj->x1 * (tmpobj->parent->xright - tmpobj->parent->xleft) * 0.01;
     tmpobj->ytop =   tmpobj->parent->ytop + tmpobj->y1 * (tmpobj->parent->ybottom - tmpobj->parent->ytop) * 0.01; 
     tmpobj->xright = tmpobj->parent->xleft + tmpobj->x2 * (tmpobj->parent->xright - tmpobj->parent->xleft) * 0.01;
     tmpobj->ybottom = tmpobj->parent->ytop + tmpobj->y2 * (tmpobj->parent->ybottom - tmpobj->parent->ytop) * 0.01; 

     /* traverse heirarchy */
     if( tmpobj->children )
      tmpobj = tmpobj->children;
     else
      {
       while( tmpobj && !tmpobj->nxt && tmpobj != obj )  tmpobj = tmpobj->parent;
       if( tmpobj && tmpobj != obj )  tmpobj = tmpobj->nxt;
      }
    }
  }
}


void Otk_calculate_object_position( OtkWidget container, OtkWidget obj )	/* Calculates an object's absolute display coordinates, relative to container. */
{
 if (container==0) return;
 obj->xleft =   container->xleft + obj->x1 * (container->xright - container->xleft) * 0.01;
 obj->ytop =    container->ytop + obj->y1 * (container->ybottom - container->ytop) * 0.01; 
 obj->xright =  container->xleft + obj->x2 * (container->xright - container->xleft) * 0.01;
 obj->ybottom = container->ytop + obj->y2 * (container->ybottom - container->ytop) * 0.01; 
}


void Otk_position_object( OtkWidget objpt, int absolute, float x, float y )
{
 OtkWidget tmpobj;

 if( absolute )
  {
   x = x - objpt->xleft;
   y = y - objpt->ytop;
  }

 objpt->xleft = objpt->xleft + x;
 objpt->xright = objpt->xright + x;
 objpt->ytop = objpt->ytop + y;
 objpt->ybottom = objpt->ybottom + y;
 objpt->x1 = 100.0*((objpt->xleft - objpt->parent->xleft)/
		    (objpt->parent->xright - objpt->parent->xleft));
 objpt->x2 = 100.0*((objpt->xright - objpt->parent->xleft)/
		    (objpt->parent->xright - objpt->parent->xleft));
 objpt->y1 = 100.0*((objpt->ytop - objpt->parent->ytop)/
		    (objpt->parent->ybottom - objpt->parent->ytop));
 objpt->y2 = 100.0*((objpt->ybottom - objpt->parent->ytop)/
		    (objpt->parent->ybottom - objpt->parent->ytop));

 if( objpt->children )
  {
   tmpobj = objpt->children;
   while( tmpobj != objpt )
    {
     tmpobj->xleft = tmpobj->xleft + x;
     tmpobj->xright = tmpobj->xright + x;
     tmpobj->ytop = tmpobj->ytop + y;
     tmpobj->ybottom = tmpobj->ybottom + y;
     tmpobj->x1 = 100.0*((tmpobj->xleft - tmpobj->parent->xleft)/
			 (tmpobj->parent->xright - tmpobj->parent->xleft));
     tmpobj->x2 = 100.0*((tmpobj->xright - tmpobj->parent->xleft)/
			 (tmpobj->parent->xright - tmpobj->parent->xleft));
     tmpobj->y1 = 100.0*((tmpobj->ytop - tmpobj->parent->ytop)/
			 (tmpobj->parent->ybottom - tmpobj->parent->ytop));
     tmpobj->y2 = 100.0*((tmpobj->ybottom - tmpobj->parent->ytop)/
			 (tmpobj->parent->ybottom - tmpobj->parent->ytop));
     if (tmpobj->children != 0)
       tmpobj = tmpobj->children;				/* Descend. */
     else
      {
       while( tmpobj && !tmpobj->nxt && tmpobj != objpt )	/* Ascend, as needed. */
	 tmpobj = tmpobj->parent;
       if( tmpobj && tmpobj != objpt )			/* Advance. */
	 tmpobj = tmpobj->nxt;
      }
    }
  }
}


void Otk_move_object( OtkWidget objpt, int absolute, float x, float y )
{
 if ( absolute )
  {
   x = x - objpt->x1;
   y = y - objpt->y1;
  }
 x = x * (objpt->parent->xright - objpt->parent->xleft) * 0.01;
 y = y * (objpt->parent->ybottom - objpt->parent->ytop) * 0.01;
 Otk_position_object( objpt, 0, x, y );
}


OtkWidget Otk_add_object( int kind, OtkWidget container )
{
 OtkWidget tmppt;

  tmppt = (OtkWidget)calloc(1,sizeof(struct OtkObjectInstance));
  tmppt->superclass = kind;	/* */
  tmppt->object_class = kind;	/* */
  tmppt->object_subtype = 0;
  tmppt->state = 0;
  tmppt->mouse_sensitive = 0;
  tmppt->scale = 1.0;
  tmppt->sqrtaspect = Otk_sqrtaspect;
  tmppt->callback = 0;
  tmppt->functval1 = 0;
  tmppt->functval2 = 0;
  tmppt->functval3 = 0;
  tmppt->thickness = 1.0;
  tmppt->nrows = 1;
  tmppt->parent = container;
  tmppt->children = 0;
  tmppt->child_tail = 0;
  tmppt->hidden_children = 0;
  tmppt->hidden_tail = 0;
  Otk_object_attach( container, tmppt );
  return tmppt;
}






OtkWidget Otk_Add_Line( OtkWidget container, OtkColor tmpcolor, float thickness, float x1, float y1, float x2, float y2 )
{
  OtkWidget tmpobj;
  float base_x, base_y;

  if ((container->object_class != Otk_class_panel) && (container->object_class != Otk_class_userdefd))
   {printf("Otk Error: Add Line parent not container panel.\n");  return 0; }
  tmpobj = Otk_add_object( Otk_SC_Line, container );
  OtkTranslateColor( tmpcolor, tmpobj->color );
  tmpobj->thickness = thickness;
  tmpobj->x1 = x1;  tmpobj->y1 = y1;
  tmpobj->x2 = x2;  tmpobj->y2 = y2;
  base_x = (container->xright - container->xleft) * 0.01;
  tmpobj->xleft = container->xleft + tmpobj->x1 * base_x;
  tmpobj->xright = container->xleft + tmpobj->x2 * base_x;
  base_y = (container->ybottom - container->ytop) * 0.01;
  tmpobj->ytop = container->ytop + tmpobj->y1 * base_y;
  tmpobj->ybottom = container->ytop + tmpobj->y2 * base_y;
  tmpobj->z = container->z + 0.5 * Otk_DZ;
  Otk_Display_Changed++;
  return tmpobj;
}


void Otk_Draw_Line( OtkWidget tmppt )
{
 float point[3];

 glColor4fv( tmppt->color );
 glLineWidth( tmppt->thickness );
 if( tmppt->slant > 0.0 ) { glLineStipple(tmppt->slant, (unsigned short)tmppt->outlinestyle);  glEnable( GL_LINE_STIPPLE ); }
 glBegin( GL_LINES );
  point[0] = tmppt->xleft;  point[1] = - tmppt->ytop;  point[2] = tmppt->z;
  glVertex3fv( point );
  point[0] = tmppt->xright;  point[1] = - tmppt->ybottom;
  glVertex3fv( point );
 glEnd();
 if( tmppt->slant > 0.0 ) { glDisable( GL_LINE_STIPPLE ); }
}


OtkWidget Otk_Add_BoundingBox( OtkWidget container, OtkColor tmpcolor, float thickness, float x1, float y1, float x2, float y2 )
{ /* Draws box to corners (x1,y1) (x2, y2).  ((x2, y2) are not (width, height), but coordinates of lower corner.) */
  OtkWidget tmpobj;
  float base_x, base_y;

  if ((container->object_class != Otk_class_panel) && (container->object_class != Otk_class_userdefd))
   {printf("Otk Error: Add Box parent not container panel.\n"); return 0;}
  tmpobj = Otk_add_object( Otk_SC_Box, container );
  OtkTranslateColor( tmpcolor, tmpobj->color );
  tmpobj->thickness = thickness;
  tmpobj->x1 = x1;  tmpobj->y1 = y1;
  tmpobj->x2 = x2;  tmpobj->y2 = y2;
  base_x = (container->xright - container->xleft) * 0.01;
  tmpobj->xleft = container->xleft + tmpobj->x1 * base_x;
  tmpobj->xright = container->xleft + tmpobj->x2 * base_x;
  base_y = (container->ybottom - container->ytop) * 0.01;
  tmpobj->ytop = container->ytop + tmpobj->y1 * base_y;
  tmpobj->ybottom = container->ytop + tmpobj->y2 * base_y;
  tmpobj->z = container->z + 0.5 * Otk_DZ;
  Otk_Display_Changed++;
  return tmpobj;
}


void Otk_Draw_Box( OtkWidget tmppt )
{
 float point[3];

 glColor4fv( tmppt->color );
 glLineWidth( tmppt->thickness );
 if( tmppt->slant > 0.0 ) { glLineStipple(tmppt->slant, (unsigned short)tmppt->outlinestyle);  glEnable( GL_LINE_STIPPLE ); }
 glBegin( GL_LINES );
  point[0] = tmppt->xleft;   point[1] = - tmppt->ytop;     point[2] = tmppt->z;	  glVertex3fv( point );
  point[0] = tmppt->xright;  glVertex3fv( point );
  glVertex3fv( point );
  point[1] = - tmppt->ybottom;  glVertex3fv( point );
  glVertex3fv( point );
  point[0] = tmppt->xleft;  glVertex3fv( point );
  glVertex3fv( point );
  point[1] = - tmppt->ytop;  glVertex3fv( point );
 glEnd();
 if( tmppt->slant > 0.0 ) { glDisable( GL_LINE_STIPPLE ); }
}


void Otk_Set_Line_Thickness( OtkWidget tmpobj, float thickness )
{
 tmpobj->thickness = thickness;
}


/********************************************************************************/
/* Otk - Functions								*/
/********************************************************************************/


OtkWidget OtkMakePanel( OtkWidget container, int panel_subtype, OtkColor panel_color, float left, float top, float horiz_size, float vert_size )
{
 OtkWidget tmpobj;
 float base_x, base_y;

 tmpobj = Otk_add_object( Otk_SC_Panel, container );
 tmpobj->object_subtype = panel_subtype;	/* 0=plane, 1=raised, 2=recessed, 3=moveable, 4=recessed-diamond, 5=raised-diamond, 10=invisible. */
 OtkTranslateColor( panel_color, tmpobj->color );
 tmpobj->x1 = left;
 tmpobj->y1 = top;
 tmpobj->x2 = left + horiz_size;
 tmpobj->y2 = top + vert_size;
 tmpobj->outlinestyle = Otk_Default_Button_Outline_Style;
 if (container == 0)
  {
   tmpobj->xleft =     0.0;
   tmpobj->xright =  100.0;
   tmpobj->ytop =      0.0;
   tmpobj->ybottom = 100.0;
   tmpobj->z =         0.01;
  }
 else
  {
   base_x = (container->xright - container->xleft) * 0.01;
   tmpobj->xleft =   container->xleft + tmpobj->x1 * base_x;
   tmpobj->xright =  container->xleft + tmpobj->x2 * base_x;
   base_y = (container->ybottom - container->ytop) * 0.01;
   tmpobj->ytop =    container->ytop + tmpobj->y1 * base_y;
   tmpobj->ybottom = container->ytop + tmpobj->y2 * base_y;
   if (panel_subtype==Otk_Invisible) tmpobj->z = container->z;
   else tmpobj->z = container->z + 0.5 * Otk_DZ;
  }
 return tmpobj;
}



void Otk_SetBorderThickness( OtkWidget container, float thickness )	/* Deprecated in favor of Otk_Set_Object_Border_Thickness(...) */
{ container->thickness = thickness; }

void OtkResizePanel( OtkWidget panel, float left, float top, float horiz_size, float vert_size )
{
 OtkWidget container;

 container = panel->parent;
 panel->x1 = left;
 panel->y1 = top;
 panel->x2 = left + horiz_size;
 panel->y2 = top + vert_size;
 panel->xleft =   container->xleft + panel->x1 * (container->xright - container->xleft) * 0.01;
 panel->xright =  container->xleft + panel->x2 * (container->xright - container->xleft) * 0.01;   
 panel->ytop =    container->ytop + panel->y1 * (container->ybottom - container->ytop) * 0.01;   
 panel->ybottom = container->ytop + panel->y2 * (container->ybottom - container->ytop) * 0.01;
}


OtkWidget OtkMakeImagePanel( OtkWidget container, char *file_name, float left, float top, float horiz_size, float vert_size )
{
 OtkWidget tmpobj;

 tmpobj = Otk_add_object( Otk_SC_Panel, container );
 tmpobj->object_subtype = Otk_ImagePanel;
 tmpobj->x1 = left;
 tmpobj->y1 = top;
 tmpobj->x2 = left + horiz_size;
 tmpobj->y2 = top + vert_size;
 if (container==0)
  {
   tmpobj->xleft =     0.0;
   tmpobj->xright =  100.0;
   tmpobj->ytop =      0.0;
   tmpobj->ybottom = 100.0;
   tmpobj->z =         0.0;
  }
 else
  {
   tmpobj->xleft =   container->xleft + tmpobj->x1 * (container->xright - container->xleft) * 0.01;
   tmpobj->xright =  container->xleft + tmpobj->x2 * (container->xright - container->xleft) * 0.01;   
   tmpobj->ytop =    container->ytop + tmpobj->y1 * (container->ybottom - container->ytop) * 0.01;   
   tmpobj->ybottom = container->ytop + tmpobj->y2 * (container->ybottom - container->ytop) * 0.01;   
   tmpobj->z = container->z + 0.5 * Otk_DZ;
  }
 tmpobj->image = Otk_Read_Image_File( file_name );
 Otk_Display_Changed++;
 return tmpobj;
}


OtkWidget OtkMakeImagePanel_ImgPtr( OtkWidget container, struct Otk_image *image_ptr, float left, float top, float horiz_size, float vert_size )
{
 OtkWidget tmpobj;

 tmpobj = Otk_add_object( Otk_SC_Panel, container );
 tmpobj->object_subtype = Otk_ImagePanel;
 tmpobj->x1 = left;
 tmpobj->y1 = top;
 tmpobj->x2 = left + horiz_size;
 tmpobj->y2 = top + vert_size;
 if (container==0)
  {
   tmpobj->xleft =     0.0;
   tmpobj->xright =  100.0;
   tmpobj->ytop =      0.0;
   tmpobj->ybottom = 100.0;
   tmpobj->z =         0.0;
  }
 else
  {
   tmpobj->xleft =   container->xleft + tmpobj->x1 * (container->xright - container->xleft) * 0.01;
   tmpobj->xright =  container->xleft + tmpobj->x2 * (container->xright - container->xleft) * 0.01;   
   tmpobj->ytop =    container->ytop + tmpobj->y1 * (container->ybottom - container->ytop) * 0.01;   
   tmpobj->ybottom = container->ytop + tmpobj->y2 * (container->ybottom - container->ytop) * 0.01;   
   tmpobj->z = container->z + 0.5 * Otk_DZ;
  }
 tmpobj->image = image_ptr;
 Otk_Display_Changed++;
 return tmpobj;
}


OtkWidget OtkMakeImage( OtkWidget container, char *fname, float left, float top, float horiz_size, float vert_size, float itop, float ileft, float ihorizsz, float ivertsz )
{ OtkWidget tmpobj;
 tmpobj = OtkMakeImagePanel( container, fname, left, top, horiz_size, vert_size );
 // tmpobj->imgv[0] = ileft;
 // tmpobj->imgv[1] = itop;
 // tmpobj->imgv[2] = ileft + ihorizsz;
 // tmpobj->imgv[3] = itop + ivertsz;
 return tmpobj;
}



void Otk_center_text( OtkWidget txt )
{
  float tw, th, horz, vert;

  Otk_Get_Text_Size( txt, &tw, &th );
  horz = txt->parent->xright - txt->parent->xleft;
  vert = txt->parent->ybottom - txt->parent->ytop;
  Otk_move_object( txt, 1, 50.0*(horz - tw)/horz, 50.0*(vert - th)/vert );
}



OtkWidget OtkMakeTextLabel( OtkWidget container, char *text, OtkColor text_color, float scale, float thickness, float x, float y );
void Otk_Get_Text_Size( OtkWidget tmpobj, float *width, float *height );
void Otk_Get_Character_Size( OtkWidget tmpobj, float *width, float *height );
float Otk_Default_Button_BorderThickness=1.0;


OtkWidget OtkMakeButton( OtkWidget container, float left, float top, float horiz_size, float vert_size,
			 char *text, void (*callback)(void *x), void *parameter )
{
 OtkWidget tmpobj;
 float w, h;

 tmpobj = OtkMakePanel( container, Otk_Default_Button_Outline_Style, Otk_Default_Button_Color, left, top, horiz_size, vert_size );
 tmpobj->superclass = Otk_SC_Button;
 tmpobj->object_class = Otk_class_button;
 tmpobj->mouse_sensitive = 1;
 tmpobj->callback = callback;
 tmpobj->callback_param = parameter;
 tmpobj->thickness = Otk_Default_Button_BorderThickness;

 OtkMakeTextLabel( tmpobj, text, OtkSetColor(0.0, 0.0, 0.0), 1.0, 1.0, 0.0, 0.0 );
 Otk_Get_Text_Size( tmpobj->children, &w, &h );
 horiz_size = tmpobj->xright - tmpobj->xleft;
 vert_size = tmpobj->ybottom - tmpobj->ytop;
 tmpobj->children->sqrtaspect = sqrt( (h * horiz_size) / (w * 0.8 * vert_size) );
 tmpobj->children->scale = 0.925 * horiz_size / (w * tmpobj->children->sqrtaspect);
 Otk_center_text( tmpobj->children );		/* FONTS insertion */
 return tmpobj;
}


void Otk_Set_Button_Color( OtkWidget container, OtkColor panel_color )
{ OtkTranslateColor( panel_color, container->color ); }

void Otk_Set_Panel_Color( OtkWidget container, OtkColor panel_color )
{ OtkTranslateColor( panel_color, container->color ); }

void Otk_Set_Default_Button_Color( float r, float g, float b )
{ Otk_Default_Button_Color = OtkSetColor(r, g, b); }

void Otk_Set_Default_Button_BorderThickness( float x )
{ Otk_Default_Button_BorderThickness = x; }

void Otk_Set_Button_Outline_Style( int style )
{ Otk_Default_Button_Outline_Style = style; }

void Otk_Set_Button_BorderThickness( OtkWidget button, float x )
{ button->thickness = x; }


void Otk_Set_Button_State( OtkWidget container, int state )
{
  if ((container->superclass == Otk_SC_Button) || (container->superclass == Otk_SC_TabHandle))
   { if (state!=0) container->object_subtype = Otk_subtype_recessed; else  container->object_subtype = Otk_subtype_raised; }
  else
  if (container->superclass == Otk_SC_Menu_DropDown_Button)
   { if (state!=0) container->object_subtype = Otk_subtype_raised; else  container->object_subtype = Otk_subtype_plane; }
  else
  if ((container->superclass == Otk_SC_RadioButton) || (container->superclass == Otk_SC_ToggleButton)) 
   { if (state!=0) container->object_subtype = Otk_subtype_raised_diamond; else  container->object_subtype = Otk_subtype_recessed_diamond; }
  container->state = state; 
}

int Otk_Get_Button_State( OtkWidget container )
{ return container->state; }


void Otk_Set_Button_Icon( OtkWidget button, struct Otk_image *image_ptr )
{ 
 if (button->image) free( button->image );
 button->image = image_ptr;
 Otk_Display_Changed++;
}

void Otk_Set_Button_Icon_File( OtkWidget button, char *file_name )
{ 
 if (button->image) free( button->image );
 button->image = Otk_Read_Image_File( file_name );
 Otk_Display_Changed++;
}



OtkWidget OtkMakeToggleButton( OtkWidget container, float left, float top, float horiz_size, float vert_size,
			 void (*callback)(int state, void *x), void *parameter )
{
 OtkWidget tmpobj;

 tmpobj = OtkMakePanel( container, Otk_subtype_raised_diamond, Otk_Default_Button_Color, left, top, 0.83 * horiz_size, 1.33 * vert_size );
 tmpobj->superclass = Otk_SC_ToggleButton;
 tmpobj->object_class = Otk_class_togglebutton;
 tmpobj->object_subtype = Otk_subtype_raised_diamond;
 tmpobj->mouse_sensitive = 1;
 Otk_Set_Button_State( tmpobj, 0 );
 tmpobj->functval3 = callback;
 tmpobj->callback_param = parameter;

 return tmpobj;
}


OtkWidget OtkMakeRadioButton( OtkWidget container, float left, float top, float horiz_size, float vert_size,
			 	void (*callback)(void *x), void *parameter )
{
 OtkWidget tmpobj;

 if (container->object_class == Otk_class_radiobutton1)
  {
   DEBUG printf("sub rbutton\n");
   tmpobj = OtkMakePanel( container->parent, Otk_subtype_raised_diamond, Otk_Default_Button_Color, left, top, 0.83 * horiz_size, 1.33 * vert_size );
   tmpobj->superclass = Otk_SC_RadioButton;
   tmpobj->object_class = Otk_class_radiobutton2;	/* Child of default main radio button.  Sibling to non-default ones. */
   tmpobj->object_subtype = Otk_subtype_raised_diamond;
   Otk_Set_Button_State( tmpobj, 0 );
   Otk_object_detach( tmpobj );
   Otk_object_attach( container, tmpobj );
  }
 else
 if (container->object_class == Otk_class_radiobutton2)
  {
   DEBUG printf("subsub rbutton\n");
   tmpobj = OtkMakePanel( container->parent->parent, Otk_subtype_raised_diamond, Otk_Default_Button_Color, left, top, 0.83 * horiz_size, 1.33 * vert_size );
   tmpobj->superclass = Otk_SC_RadioButton;
   tmpobj->object_class = Otk_class_radiobutton2;	/* Initially a child to a sibling.  Convert to par-sibling. */
   tmpobj->object_subtype = Otk_subtype_raised_diamond;
   Otk_Set_Button_State( tmpobj, 0 );
   Otk_object_detach( tmpobj );
   Otk_object_attach( container->parent, tmpobj );
  }
 else
  {
   DEBUG printf("master rbutton\n");
   tmpobj = OtkMakePanel( container, Otk_subtype_recessed_diamond, Otk_Default_Button_Color, left, top, 0.83 * horiz_size, 1.33 * vert_size );
   tmpobj->superclass = Otk_SC_RadioButton;
   tmpobj->object_class = Otk_class_radiobutton1;	/* Main default button. */
   tmpobj->object_subtype = Otk_subtype_recessed_diamond;
   Otk_Set_Button_State( tmpobj, 1 );
  }
 tmpobj->mouse_sensitive = 1;
 tmpobj->callback = callback;
 tmpobj->callback_param = parameter;

 return tmpobj;
}





void Otk_SetRadioButton( OtkWidget topobj )
{
 OtkWidget children;

 Otk_Set_Button_State( topobj, 1 );
 // printf(" Setting rb (%x) to 1\n", topobj);

 /* Reset other radio button states. */
 if (topobj->object_class==Otk_class_radiobutton2) { Otk_Set_Button_State( topobj->parent, 0 );  children = topobj->parent->children; }
 else children = topobj->children;
 while (children!=0)
  {
   if ((children->object_class==Otk_class_radiobutton2) && (children != topobj)) 
    { Otk_Set_Button_State( children, 0 ); }
   children = children->nxt;
  }
 Otk_Display_Changed++;
}



void Otk_ReDraw_Display()
{
 Otk_Display_Changed = Otk_Display_Changed + 4;
}



float Otk_window_level=200.0;


OtkWidget OtkMakeWindow( int panel_type, OtkColor tab_color, OtkColor panel_color, float left, float top, float horiz_size, float vert_size )
{
 OtkWidget windowpane, handlebar, killbutton, cross;
 float handleheight;

if (Otk_window_level > 400) Otk_window_level = 210.0;
 /* Create a window-pane, a handle-bar, and kill-button (on the handle-bar).	*/
 /* The handle-bar is mouse-sensitive to drag-movement.				*/
 /* The handle-bar is the parent, so that drag-moves bring all pieces.		*/
 /* Return the window-pane, because that is the panel for further attachments.  */
 /* Remove operations must remember to remove window-pane's or kill-button's	*/
 /* parent, which is the handle-bar.						*/

 handleheight = 4.0 * (550.0/OtkWindowSizeY) * sqrt(0.008 * vert_size + 0.1);
 if (vert_size < handleheight + 1.0) vert_size = handleheight + 1.0;

 if (OtkOuterWindow==0) { printf("Otk Error: Cannot make moveable window before outer window.\n"); return 0; }
 handlebar = OtkMakePanel( OtkOuterWindow, Otk_subtype_moveable, tab_color, left,  top, horiz_size, handleheight );
 handlebar->superclass = Otk_SC_Window;
 handlebar->z = Otk_window_level;
 handlebar->mouse_sensitive = 1;

 windowpane = OtkMakePanel( handlebar, panel_type, panel_color, left,  top+handleheight, horiz_size, vert_size-handleheight );
 Otk_calculate_object_position( OtkOuterWindow, windowpane );	/* Set window's coords based on outer-window. */
 Otk_Set_Object_Border_Thickness( windowpane, 0.5 );
 windowpane->superclass = Otk_SC_WindowPane;
 windowpane->z = Otk_window_level;

 killbutton = OtkMakePanel( handlebar, Otk_Flat, OtkSetColor(0.7, 0.1, 0.1), 92.0,  20.0, 5, 60.0 );
 cross = Otk_Add_Line( killbutton, OtkSetColor(0.9, 0.9, 0.9), 2.0, 10.0, 10.0, 90.0, 90.0 );
 cross->z = cross->z - 0.3 * Otk_DZ;
 cross = Otk_Add_Line( killbutton, OtkSetColor(0.9, 0.9, 0.9), 2.0, 10.0, 90.0, 90.0, 10.0 );
 cross->z = cross->z - 0.3 * Otk_DZ;
 killbutton->superclass = Otk_SC_Window;
 killbutton->object_class = Otk_class_button;
 killbutton->mouse_sensitive = 1;

 Otk_window_level = Otk_window_level + 10.0;
 return windowpane;
}


void OtkSetWindowTitle( OtkWidget window, OtkColor text_color, char *title )
{
 OtkWidget label;
 float w, h, hsz, vsz;

 window = window->parent;
 label = OtkMakeTextLabel( window, title, text_color, 1.0, 1.0, 5.0, 30.0 );
 /* Now adjust text size to make it fit nicely in the title bar. */
 label->sqrtaspect = 1.0;
 label->scale = 1.0;
 label->x1 = 5.0;
 label->xleft = window->xleft + 0.05 * (window->xright - window->xleft);
 Otk_Get_Text_Size( label, &w, &h );
 hsz = window->xright - window->xleft;
 vsz = window->ybottom - window->ytop;
 if (w/hsz < 0.8) w = 0.8 * hsz;
 label->sqrtaspect = sqrt( (h * hsz) / (w * 0.8 * vsz) );
 label->scale = 0.8 * hsz / (w * label->sqrtaspect);
}


void Otk_RegisterWindowKillEventFunction( OtkWidget topobj, void (*callback)(void *x), void *parameter )
{
 if ((topobj->parent != 0) && (topobj->superclass == Otk_SC_WindowPane)) topobj = topobj->parent;
 topobj->callback = callback;
 topobj->callback_param = parameter;
}



void Otk_Triangle( float *a, float *b, float *c )
{
 float tvec[3]={0.0,0.0,1.0};

 // OtkNormOFtriangle( a, b, c, tvec);
 glNormal3fv( tvec );
 glVertex3fv( a );
 glVertex3fv( b );
 glVertex3fv( c );
}



void Otk_Draw_Triangle( OtkWidget tmppt )
{
 float VRT[3][3], tcolor[4];
 int k;

 glBegin( GL_TRIANGLES );
 for (k=0; k<4; k++) {tcolor[k] = tmppt->color[k];}
 glColor4fv( tcolor );
 VRT[0][0] = tmppt->x1;		VRT[0][1] = - tmppt->y1;		VRT[0][2] = tmppt->z;
 VRT[1][0] = tmppt->x2;		VRT[1][1] = - tmppt->y2;		VRT[1][2] = tmppt->z;
 VRT[2][0] = tmppt->xleft;	VRT[2][1] = - tmppt->ytop;	VRT[2][2] = tmppt->z;
 Otk_Triangle( VRT[0], VRT[1], VRT[2] );
 glEnd(); 
}



OtkWidget Otk_MakeUserDefinedObject( OtkWidget container, float x, float y )
{
 OtkWidget tmpobj;

 if (container->object_class != Otk_class_panel) {printf("Otk Error: Add UserDefinedObject to parent not container panel.\n"); return 0;}
 tmpobj = Otk_add_object( Otk_class_userdefd, container );
 tmpobj->z = container->z + 0.5 * Otk_DZ;
 tmpobj->x1 = x;
 tmpobj->y1 = y;
 tmpobj->xleft = container->xleft + 0.01 * tmpobj->x1 * (container->xright - container->xleft);
 tmpobj->ytop = container->ytop + 0.01 * tmpobj->y1 * (container->ybottom - container->ytop);
 tmpobj->scissor_to_parent = 1;
 tmpobj->Id = glGenLists( 1 );
 return tmpobj;
}




float cosine_table[21][2] = { {0.951057, 0.309017}, {0.809017, 0.587785}, {0.587786, 0.809017},
	 {0.309018, 0.951056}, {1.26759e-06, 1}, {-0.309016, 0.951057}, {-0.587784, 0.809018},
	 {-0.809016, 0.587787}, {-0.951056, 0.309019}, {-1, 2.7736e-06}, {-0.951057, -0.309014},
	 {-0.809019, -0.587782}, {-0.587788, -0.809015}, {-0.309021, -0.951055}, {-4.27961e-06, -1},
	 {0.309013, -0.951058}, {0.587781, -0.80902}, {0.809014, -0.58779}, {0.951055, -0.309022},
	 {1, -6.02404e-06}, {0.951059, 0.309011}};


void Otk_Draw_Circle( OtkWidget tmppt )
{
 float VRT[2][3], tcolor[4], x, y, a, b, rx, ry;
 int k;

 for (k=0; k<4; k++) {tcolor[k] = tmppt->color[k];}
 glColor4fv( tcolor );
 glLineWidth( tmppt->thickness );
 glBegin( GL_LINES );
 VRT[0][2] = tmppt->z;  VRT[1][2] = tmppt->z;
 a = 1.0;  b = 0.0;     k = 0;
 rx = 0.5 * (tmppt->xright - tmppt->xleft);
 ry = 0.5 * (tmppt->ybottom - tmppt->ytop);
 x = tmppt->xleft + rx;
 y = tmppt->ytop + ry;
 VRT[1][0] = x + rx;
 VRT[1][1] = - y;
 while (k < 21)
  {
   VRT[0][0] = VRT[1][0];   VRT[0][1] = VRT[1][1];
   glVertex3fv( VRT[0] );
   a = cosine_table[k][0];  b = cosine_table[k++][1];
   VRT[1][0] = x + rx * a;  VRT[1][1] = - (y + ry * b);
   glVertex3fv( VRT[1] );
  }
 glEnd(); 
}



OtkWidget Otk_MakeCircle( OtkWidget container, float x, float y, float radius, OtkColor circ_color, float thickness )
{
 OtkWidget tmpobj;

 tmpobj = Otk_add_object( Otk_class_circle, container );
 tmpobj->z = container->z + 0.5 * Otk_DZ;
 OtkTranslateColor( circ_color, tmpobj->color );
 tmpobj->thickness = thickness;
 tmpobj->x1 = x - radius;
 tmpobj->y1 = y - radius;
 tmpobj->x2 = x + radius;
 tmpobj->y2 = y + radius;
 tmpobj->xleft =   container->xleft + 0.01 * tmpobj->x1 * (container->xright - container->xleft);
 tmpobj->xright =  container->xleft + 0.01 * tmpobj->x2 *(container->xright - container->xleft);   
 tmpobj->ytop =    container->ytop + 0.01 * tmpobj->y1 * (container->ybottom - container->ytop);
 tmpobj->ybottom = container->ytop + 0.01 * tmpobj->y2 * (container->ybottom - container->ytop);   
 return tmpobj;
}


void Otk_Draw_Disk( OtkWidget tmppt )
{
#if (1)
 float VRT[3][3], tcolor[4], phi=0.0, dphi=3.14159/10.0, a, b, c, rx, ry;
 int k, m;

 // glEnable( GL_BLEND );
 glBegin( GL_TRIANGLES );
 if (tmppt->object_subtype != Otk_subtype_raised)
  { /* Flat disk. */
   for (k=0; k<4; k++) {tcolor[k] = tmppt->color[k];}
   glColor4fv( tcolor );
   VRT[0][0] = tmppt->xleft;    VRT[0][1] = - tmppt->ytop;      VRT[0][2] = tmppt->z;
   VRT[1][2] = tmppt->z;  VRT[2][2] = tmppt->z;
   k = 0;
   rx = tmppt->xright - tmppt->xleft;
   ry = tmppt->ybottom - tmppt->ytop;
   VRT[2][0] = tmppt->xleft + rx;
   VRT[2][1] = - tmppt->ytop;
   while (k < 21)
    {
     VRT[1][0] = VRT[2][0];    	VRT[1][1] = VRT[2][1];
     // phi = phi + dphi;
     // a = cosine_table[k][0];  b = cosine_table[k++][1];
     VRT[2][0] = tmppt->xleft + rx * cosine_table[k][0];
     VRT[2][1] = - (tmppt->ytop + ry * cosine_table[k++][1]);
     Otk_Triangle( VRT[0], VRT[2], VRT[1] );
    }
   glEnd(); 
  }
 else
  { /* Raised, make a slight reflection. */
   VRT[0][0] = tmppt->xleft;      VRT[0][1] = - tmppt->ytop;              VRT[0][2] = tmppt->z;
   VRT[1][2] = tmppt->z;  VRT[2][2] = tmppt->z;
   a = 1.0;  b = 0.0;     k = 0;
   rx = tmppt->xright - tmppt->xleft;
   ry = tmppt->ybottom - tmppt->ytop;
   VRT[2][0] = tmppt->xleft + rx * a;
   VRT[2][1] = - (tmppt->ytop + rx * b);
   while (phi < 2.0 * 3.14159 + 0.05 * dphi)
    {
     if ((phi>= 0.9 * 3.14159) && (phi<= 1.5 * 3.14159))
      {
 	c = 0.30 * (3.0 - fabs(phi-1.2*3.14159));
	for (m=0; m<4; m++) {tcolor[m] = c + tmppt->color[m];  if (tcolor[m]>1.0) tcolor[m] = 1.0;}
      }
     else
      for (m=0; m<4; m++) {tcolor[m] = tmppt->color[m];}
     glColor4fv( tcolor );
     VRT[1][0] = VRT[2][0];    		  VRT[1][1] = VRT[2][1];
     phi = phi + dphi;
     a = cosine_table[k][0];  b = cosine_table[k++][1];
     VRT[2][0] = tmppt->xleft + rx * a;	  VRT[2][1] = - (tmppt->ytop + ry * b);
     Otk_Triangle( VRT[0], VRT[2], VRT[1] );
    }
   glEnd(); 
  }
 // glDisable( GL_BLEND );

#else

 float VRT[3][3], tcolor[4], phi=0.0, dphi=3.14159/10.0, a, b, rx, ry, tvec[3]={0.0,0.0,1.0};
 int k;

 glBegin( GL_TRIANGLE_FAN );
 for (k=0; k<4; k++) {tcolor[k] = tmppt->color[k];}
 glColor4fv( tcolor );

 rx = tmppt->xright - tmppt->xleft;
 ry = tmppt->ybottom - tmppt->ytop;
 a = 1.0;  b = 0.0;
 k = 0;

 VRT[0][0] = tmppt->xleft;
 VRT[0][1] = - tmppt->ytop;
 VRT[0][2] = tmppt->z;

 VRT[1][0] = tmppt->xleft + rx * a;
 VRT[1][1] = -(tmppt->ytop + ry * b);
 VRT[1][2] = tmppt->z;

 // phi = phi + dphi;
 // a = cosine_table[k][0];  b = cosine_table[k++][1];
 VRT[2][0] = tmppt->xleft + rx * cosine_table[k][0];
 VRT[2][1] = -(tmppt->ytop + ry * cosine_table[k++][1]);
 VRT[2][2] = tmppt->z;

 glNormal3fv( tvec );
 glVertex3fv( VRT[0] );
 glVertex3fv( VRT[1] );
 glVertex3fv( VRT[2] );
 while (k < 21)
  {
   // phi = phi + dphi;
   // a = cos(phi);	b = sin(phi);
   // a = cosine_table[k][0];  b = cosine_table[k++][1];
   VRT[2][0] = tmppt->xleft + rx * cosine_table[k][0];
   VRT[2][1] = - (tmppt->ytop + ry * cosine_table[k++][1]);
   glVertex3fv( VRT[2] );
  }
 glEnd(); 
#endif
}


OtkWidget Otk_MakeDisk( OtkWidget container, float x, float y, float radius, OtkColor disk_color )
{
 OtkWidget tmpobj;

 tmpobj = Otk_add_object( Otk_class_disk, container );
 tmpobj->z = container->z + 0.5 * Otk_DZ;
 OtkTranslateColor( disk_color, tmpobj->color );
 tmpobj->x1 = x;
 tmpobj->y1 = y;
 tmpobj->x2 = x + radius;
 tmpobj->y2 = y + radius;
 tmpobj->xleft =   container->xleft + 0.01 * x * (container->xright - container->xleft);
 tmpobj->xright =  container->xleft + 0.01 * tmpobj->x2 *(container->xright - container->xleft);   
 tmpobj->ytop =    container->ytop + 0.01 * y * (container->ybottom - container->ytop);
 tmpobj->ybottom = container->ytop + 0.01 * tmpobj->y2 * (container->ybottom - container->ytop);   
 return tmpobj;
}



void Otk_render_image_rectangle( OtkWidget tmppt, float xleft, float ytop, float xright, float ybottom )
{ /* Internal routine called only by Otk_Draw_Panel.  Places image into framebuffer. */
 int k;
 float vrtx[4]={0.0,0.0,1.0,1.0}, vrty[4]={0.0,1.0,1.0,0.0};

   if (tmppt->image == 0) return;
   glEnable( GL_TEXTURE_2D );
   glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
   glCallList( tmppt->image->calllist_num );  
   glBegin( GL_QUADS );
   if (tmppt->font != 0)	/* For images, font field can hold sub-picture region. */
    { float *ptr;
      ptr = (float *)(tmppt->font);
      for (k=0; k<4; k++) vrtx[k] = ptr[k];
      for (k=0; k<4; k++) vrty[k] = ptr[k+4];
    }
   switch ((int)(tmppt->slant))
     {
      case 1:	/* Flip LR. */
	vrtx[0] = 1.0;  vrtx[1] = 1.0;  vrtx[2] = 0.0;  vrtx[3] = 0.0;
	// vrty[0] = 0.0;  vrty[1] = 1.0;  vrty[2] = 1.0;  vrty[3] = 0.0;
	break;
      case 2:	/* Flip TB. */
	// vrtx[0] = 0.0;  vrtx[1] = 0.0;  vrtx[2] = 1.0;  vrtx[3] = 1.0;
	vrty[0] = 1.0;  vrty[1] = 0.0;  vrty[2] = 0.0;  vrty[3] = 1.0;
	break;
      case 3:	/* Flip LR+TB. */
	vrtx[0] = 1.0;  vrtx[1] = 1.0;  vrtx[2] = 0.0;  vrtx[3] = 0.0;
	vrty[0] = 1.0;  vrty[1] = 0.0;  vrty[2] = 0.0;  vrty[3] = 1.0;
	break;
      case 4:	/* Rotate 90-deg. */
	vrtx[0] = 1.0;  vrtx[1] = 0.0;  vrtx[2] = 0.0;  vrtx[3] = 1.0;
	vrty[0] = 0.0;  vrty[1] = 0.0;  vrty[2] = 1.0;  vrty[3] = 1.0;
	break;
      case 5:	/* Rotated + Flipped LR. */
	vrtx[0] = 0.0;  vrtx[1] = 1.0;  vrtx[2] = 1.0;  vrtx[3] = 0.0;
	vrty[0] = 0.0;  vrty[1] = 0.0;  vrty[2] = 1.0;  vrty[3] = 1.0;
	break;
      case 6:	/* Rotated + Flipped TB. */
	vrtx[0] = 1.0;  vrtx[1] = 0.0;  vrtx[2] = 0.0;  vrtx[3] = 1.0;
	vrty[0] = 1.0;  vrty[1] = 1.0;  vrty[2] = 0.0;  vrty[3] = 0.0;
	break;
      case 7:	/* Rotated + Flipped LR+TB. */
	vrtx[0] = 0.0;  vrtx[1] = 1.0;  vrtx[2] = 1.0;  vrtx[3] = 0.0;
	vrty[0] = 1.0;  vrty[1] = 1.0;  vrty[2] = 0.0;  vrty[3] = 0.0;
	break;
     }
    glTexCoord2f( vrtx[0], vrty[0] );
    glVertex3f( xleft, -ytop, tmppt->z );
    glTexCoord2f( vrtx[1], vrty[1] );
    glVertex3f( xleft, -ybottom, tmppt->z );
    glTexCoord2f( vrtx[2], vrty[2] );
    glVertex3f( xright, -ybottom, tmppt->z );
    glTexCoord2f( vrtx[3], vrty[3] );
    glVertex3f( xright, -ytop, tmppt->z );
   glEnd();
   glDisable( GL_TEXTURE_2D );
}


void Otk_Draw_Panel( OtkWidget tmppt )
{
 float VRT[8][3], tcolor[4], xdelta, ydelta, aspect, yd2, hdelta=0.0, vdelta=0.0, inc=1.0, dec=1.0, cdelta, border;
 float bwdth, bhght;
 int k, objectkind;

 if (tmppt->object_subtype == Otk_ImagePanel)
  { 
   Otk_render_image_rectangle( tmppt, tmppt->xleft, tmppt->ytop, tmppt->xright, tmppt->ybottom );
   return;
  }
 bwdth = tmppt->xright - tmppt->xleft;
 bhght = tmppt->ybottom - tmppt->ytop;
 if (((tmppt->superclass==Otk_SC_RadioButton) || (tmppt->superclass==Otk_SC_ToggleButton)) && 
	((tmppt->object_subtype==Otk_subtype_raised_diamond) || (tmppt->object_subtype==Otk_subtype_recessed_diamond)))
   { /* Diamond */	/* Like Rectangular Panel, but rotated 45 degrees. */
    hdelta = 0.5 * bwdth;
    vdelta = 0.5 * bhght;
    dec = 0.0;   inc = 6.0;	/* Selectively zero-out (decrease) or increase the recessed/raised x or y offset. */
    objectkind = tmppt->object_subtype - 3;
   }
 else objectkind = tmppt->object_subtype;

  /* Define outer boundaries of panel. */
  VRT[0][0] = tmppt->xleft + hdelta;	/* Upper left. */
  VRT[0][1] = - tmppt->ytop;
  VRT[0][2] = tmppt->z;

  VRT[1][0] = tmppt->xright;	/* Upper right. */
  VRT[1][1] = - (tmppt->ytop + vdelta);
  VRT[1][2] = tmppt->z;

  VRT[2][0] = tmppt->xright - hdelta;	/* Lower right. */
  VRT[2][1] = - tmppt->ybottom;
  VRT[2][2] = tmppt->z;

  VRT[3][0] = tmppt->xleft;	/* Lower left. */
  VRT[3][1] = - (tmppt->ybottom - vdelta);
  VRT[3][2] = tmppt->z;

  if (objectkind != Otk_subtype_plane)	/* Prepare vertices for raised or lowered foreground/surface, for inner rectangle. */
   {
    aspect = (float)OtkWindowSizeX * bwdth / ((float)OtkWindowSizeY * bhght);
    aspect = sqrt(aspect);
    border = 0.04 * tmppt->thickness * Otk_border_thickness;
    xdelta = bwdth * border / aspect;
    ydelta = bhght * border * aspect;
    if ((objectkind == Otk_subtype_recessed) && (tmppt->superclass != Otk_SC_RadioButton) && (tmppt->superclass != Otk_SC_ToggleButton))
     yd2 = 1.5; else yd2 = 1.0;  /* Correction for bottom overhang when depressed. */
    VRT[4][0] = tmppt->xleft + xdelta * dec + hdelta;	/* Upper left. */
    VRT[4][1] = - (tmppt->ytop + ydelta * inc);
    VRT[4][2] = tmppt->z;

    VRT[5][0] = tmppt->xright - xdelta * inc;		/* Upper right. */
    VRT[5][1] = - (tmppt->ytop + ydelta * dec + vdelta);
    VRT[5][2] = tmppt->z;

    VRT[6][0] = tmppt->xright - xdelta * dec - hdelta;	/* Lower right. */
    VRT[6][1] = - (tmppt->ybottom - yd2 * ydelta * inc);
    VRT[6][2] = tmppt->z;

    VRT[7][0] = tmppt->xleft + xdelta * inc;			/* Lower left. */
    VRT[7][1] = - (tmppt->ybottom - yd2 * ydelta * dec - vdelta);
    VRT[7][2] = tmppt->z;
   }

  /* Place background/border. (triangle 1 - upper left) */
  if (tmppt->superclass > Otk_SC_WindowPane) cdelta = 0.25; else cdelta = 0.5;
  tcolor[3] = tmppt->color[3];
  switch (objectkind)
   {
    case Otk_subtype_plane:    glColor4fv( tmppt->color );  break;
    case Otk_subtype_raised:   for (k=0; k<3; k++) {tcolor[k] = 1.5 * tmppt->color[k];  if (tcolor[k]>1.0) tcolor[k] = 1.0; }  glColor4fv( tcolor );  break;
    case Otk_subtype_recessed: for (k=0; k<3; k++) {tcolor[k] = cdelta * tmppt->color[k];}  glColor4fv( tcolor );  break;
   }
  glBegin( GL_TRIANGLES );
  if (objectkind==Otk_subtype_plane)
   {
    Otk_Triangle( VRT[1], VRT[0], VRT[3] );
   }
  else
   {
    Otk_Triangle( VRT[0], VRT[3], VRT[7] );
    Otk_Triangle( VRT[0], VRT[7], VRT[5] );
    Otk_Triangle( VRT[0], VRT[5], VRT[1] );
   }

  /* Place background/border. (triangle 2 - lower right) */
  switch (objectkind)
   {
    case Otk_subtype_raised:   for (k=0; k<3; k++) {tcolor[k] = 0.5 * tmppt->color[k];  }  glColor4fv( tcolor );  break;
    case Otk_subtype_recessed: for (k=0; k<3; k++) {tcolor[k] = 1.5 * tmppt->color[k];  if (tcolor[k]>1.0) tcolor[k] = 1.0; }  glColor4fv( tcolor );  break;
   }
  if (objectkind==Otk_subtype_plane)
   {
    Otk_Triangle( VRT[1], VRT[3], VRT[2] );
   }
  else
   {
    Otk_Triangle( VRT[1], VRT[5], VRT[2] );
    Otk_Triangle( VRT[7], VRT[2], VRT[5] );
    Otk_Triangle( VRT[7], VRT[3], VRT[2] );
   }

  /* Place raised or lowered foreground/surface. */
  if (objectkind != Otk_subtype_plane)	/* Now draw the inner rectangle. */
   {
    if ((tmppt->image != 0) && (tmppt->superclass == Otk_SC_Button))
     { /* Display icon. */
	glEnd();
	inc = 0.02 * Otk_DZ;
	tmppt->z = tmppt->z + inc;
	Otk_render_image_rectangle( tmppt, VRT[4][0], -VRT[4][1], VRT[6][0], -VRT[7][1] );
	// Otk_render_image_rectangle( tmppt, tmppt->xleft, tmppt->ytop, tmppt->xright, tmppt->ybottom );
	tmppt->z = tmppt->z - inc;
     }
    else
     { /* Regular non-icon button surface. */
	if (((tmppt->superclass==Otk_SC_RadioButton) || (tmppt->superclass==Otk_SC_ToggleButton)) && 
                (tmppt->object_subtype!=Otk_subtype_recessed_diamond))
	 { for (k=0; k<3; k++) tcolor[k] = 0.5 * tmppt->color[k]; glColor4fv( tcolor ); }
	else
	if ((tmppt->superclass > Otk_SC_WindowPane) && (objectkind==Otk_subtype_recessed))
	 { for (k=0; k<3; k++) tcolor[k] = 0.75 * tmppt->color[k]; glColor4fv( tcolor ); }
	else glColor4fv( tmppt->color );
	VRT[4][2] = tmppt->z + 0.02 * Otk_DZ;
	VRT[5][2] = tmppt->z + 0.02 * Otk_DZ;
	VRT[6][2] = tmppt->z + 0.02 * Otk_DZ;
	VRT[7][2] = tmppt->z + 0.02 * Otk_DZ;
	Otk_Triangle( VRT[5], VRT[4], VRT[7] );
	Otk_Triangle( VRT[5], VRT[7], VRT[6] );
	glEnd();
     }
    /* Draw outline lines. */
    if (tmppt->outlinestyle!=0)
     {
	for (k=0; k<3; k++) tcolor[k] = 0.0;
	glColor4fv( tcolor );
	glLineWidth( 2.0 );
	glBegin( GL_LINES );
	for (k=0; k<4; k++) VRT[k][2] = VRT[k][2] + 0.01;
	glVertex3fv( VRT[0]);	glVertex3fv( VRT[1] );
	glVertex3fv( VRT[1] );	glVertex3fv( VRT[2] );
	glVertex3fv( VRT[2] );	glVertex3fv( VRT[3] );
	glVertex3fv( VRT[3] );	glVertex3fv( VRT[0] );
	glEnd(); 
     }
   }
  else glEnd(); 
}





void Otk_Draw_String( OtkWidget tmppt );
int otk_text_quality=0;


void Otk_Draw_Text( OtkWidget tmppt )
{
 int j;
 float dx, dy, x1, y1, x2, y2, z, origcolor[3];

 switch (otk_text_quality)
  {
   case 0:   Otk_Draw_String( tmppt );	break;
   case 1:
	// tmppt->color[3] = 0.9;
	// glEnable( GL_BLEND );
	for (j=0; j < 3; j++)
	 {
	  origcolor[j] = tmppt->color[j];
	  if (tmppt->color[j] >= 0.5) tmppt->color[j] = 0.6 * tmppt->color[j];
	  else tmppt->color[j] = 1.0 - 0.6 * (1.0 - tmppt->color[j]);
	 }
	dx = 30.0 / (float)OtkWindowSizeX;
	dy = 30.0 / (float)OtkWindowSizeY;
	x1 = tmppt->xleft;	x2 = tmppt->xright;
	y1 = tmppt->ytop;	y2 = tmppt->ybottom;
	tmppt->xleft = x1 - dx;	 tmppt->xright = x2 - dx;
	tmppt->ytop = y1 - dy;	 tmppt->ybottom = y2 - dy;
	z = tmppt->z;
	tmppt->z = z - 0.25;
	glPushMatrix();
	Otk_Draw_String( tmppt );
	glPopMatrix();
	tmppt->xleft = x1;	tmppt->xright = x2;
	tmppt->ytop = y1;	tmppt->ybottom = y2;
	for (j=0; j < 3; j++) tmppt->color[j] = origcolor[j];	
	tmppt->z = z + 0.25;
	Otk_Draw_String( tmppt );
	tmppt->z = z;
	// glDisable( GL_BLEND );
	break;
  }
}




void Otk_Get_String_Size( OtkWidget tmpobj, char *str, int len, int *nlen, float *width, float *height )
{
 OtkFont *font;
 char *end = str + (len > 0 ? len : strlen( str ));
 unsigned char chr;
 float dx, scale, nrmscale, lowscale, spcscale;
 float horz=0.0;
 int maxcol;

 nrmscale = (0.15 * 9.5) * tmpobj->scale * tmpobj->sqrtaspect;
 lowscale = 0.8 * nrmscale;
 spcscale = 0.9 * nrmscale;

 if ( tmpobj->parent )
   horz = tmpobj->parent->xright - tmpobj->parent->xleft;
 else
   nlen = NULL;

 if( tmpobj->font )
   font = tmpobj->font;
 else
  {
   if( !Otk_Default_Font )
     Otk_Default_Font = Otk_Build_Internal_Font(Otk_FontDefault, Otk_FontSpacingDefault );
   font = Otk_Default_Font;
  }

 if( font->glyph_defs )
  {
   *width = 0;
   while ( str < end && *str )
    {
     chr = *str;
     if( !font->glyph_defs[(unsigned int)chr] && islower( chr ) )
      {
       chr = toupper( chr );
       scale = lowscale;
      }
     else
      scale = nrmscale;
     if ( font->glyph_defs[(unsigned int)chr] )
       dx = font->glyph_defs[(unsigned int)chr]->x_adv * scale;
     else
       dx = spcscale;

     if ( nlen && *width + dx > horz )
      {
       *nlen = *nlen - (end - str);
       end = str;
      }
     else
      *width = *width + dx;
     str++;
    }
   *width = *width * 1.25;
  }
 else
  {
   maxcol = floor( (tmpobj->xright - tmpobj->xleft)/nrmscale );
   if ( nlen && len > maxcol )
    {
     *nlen = maxcol;
     *width = nrmscale * (float)(*nlen);
    }
   else
     *width = nrmscale * (float)len;
  }
 *height = (font->ascent - font->descent) * (0.12 * 15.0) * tmpobj->scale / tmpobj->sqrtaspect;
}



void Otk_Draw_String( OtkWidget tmppt )
{
 int k=0, row=0, col=0, len=0, len2=0;
 float x, y, v;
 char *eol;
 OtkFont *font;

 if ((tmppt->text==0) || (tmppt->text[0]=='\0')) return;

 if( tmppt->font )
   font = tmppt->font;
 else
  {
   if( !Otk_Default_Font )
     Otk_Default_Font = Otk_Build_Internal_Font(Otk_FontDefault, Otk_FontSpacingDefault);
   font = Otk_Default_Font;
  }

 // printf("Superclass=%d class=%d subtype=%d\n", tmppt->superclass, tmppt->object_class, tmppt->object_subtype );
 // printf("text='%s' horiztextscroll=%d\n", tmppt->text, tmppt->horiztextscroll);

 if ((tmppt->superclass == Otk_SC_FormBox) && (tmppt->nrows > 1))
 { /*multi-line_textedit_box*/ 
   float vscale, dv;

   // printf("\n\nnrows=%d ncols=%d, horiz_cscrol=%d  text='%s' \n", tmppt->nrows, tmppt->ncols, tmppt->horiztextscroll, tmppt->text );

   Otk_Get_String_Size( tmppt, tmppt->text, len, &len2, &x, &v );
   v = 0.4 * v;		/* Consider text height to be smaller than reported, to magnify it slightly. */
   // printf(" v = %g    y = v * %d - %g = %g = y\n", v, tmppt->verttextscroll, tmppt->ytop, v * (float)tmppt->verttextscroll - tmppt->ytop );

   dv = (tmppt->parent->ybottom - tmppt->parent->ytop) / (float)((tmppt->nrows) + 0.5);		/* dv = row-spacing in outer-window coords. */
   glTranslatef( tmppt->xleft, dv * (float)(tmppt->verttextscroll) - tmppt->ytop, tmppt->z );	/* Shift to initial position. */
   vscale = dv / v;
   glScalef( (0.15*9.5)*tmppt->scale * tmppt->sqrtaspect, vscale, 1.0 );


   Otk_letter_orientation = tmppt->outlinestyle;
   glColor4fv( tmppt->color );
   glLineWidth( tmppt->thickness );

   if( Otk_letter_orientation )
    glRotatef( 90.0, 0.0, 0.0, 1.0 );

   // printf("tmppt->ytop=%g tmppt->ybottom=%g, tmppt->parent->ytop=%g tmppt->parent->ybottom=%g\n", tmppt->ytop, tmppt->ybottom, tmppt->parent->ytop, tmppt->parent->ybottom );
   x = (tmppt->parent->ybottom - tmppt->parent->ytop) / ((float)(tmppt->nrows) + 0.5);

   if( tmppt->slant != 0.0 )
     Otk_glShear( 0, 0, -tmppt->slant, 0, 0, 0 ); 	/* negative slant to be compatible */

   /* Advance to initial row, if scrolled. */
   k = 0;
   while ((k < tmppt->horiztextscroll) && (tmppt->text[k] != '\0') && (tmppt->text[k]!='\n')) k++;

   glListBase( font->glyphs - font->start_glyph );
   while ((tmppt->text[k]!='\0') && (row < tmppt->nrows + tmppt->verttextscroll))
    {
     if (tmppt->text[k]=='\n')
      {
       row++;
       glTranslatef( 0.0, -v, 0.0 );	/* Shift amount is effectively v * (dv / v) = dv, */
       col = -1; 			/*  by virtue of glScalef(vscale), above. */
       do
	{ col++;  k++; }
       while ((tmppt->text[k]!='\0') && (tmppt->text[k]!='\n') && (col < tmppt->horiztextscroll));
      }
     else
      {
       eol = strchr( &tmppt->text[k], '\n' );
       if( eol )
        len = eol - &tmppt->text[k];
       else
        len = strlen( &tmppt->text[k] );
       len2 = len;
       if( len2 )
        {
         glPushMatrix();
         glPolygonOffset( 0, row );
         glCallLists( len, GL_BYTE, &tmppt->text[k] );
         glPopMatrix();
        }
       col += len2;
       k += len2;
       if( len2 < len ) { while ((tmppt->text[k]!='\0') && (tmppt->text[k]!='\n')) { k++;} }
    }
  }


 } /*multi-line_textedit_box*/ 
 else
 { /*textlabels_and_one-line_textformboxs*/

 v = (font->ascent - font->descent) * (0.12*15.0) * tmppt->scale / tmppt->sqrtaspect;
 y = v * (float)tmppt->verttextscroll - tmppt->ytop;
 Otk_letter_orientation = tmppt->outlinestyle;

 glColor4fv( tmppt->color );
 glLineWidth( tmppt->thickness );

 glTranslatef( tmppt->xleft, y, tmppt->z );
 if( Otk_letter_orientation )
   glRotatef( 90.0, 0.0, 0.0, 1.0 );
 glScalef( (0.15*9.5)*tmppt->scale * tmppt->sqrtaspect, (0.12*15.0)*tmppt->scale/tmppt->sqrtaspect, 1.0 );
 if( tmppt->slant != 0.0 )
   Otk_glShear( 0, 0, -tmppt->slant, 0, 0, 0 ); /* negative slant to be compatible */

 while ((k < tmppt->horiztextscroll) && (tmppt->text[k] != '\0') && (tmppt->text[k]!='\n')) k++;

 glListBase( font->glyphs - font->start_glyph );
 Otk_Get_String_Size( tmppt, &tmppt->text[k], len, &len2, &x, &v );

 while ((tmppt->text[k]!='\0') && (row < tmppt->nrows + tmppt->verttextscroll))
  {
   if (tmppt->text[k]=='\n')
    {
     row++;
     glTranslatef( 0.0, -0.4 * v, 0.0 );
     col = -1; 
     do
      {
       col++;
       k++;
      }
     while ((tmppt->text[k]!='\0') && (tmppt->text[k]!='\n') && (col < tmppt->horiztextscroll));
    }
   else
    {
     eol = strchr( &tmppt->text[k], '\n' );
     if( eol )
       len = eol - &tmppt->text[k];
     else
       len = strlen( &tmppt->text[k] );
     len2 = len;
     if( len2 )
      {
       glPushMatrix();
       glPolygonOffset( 0, row );
       glCallLists( len, GL_BYTE, &(tmppt->text[k]) );	/* Renders character-string beginning at text[k]. */
       glPopMatrix();
      }
     col = col + len2;
     k = k + len2;
     if ( len2 < len )
       while ((tmppt->text[k] != '\0') && (tmppt->text[k] != '\n'))
	 k++;
    }
  }

 } /*textlabels_and_one-line_textformboxs*/

 Otk_letter_orientation = 0;
}



OtkWidget OtkMakeTextLabel( OtkWidget container, char *text, OtkColor text_color, float scale, float thickness, float x, float y )
{
 OtkWidget tmpobj;

 tmpobj = Otk_add_object( Otk_SC_TextLabel, container );
 tmpobj->x1 = x;
 tmpobj->y1 = y;
 tmpobj->z = container->z + 0.2 * Otk_DZ;
 tmpobj->xleft =  container->xleft + tmpobj->x1 * (container->xright - container->xleft) * 0.01;
 tmpobj->ytop  =  container->ytop + tmpobj->y1 * (container->ybottom - container->ytop) * 0.01;
 OtkTranslateColor( text_color, tmpobj->color );
 tmpobj->text = (char *)strdup( text );
 tmpobj->scale = scale;
 tmpobj->thickness = thickness;
 tmpobj->slant = 0.0;
 tmpobj->outlinestyle = 0;
 return tmpobj;
}


void Otk_Modify_Text_Slant( OtkWidget tmpobj, float slant ) 	// { tmpobj->slant = -slant; }
{
 if (tmpobj->superclass == Otk_SC_FormBox)
  tmpobj->children->slant = -slant;
 else
  tmpobj->slant = -slant;
}


void Otk_Modify_Color( OtkWidget tmpobj, OtkColor color ) { OtkTranslateColor( color, tmpobj->color ); }
void Otk_Modify_Text_Color( OtkWidget tmpobj, OtkColor text_color ) { OtkTranslateColor( text_color, tmpobj->color ); }
void Otk_Modify_Text_Thickness( OtkWidget tmpobj, float thickness ) { tmpobj->thickness = thickness; }
void Otk_Modify_Text_Scale( OtkWidget tmpobj, float scale ) { tmpobj->scale = scale; }
void Otk_Modify_Text_Aspect( OtkWidget tmpobj, float aspect ) { tmpobj->sqrtaspect = sqrt(aspect); }
void Otk_Set_Text_Aspect( float aspect ) { Otk_sqrtaspect = sqrt(aspect); }

void Otk_Set_Text_Editable( OtkWidget tmpobj, int on_notoff )
{
 if (tmpobj->superclass != Otk_SC_FormBox)
  {printf("Otk_Set_Text_Editable called on non-formbox object.\n"); return;}
 if (on_notoff)
  tmpobj->mouse_sensitive = 2;
 else
  tmpobj->mouse_sensitive = 0;
}


void Otk_Modify_Text_Position( OtkWidget tmpobj, float x, float y ) 
{ tmpobj->x1 = x; tmpobj->y1 = y;
  if (tmpobj->parent!=0)
   { tmpobj->xleft =  tmpobj->parent->xleft + tmpobj->x1 * (tmpobj->parent->xright - tmpobj->parent->xleft) * 0.01;
     tmpobj->ytop  =  tmpobj->parent->ytop + tmpobj->y1 * (tmpobj->parent->ybottom - tmpobj->parent->ytop) * 0.01; 
   } else { tmpobj->xleft =  x;  tmpobj->ytop  =  y;  }
}


void Otk_Get_Character_Size( OtkWidget tmpobj, float *width, float *height )
{
 char *str = "X";

 if (tmpobj->object_class != Otk_class_text) printf("Otk Error: Otk_Get_Character_Size on non-text object (%d).\n", tmpobj->object_class);
 // *width = (9.5 /* + 7.0 */) * 0.15 * tmpobj->scale * tmpobj->sqrtaspect;
 // *height = 12.0 * 0.15 * tmpobj->scale / tmpobj->sqrtaspect;
 Otk_Get_String_Size( tmpobj, str, 1, NULL, width, height );
}


void Otk_Get_Text_Size( OtkWidget tmpobj, float *width, float *height )
{
 int len = strlen(tmpobj->text);

 if (tmpobj->object_class != Otk_class_text) printf("Otk Error: Otk_Get_Text_Size on non-text object (%d).\n", tmpobj->object_class);
 Otk_Get_String_Size( tmpobj, tmpobj->text, len, NULL, width, height ); 
}


void Otk_FitTextInPanel( OtkWidget txtobj )
{
 float w, h, horiz_size, vert_size;
 OtkWidget panel;

 if (txtobj->superclass!=Otk_SC_TextLabel) {printf("Warning: Otk_FitTextInPanel called on non-text object.\n"); return;}
 panel = txtobj->parent;
 // if (panel->object_class!=Otk_class_panel) {printf("Warning: Otk_FitTextInPanel text-object parent not panel.\n"); return;}
 txtobj->sqrtaspect = 1.0;
 txtobj->scale = 1.0;
 txtobj->x1 = 0.0;
 txtobj->xleft = panel->xleft;
 Otk_Get_Text_Size( txtobj, &w, &h );
 horiz_size = panel->xright - panel->xleft;
 vert_size = panel->ybottom - panel->ytop;
 txtobj->sqrtaspect = sqrt( (h * horiz_size) / (w * 0.8 * vert_size) );
 txtobj->scale = 0.925 * horiz_size / (w * txtobj->sqrtaspect);
}





OtkWidget OtkMakeTextFormBox( OtkWidget container, char *text, int ncols, 
				float left, float top, float horiz_size, float vert_size,
				void (*callback)(char *s, void *x), void *parameter )
{
 OtkWidget tmpobj, subwin;
 float w, h;

 tmpobj = OtkMakePanel( container, Otk_subtype_recessed, OtkSetColor(0.9, 0.9, 0.9), left,  top, horiz_size, vert_size );
 tmpobj->superclass = Otk_SC_FormBox;
 subwin = OtkMakeTextLabel( tmpobj, text, OtkSetColor(0.0, 0.0, 0.0), 1.0, 1.0, 3.0, 22.0 );
 if (!Otk_Vect_Font) Otk_Vect_Font = Otk_Build_Internal_Font(Otk_Font_Vect, Otk_FontSpacing_Mono);
 subwin->font = Otk_Vect_Font;

 subwin->superclass = Otk_SC_FormBox;
 subwin->scissor_to_parent = 1;
 Otk_Get_Character_Size( subwin, &w, &h );
 subwin->sqrtaspect = sqrt( ((tmpobj->xright - tmpobj->xleft) * h) / (((float)ncols+0.5) * w * 0.8 * (tmpobj->ybottom - tmpobj->ytop)) );
 subwin->scale = subwin->sqrtaspect * 0.8 * (tmpobj->ybottom - tmpobj->ytop) / h;
 subwin->nrows = 1;
 subwin->ncols = ncols;
 subwin->horiztextscroll = 0;
 subwin->verttextscroll = 0;
 tmpobj->mouse_sensitive = 2;
 tmpobj->functval1 = callback;
 tmpobj->callback_param = parameter;

 return tmpobj;  
}


void Otk_Modify_Text( OtkWidget tmpobj, char *text ) 
{
 switch (tmpobj->superclass)
  {
   case Otk_SC_FormBox:
   case Otk_SC_Button:   case Otk_SC_TabHandle:   case Otk_SC_Menu_DropDown_Button:
	tmpobj = tmpobj->children;
	break;
   case Otk_SC_Menu_Item:
	if (tmpobj->object_class != Otk_class_text) tmpobj = tmpobj->children;
	break;
  }
 free( tmpobj->text );
 tmpobj->text = strdup( text );
 Otk_Display_Changed++;
}


void Otk_Get_Text( OtkWidget tmpobj, char *text, int n ) 
{ 
 int j=0;

 switch (tmpobj->superclass)
  {
   case Otk_SC_FormBox:
   case Otk_SC_Button:   case Otk_SC_TabHandle:   case Otk_SC_Menu_DropDown_Button:
	tmpobj = tmpobj->children;
	break;
   case Otk_SC_Menu_Item:
	if (tmpobj->object_class != Otk_class_text) tmpobj = tmpobj->children;
	break;
  }
 do { text[j] = tmpobj->text[j]; j++; } while ((j<n) && (text[j-1]!='\0'));
 text[j-1] = '\0';
}


void Otk_Modify_FormText_Aspect( OtkWidget tmpobj, float aspect )	/* Deprecated. */
{float w, h;
 tmpobj->children->scale = 1.0;
 tmpobj->children->sqrtaspect = sqrt(aspect);
 Otk_Get_Text_Size( tmpobj->children, &w, &h );
 tmpobj->children->scale = 0.8 * (tmpobj->ybottom - tmpobj->ytop) / h; 
}



OtkWidget OtkMakeTextEditBox( OtkWidget container, char *text, int nrows, int ncols, 
				float left, float top, float horiz_size, float vert_size )
{
 OtkWidget tmpobj, subwin;
 float w, h;

 tmpobj = OtkMakePanel( container, Otk_subtype_recessed, OtkSetColor(0.9, 0.9, 0.9), left,  top, horiz_size, vert_size );
 tmpobj->superclass = Otk_SC_FormBox;
 subwin = OtkMakeTextLabel( tmpobj, text, OtkSetColor(0.0, 0.0, 0.0), 1.0, 1.0, 3.0, 4.0 * 100.0 / vert_size );
 subwin->superclass = Otk_SC_FormBox;
 subwin->scissor_to_parent = 1;

 if (!Otk_Vect_Font) Otk_Vect_Font = Otk_Build_Internal_Font(Otk_Font_Vect, Otk_FontSpacing_Mono);
 subwin->font = Otk_Vect_Font;

 Otk_Get_Character_Size( subwin, &w, &h );
 // printf("MakeTextEditBox Character height, h = %g\n", h );
 subwin->sqrtaspect = sqrt( ((tmpobj->xright - tmpobj->xleft) * (float)nrows * h) / (((float)ncols+0.5) * w * 0.8 * (tmpobj->ybottom - tmpobj->ytop)) );
 subwin->scale = subwin->sqrtaspect * 0.8 * (tmpobj->ybottom - tmpobj->ytop) / ((float)nrows * h);
 subwin->nrows = nrows;
 subwin->ncols = ncols;
 subwin->horiztextscroll = 0;
 subwin->verttextscroll = 0;
 tmpobj->mouse_sensitive = 2;
 tmpobj->functval1 = 0;
 tmpobj->callback_param = 0;

 return tmpobj;  
}


void Otk_Right_Justify_Formbox( OtkWidget formbox )
{
  char *tmpwrd;
  int k, m;

  if ((formbox->superclass != Otk_SC_FormBox) || (formbox->object_subtype != Otk_subtype_recessed))
   { printf("Otk_RightJustify_Formbox: Not called on formbox object.\n");  return; }
  formbox = formbox->children;
  k = formbox->ncols - strlen(formbox->text);
  if (k <= 0) return;
  tmpwrd = (char *)malloc( (formbox->ncols + 2) * sizeof(char) );
  for (m=0;  m < k;  m++) tmpwrd[m] = ' ';	/* Add k leading spaces. */
  strcpy( &(tmpwrd[k]), formbox->text );
  free( formbox->text );
  formbox->text = tmpwrd;
}


void Otk_Center_Justify_Formbox( OtkWidget formbox )
{
  char *tmpwrd;
  int k, m;

  if ((formbox->superclass != Otk_SC_FormBox) || (formbox->object_subtype != Otk_subtype_recessed))
   { printf("Otk_Center_Justify_Formbox: Not called on formbox object.\n");  return; }
  formbox = formbox->children;
  k = formbox->ncols - strlen(formbox->text);
  k = k >> 1;
  if (k <= 0) return;
  tmpwrd = (char *)malloc( (strlen(formbox->text) + k + 3) * sizeof(char) );
  for (m=0;  m < k;  m++) tmpwrd[m] = ' ';	/* Add k leading spaces. */
  strcpy( &(tmpwrd[k]), formbox->text );
  free( formbox->text );
  formbox->text = tmpwrd;
}



float otk_default_slider_width=3.0;

void Otk_Set_Default_Slider_Width( float x )
{
 otk_default_slider_width = x;
}


OtkWidget OtkMakeSliderHorizontal( OtkWidget container, float left, float top, float horiz_size, 
				void (*callback)(float v, void *x), void *parameter )
{
 OtkWidget tmpobj, subwin;

 /* Make the slider-slot. */
 tmpobj = OtkMakePanel( container, Otk_subtype_recessed, OtkSetColor(0.3, 0.3, 0.3), left,  top, horiz_size, otk_default_slider_width );
 tmpobj->superclass = Otk_SC_hSlider;
 tmpobj->mouse_sensitive = mouse_groove_click;
 /* Make the slider-knob. */
 subwin = OtkMakePanel( tmpobj, Otk_subtype_raised, OtkSetColor(0.6, 0.6, 0.6), 46.0,  -16.0, 8.0, 132.0 );
 subwin->superclass = Otk_SC_hSlider;
 subwin->z = container->z + 0.8 * Otk_DZ;

 subwin->mouse_sensitive = 3;	/* Horizontal slider. */
 tmpobj->functval2 = callback;
 tmpobj->callback_param = parameter;
 return tmpobj;
}


OtkWidget OtkMakeSliderVertical( OtkWidget container, float left, float top, float vertical_size, 
				void (*callback)(float v, void *x), void *parameter )
{
 OtkWidget tmpobj, subwin;

 /* Make the slider-slot. */
 tmpobj = OtkMakePanel( container, Otk_subtype_raised, OtkSetColor(0.3, 0.3, 0.3), left,  top, otk_default_slider_width, vertical_size );
 tmpobj->superclass = Otk_SC_vSlider;
 tmpobj->mouse_sensitive = mouse_groove_click;
 /* Make the slider-knob. */
 subwin = OtkMakePanel( tmpobj, Otk_subtype_raised, OtkSetColor(0.6, 0.6, 0.6), -16.0, 46.0,  132.0, 8.0 );
 subwin->superclass = Otk_SC_vSlider;
 subwin->z = container->z + 2.0 * Otk_DZ;

 subwin->mouse_sensitive = 4;	/* Vertical slider. */
 tmpobj->functval2 = callback;
 tmpobj->callback_param = parameter;
 return tmpobj;
}


void Otk_SetSlider( OtkWidget slider, float position, float sz )	/* Position is range 0.0 to 1.0.  Sz is knob scale factor relative 1.0. */
{									/* Deprecated in favovr of Otk_SetSliderKnob(...) */
 float dx, dy;
 OtkWidget groove, knob;

 groove = slider;
 knob = slider->children;
 if (groove->superclass==Otk_SC_vSlider)
  {
   dy = sz * (knob->ybottom - knob->ytop);
   knob->y1 = (100.0 - dy) * position;
   knob->y2 =  knob->y1 + dy;
   knob->ytop = groove->ytop + position * (groove->ybottom - dy - groove->ytop);
   knob->ybottom = groove->ytop + dy + position * (groove->ybottom - dy - groove->ytop);
  } else
 if (groove->superclass==Otk_SC_hSlider)
  {
   dx = sz * (knob->xright - knob->xleft);
   knob->x1 = (100.0 - dx) * position;
   knob->x2 =  knob->x1 + dx;
   knob->xleft = groove->xleft + position * (groove->xright - dx - groove->xleft);
   knob->xright = groove->xleft + dx + position * (groove->xright - dx - groove->xleft);
  }
 else printf("Otk_SetSlider: Wrong object class %d\n", slider->superclass);
}


void Otk_SetSliderKnob( OtkWidget slider, float position, float hsz, float vsz )	/* Position is range 0.0 to 1.0.  Sz is knob scale factor relative 1.0. */
{
 float dx, dy, hpos, vpos;
 OtkWidget groove, knob;

 groove = slider;
 knob = slider->children;
 if (groove->superclass==Otk_SC_vSlider)
  {
    vpos = position;
    hpos = 0.5;
  } else
 if (groove->superclass==Otk_SC_hSlider)
  {
    vpos = 0.5;
    hpos = position;
  }
 else { printf("Otk_SetSliderKnob: Wrong object class %d\n", slider->superclass);  return; }
   
 dy = vsz * (knob->ybottom - knob->ytop);
 knob->y1 = (100.0 - dy) * vpos;
 knob->y2 =  knob->y1 + dy;
 knob->ytop = groove->ytop + vpos * (groove->ybottom - dy - groove->ytop);
 knob->ybottom = groove->ytop + dy + vpos * (groove->ybottom - dy - groove->ytop);

 dx = hsz * (knob->xright - knob->xleft);
 knob->x1 = (100.0 - dx) * hpos;
 knob->x2 =  knob->x1 + dx;
 knob->xleft = groove->xleft + hpos * (groove->xright - dx - groove->xleft);
 knob->xright = groove->xleft + dx + hpos * (groove->xright - dx - groove->xleft);
}


void Otk_SetSliderVertical( OtkWidget slider, float position, float sz )
{	/* Deprecated in favor of the more general "Otk_SetSliderKnob()". */
 Otk_SetSlider( slider, position, sz );
}


void Otk_SetSlider_Thickness( OtkWidget slider, float thickness )	/* Thickness factor is relative 1.0. */
{
 float dx, dy, center;
 OtkWidget groove, knob;

 groove = slider;
 knob = slider->children;
 if (groove->superclass==Otk_SC_vSlider)
  {
   dx = thickness * (groove->x2 - groove->x1);
   groove->x2 = groove->x1 + dx;
   center = 0.5 * (groove->x1 + groove->x2);
   dx = thickness * (knob->x2 - knob->x1);
   knob->x1 = center - 0.5 * dx;
   knob->x2 = center + 0.5 * dx;
   dx = thickness * (groove->xright - groove->xleft);
   groove->xright = groove->xleft + dx;
   center = 0.5 * (groove->xleft + groove->xright);
   dx = thickness * (knob->xright - knob->xleft);
   knob->xleft= center - 0.5 * dx;
   knob->xright = center + 0.5 * dx;
  } else
 if (groove->superclass==Otk_SC_hSlider)
  {
   dy = thickness * (groove->y2 - groove->y1);
   groove->y2 = groove->y1 + 0.5 * dy;
   center = 0.5 * (groove->y1 + groove->y2);
   dy = thickness * (knob->y2 - knob->y1);
   knob->y1 = center - 0.5 * dy;
   knob->y2 = center + 0.5 * dy;
   dy = thickness * (groove->ybottom - groove->ytop);
   groove->ybottom = groove->ytop + dy;
   center = 0.5 * (groove->ytop + groove->ybottom);
   dy = thickness * (knob->ybottom - knob->ytop);
   knob->ytop = center - 0.5 * dy;
   knob->ybottom = center + 0.5 * dy;
  }
 else printf("Otk_SetSlider_Thickness: Wrong object class %d\n", slider->superclass);
}


float Otk_GetSlider( OtkWidget slider )	/* Returns slider position in range 0 to 1.0. */
{
 if (slider->superclass==Otk_SC_vSlider)
  {
   slider = slider->children;	/* Access the slider-knob. */
   return slider->y1 / (100.0 - (slider->y2 - slider->y1));
  }
 else
 if (slider->superclass==Otk_SC_hSlider)
  {
   slider = slider->children;	/* Access the slider-knob. */
   return slider->x1 / (100.0 - (slider->x2 - slider->x1));
  }
 else { printf("Otk_GetSlider: Wrong object class %d\n", slider->superclass);  return 0; }
}



void Otk_scrolltext( float v, void *x)
{
 printf("Scroll text\n");
}


void OtkAddTextScrollbar( OtkWidget container, float width )
{
 OtkWidget tmpobj, subwin;

 if (container->superclass != Otk_SC_FormBox)
  {printf("OtkAddTextSlider: Not child of text-edit-box (but %d).\n", container->superclass); return;}

 if (width<=0.0) width = 4.0;
 tmpobj = OtkMakePanel( container, Otk_subtype_recessed, OtkSetColor(0.3, 0.3, 0.3), 101.0,  0.0, width, 100.0 );
 tmpobj->superclass = Otk_SC_textscrollbar;
 tmpobj->z = container->z;
 subwin = OtkMakePanel( tmpobj, Otk_subtype_raised, OtkSetColor(0.6, 0.6, 0.6), 5, 5,  90, 95.0 );
 subwin->superclass = Otk_SC_textscrollbar;
 subwin->z = container->z + 0.3 * Otk_DZ;

 subwin->mouse_sensitive = 5;	/* Text Scroll Bar. */
 tmpobj->functval2 = Otk_scrolltext;
 tmpobj->callback_param = tmpobj;
}


/********************************************************************************/


OtkWidget Otk_Make_Menu( OtkWidget container, float left, float top, float horiz_size, float vert_size, char *text )
{
 OtkWidget tmpobj;
 float w, h;

 tmpobj = OtkMakePanel( container, Otk_subtype_plane, Otk_Default_Button_Color, left, top, horiz_size, vert_size );
 tmpobj->superclass = Otk_SC_Menu_DropDown_Button;
 tmpobj->object_class = Otk_class_panel;
 tmpobj->mouse_sensitive = 1;

 OtkMakeTextLabel( tmpobj, text, OtkSetColor(0.0, 0.0, 0.0), 1.0, 1.0, 7.0, 20.0 );
 Otk_Get_Text_Size( tmpobj->children, &w, &h );
 horiz_size = tmpobj->xright - tmpobj->xleft;
 vert_size = tmpobj->ybottom - tmpobj->ytop;
 tmpobj->children->sqrtaspect = sqrt( (h * horiz_size) / (w * 0.8 * vert_size) );
 tmpobj->children->scale = 0.925 * horiz_size / (w * tmpobj->children->sqrtaspect);
 return tmpobj;
}


OtkWidget Otk_Add_Menu_Item( OtkWidget container, char *text, void (*callback)(void *x), void *parameter )
{
 OtkWidget tmpobj;

 if (container->superclass == Otk_SC_Menu_Item) container = container->parent;
 if (container->children->nxt == 0)
  { /* Add background panel iff first item. */
    tmpobj = OtkMakePanel( container, Otk_subtype_plane, Otk_Default_Button_Color, 
			container->xleft, container->ybottom, container->xright - container->xleft, container->ybottom - container->ytop);
    Otk_object_detach( tmpobj );	/* Remove from drawable list, until needed. */
    Otk_object_attach_hidden( container, tmpobj );
  }
 tmpobj = OtkMakePanel( container, Otk_subtype_raised, Otk_Default_Button_Color, 
			container->xleft, container->ybottom, container->xright - container->xleft, container->ybottom - container->ytop );
 Otk_object_detach( tmpobj );	/* Remove from drawable list, until needed. */
 Otk_object_attach_hidden( container, tmpobj );
 tmpobj->superclass = Otk_SC_Menu_Item;
 tmpobj->object_class = Otk_class_panel;
 tmpobj->mouse_sensitive = 1;
 tmpobj->callback = callback;	/* If callback is null, then open down new menu-list populated by my children, which are not normally drawn. */
 tmpobj->callback_param = parameter;

 OtkMakeTextLabel( tmpobj, text, OtkSetColor(0.0, 0.0, 0.0), 1.0, 1.0, 7.0, 20.0 );
 tmpobj->children->superclass = Otk_SC_Menu_Item;
 tmpobj->children->sqrtaspect = container->children->sqrtaspect;
 tmpobj->children->scale = container->children->scale;
 // printf("MenuItem '%s' sqrtaspect = %g  scale = %g\n", text, tmpobj->children->sqrtaspect, tmpobj->children->scale);
 return tmpobj;
}


OtkWidget Otk_Add_SubMenu( OtkWidget container, char *text )
{
 OtkWidget tmpobj, label;

 if (container->superclass == Otk_SC_Menu_Item) container = container->parent;
 if (container->children->nxt == 0)
  { /* Add background panel iff first item. */
    tmpobj = OtkMakePanel( container, Otk_subtype_plane, Otk_Default_Button_Color, 
			container->xleft, container->ybottom, container->xright - container->xleft, container->ybottom - container->ytop);
    Otk_object_detach( tmpobj );	/* Remove from drawable list, until needed. */
    Otk_object_attach_hidden( container, tmpobj );
  }
 tmpobj = OtkMakePanel( container, Otk_subtype_plane, Otk_Default_Button_Color, 
			container->xleft, container->ybottom, container->xright - container->xleft, container->ybottom - container->ytop);
 Otk_object_detach( tmpobj );	/* Remove from drawable list, until needed. */
 Otk_object_attach_hidden( container, tmpobj );
 // printf("While adding submenu '%s': Attaching to container %x hidden_children %x, not hidden_list = %x\n", text, container, tmpobj, container->hidden_children );
 tmpobj->superclass = Otk_SC_Menu_DropDown_Button;
 tmpobj->object_class = Otk_class_panel;
 tmpobj->mouse_sensitive = 1;

 label = OtkMakeTextLabel( tmpobj, text, OtkSetColor(0.0, 0.0, 0.0), 1.0, 1.0, 7.0, 20.0 );
 label->superclass = Otk_SC_Menu_Submenu;
 label->sqrtaspect =  container->children->sqrtaspect;
 label->scale = container->children->scale;

 label = OtkMakeTextLabel( label, ">", OtkSetColor(0.0, 0.0, 0.0), 1.0, 1.0, 95.0, 20.0 );
 label->superclass = Otk_SC_Menu_Submenu;
 label->sqrtaspect =  container->children->sqrtaspect;
 label->scale = container->children->scale;

 // printf("SubMenu '%s'\n", text );
 return tmpobj;
}


void Otk_Set_Menu_Selectable( OtkWidget tmpobj, int on_notoff )
{
 if ((tmpobj->superclass != Otk_SC_Menu_DropDown_Button) && (tmpobj->superclass != Otk_SC_Menu_Item))
  {printf("Otk_Set_Menu_Selectable called on non-menu object.\n"); return;}
 if (on_notoff)
  tmpobj->mouse_sensitive = 1;
 else
  tmpobj->mouse_sensitive = 0;
 tmpobj->children->color[0] = 0.4;
 tmpobj->children->color[1] = 0.4;
 tmpobj->children->color[2] = 0.4;
}


/********************************************************************************/

OtkWidget Otk_Selected_Item=0;


OtkWidget Otk_Make_Selection_List( OtkWidget container, int rows, int cols, 
				   float left, float top, float horiz_size, float vert_size )
{
 OtkWidget tmpobj;

 /* Make a panel for the file-list. */
 tmpobj = OtkMakePanel( container, Otk_subtype_plane, Otk_White, left, top, horiz_size, vert_size );
 tmpobj->superclass = Otk_SC_Select_List;
 tmpobj->object_class = Otk_class_panel;
 tmpobj->mouse_sensitive = 0;
 tmpobj->nrows = rows;
 tmpobj->ncols = cols;
 tmpobj->nentries = 0;
 tmpobj->verttextscroll = -1;	/* -1 = no scroll-bar, 1=scroll-bar, -2=master-scroller pre-scrollbar, 2=master-scroller w/scrollbar, 3=subordinate-scroller. */
 tmpobj->callback = 0;
 tmpobj->callback_param = 0;
 tmpobj->thickness = 3.0;

 tmpobj->sqrtaspect = 2.6 * (float)rows * horiz_size / ((float)cols * vert_size);
 Otk_Frame_Selection_List(tmpobj);
 return tmpobj;
}


void Otk_Frame_Selection_List( OtkWidget container )
{ OtkWidget b;
 /* Place a border around the list. */
 b = Otk_Add_BoundingBox( container, Otk_Black, 2.0, 0, 0, 100, 100 );
 b->z = b->z + 1.5 * Otk_DZ;
}

void Otk_Frame_Item( OtkWidget container, OtkWidget item, float thickness )
{ OtkWidget b;
 /* Place a border around the list. */
 b = Otk_Add_BoundingBox( container, Otk_Black, thickness, item->x1, item->y1, item->x2, item->y2 );
 b->z = b->z + 1.5 * Otk_DZ;
}




void Otk_scroll_object( OtkWidget container, float x, float y )
{
 OtkWidget tcpt1, tcpt2, unhidem = NULL;
 float dy, dx;
 int detached;

 dx = (x - container->xscroll) * (container->xright - container->xleft);
 dy = (y - container->yscroll) * (container->ybottom - container->ytop);
 container->xscroll = x;
 container->yscroll = y;

 if ( container->hidden_children )
  { /*hiddenkids*/
   // adjust items from the head of the hidden list
   tcpt1 = container->hidden_children;
   do
    {
     detached = 0;
     if ( tcpt1->scissor_to_parent )
      {
       Otk_position_object( tcpt1, 0, dx, dy );
       if ( (tcpt1->y1 < 100.0) && (tcpt1->y2 > 0.0) && (tcpt1->x1 < 100.0) && (tcpt1->x2 > 0.0) )
	{
	 detached = 1;
	 container->hidden_children = tcpt1->nxt;
	 tcpt1->nxt = unhidem;
	 unhidem = tcpt1;
	 tcpt1 = container->hidden_children;
	}
      }
    }
   while ( tcpt1 && detached );

   if ( tcpt1 )
    {
     while ( tcpt1->nxt )
      {
       if ( tcpt1->nxt->scissor_to_parent )
	{
	 Otk_position_object( tcpt1->nxt, 0, dx, dy );
	 if ( (tcpt1->nxt->y1 < 100.0) && (tcpt1->nxt->y2 > 0.0) && (tcpt1->nxt->x1 < 100.0) && (tcpt1->nxt->x2 > 0.0) )
	  {
	   tcpt2 = tcpt1->nxt;
	   tcpt1->nxt = tcpt2->nxt;
	   tcpt2->nxt = unhidem;
	   unhidem = tcpt2;
	  }
	 else
	   tcpt1 = tcpt1->nxt;
	}
       else
	 tcpt1 = tcpt1->nxt;
      }
    }
  } /*hiddenkids*/

 if ( container->children )
  { /*visiblekids*/
   tcpt1 = container->children;
   do
    {
     detached = 0;
     if ( tcpt1->scissor_to_parent )
      {
       Otk_position_object( tcpt1, 0, dx, dy );
       if ( (tcpt1->y1 > 100.0) || (tcpt1->y2 < 0.0) || (tcpt1->x1 > 100.0) || (tcpt1->x2 < 0.0) )
        {
	 detached = 1;
	 container->children = tcpt1->nxt;
	 tcpt1->nxt = container->hidden_children;
	 container->hidden_children = tcpt1;
	 tcpt1 = container->children;
	}
      }
    }
   while ( tcpt1 && detached );

   if ( tcpt1 )
    {
     while ( tcpt1->nxt )
      {
       if ( tcpt1->nxt->scissor_to_parent )
	{
	 Otk_position_object( tcpt1->nxt, 0, dx, dy );
	 if ( (tcpt1->nxt->y1 > 100.0) || (tcpt1->nxt->y2 < 0.0) || (tcpt1->nxt->x1 > 100.0) || (tcpt1->nxt->x2 < 0.0) )
	  {
	   tcpt2 = tcpt1->nxt;
	   tcpt1->nxt = tcpt2->nxt;
	   tcpt2->nxt = container->hidden_children;
	   container->hidden_children = tcpt2;
	  }
	 else
	   tcpt1 = tcpt1->nxt;
	}
       else
	 tcpt1 = tcpt1->nxt;
      }
     tcpt1->nxt = unhidem;
    }
   else
    {
     container->children = unhidem;
    }
  } /*visiblekids*/
 else
  { /*nokids*/
   container->children = unhidem;
  } /*nokids*/
}




void Otk_scrolllist( float v, void *x )
{
 OtkWidget container, tcpt1, tcpt2, tcpt3;
 float dy, y1, ctht, vdy;

 container = (OtkWidget)x;
 dy = ((float)(container->nentries - container->nrows) + 0.6) / (float)(container->nrows);
 ctht = (container->ybottom - container->ytop) * 0.01;
 vdy = - 100.0 * v * dy;

 /* First check the hidden children. */
 tcpt1 = container->hidden_children;
 while (tcpt1!=0)
  {
   if (tcpt1->superclass==Otk_SC_Select_List_Item)
    {
     tcpt1->ytop =    container->ytop + (tcpt1->y1 + vdy) * ctht;
     tcpt1->ybottom = container->ytop + (tcpt1->y2 + vdy) * ctht;
     if ((tcpt1->ytop >= container->ytop) && (tcpt1->ybottom <= container->ybottom))
      {
       tcpt3 = tcpt1;
       tcpt1 = tcpt1->nxt;
       Otk_object_detach_hidden( tcpt3 );
       Otk_object_attach( container, tcpt3 );
       tcpt2 = tcpt3->children;
       y1 = tcpt3->y1 + 0.01 * tcpt2->y1 * (tcpt3->y2 - tcpt3->y1);
       tcpt2->ytop =    container->ytop + (y1 + vdy) * ctht;
       y1 = tcpt3->y1 + 0.8 * (tcpt3->y2 - tcpt3->y1);
       tcpt2->ybottom = container->ytop + (y1 + vdy) * ctht;
       /* MUST RECOMPUTE (UPDATE) THE X-VALUES !!! */
       tcpt3->xleft =  container->xleft + tcpt3->x1 * (container->xright - container->xleft) * 0.01;
       tcpt3->xright = container->xleft + tcpt3->x2 * (container->xright - container->xleft) * 0.01;   
       tcpt2->xleft =  tcpt3->xleft + tcpt2->x1 * (tcpt3->xright - tcpt3->xleft) * 0.01;
       tcpt2->xright = tcpt3->xleft + tcpt2->x2 * (tcpt3->xright - tcpt3->xleft) * 0.01;   
      }
     else tcpt1 = tcpt1->nxt;
    } 
   else tcpt1 = tcpt1->nxt;
  }

 /* Now check the regular children. */
 tcpt1 = container->children;
 while (tcpt1!=0)
  {
   if (tcpt1->superclass==Otk_SC_Select_List_Item)
    {
     tcpt1->ytop =    container->ytop + (tcpt1->y1 + vdy) * ctht;
     tcpt1->ybottom = container->ytop + (tcpt1->y2 + vdy) * ctht;
     if (tcpt1->ytop < container->ytop) 
      { // tcpt1->ytop = -101.0;  tcpt1->ybottom = -100.0;
	tcpt3 = tcpt1;
	tcpt1 = tcpt1->nxt;
	Otk_object_detach( tcpt3 );	/* Remove from drawable list, until needed. */
	Otk_object_quick_attach_hidden( container, tcpt3 );
      }
     else
     if (tcpt1->ybottom > container->ybottom)
      { // tcpt1->ytop = -101.0;  tcpt1->ybottom = -100.0;
	tcpt3 = tcpt1;
	tcpt1 = tcpt1->nxt;
	Otk_object_detach( tcpt3 );	/* Remove from drawable list, until needed. */
	Otk_object_quick_attach_hidden( container, tcpt3 );
      }
     else
      {
       tcpt2 = tcpt1->children;
       y1 = tcpt1->y1 + 0.01 * tcpt2->y1 * (tcpt1->y2 - tcpt1->y1);
       tcpt2->ytop =    container->ytop + (y1 + vdy) * ctht;
       y1 = tcpt1->y1 + 0.8 * (tcpt1->y2 - tcpt1->y1);
       tcpt2->ybottom = container->ytop + (y1 + vdy) * ctht;
       // if (tcpt2->ytop < container->ytop) { tcpt2->ytop = -101.0;  tcpt2->ybottom = -100.0; }
       // if (tcpt2->ybottom > container->ybottom) { if (tcpt2->ybottom<-90.0) stop = 1;  tcpt2->ytop = -101.0;  tcpt2->ybottom = -100.0; }
       tcpt1 = tcpt1->nxt;
      }
    } 
   else tcpt1 = tcpt1->nxt;
  }

 if (container->verttextscroll == 2)	/* If this is a master scroll-bar, */
  {					/* then scroll the subordinate lists too. */
   tcpt1 = container->children;
   while (tcpt1 != 0)
    {
     if ((tcpt1->superclass==Otk_SC_Select_List_SubList) && (tcpt1->object_class == Otk_class_other))
      Otk_scrolllist( v, tcpt1->scissor_parent );
     tcpt1 = tcpt1->nxt;
    }	    
  }
}


void Otk_scrolllist_up( void *container )
{
 OtkWidget tmpobj;
 float vpos; 

 tmpobj = (OtkWidget)container;
 if (tmpobj->mouse_sensitive)
  { /*knob*/
    vpos = Otk_GetSlider( tmpobj->parent );
    vpos = vpos - 0.05;
    if (vpos < 0.0) vpos = 0.0;
    Otk_SetSlider( tmpobj->parent, vpos, 1.0 );
    if (tmpobj->parent->functval2 != 0) tmpobj->parent->functval2( vpos, tmpobj->parent->callback_param );
    return;
  } /*knob*/
 tmpobj = tmpobj->children;	/* Find selection-list's slider. */
 if (tmpobj==0) { printf("Unexpected error 303\n"); return; }
 while (tmpobj->superclass != Otk_SC_vSlider)
  { tmpobj = tmpobj->nxt;  if (tmpobj==0) { printf("Unexpected error 303b\n"); return; } }
 vpos = Otk_GetSlider( tmpobj );
 vpos = vpos - 1.0 / (float)(((OtkWidget)container)->nentries);
 if (vpos < 0.0) vpos = 0.0;
 Otk_SetSlider( tmpobj, vpos, 1.0 );
 Otk_scrolllist( vpos, container );
 Otk_fb_clear_last_selected();
}


void Otk_scrolllist_down( void *container )
{
 OtkWidget tmpobj;
 float vpos;

 tmpobj = (OtkWidget)container;
 if (tmpobj->mouse_sensitive)
  { /*knob*/
    vpos = Otk_GetSlider( tmpobj->parent );
    vpos = vpos + 0.05;
    if (vpos > 1.0) vpos = 1.0;
    Otk_SetSlider( tmpobj->parent, vpos, 1.0 );
    if (tmpobj->parent->functval2 != 0) tmpobj->parent->functval2( vpos, tmpobj->parent->callback_param );
    return;
  } /*knob*/
 tmpobj = tmpobj->children;	/* Find selection-list's slider. */
 if (tmpobj==0) { printf("Unexpected error 304\n"); return; }
 while (tmpobj->superclass != Otk_SC_vSlider)
  { tmpobj = tmpobj->nxt;  if (tmpobj==0) { printf("Unexpected error 304\n"); return; } }
 vpos = Otk_GetSlider( tmpobj );
 vpos = vpos + 1.0 / (float)(((OtkWidget)container)->nentries);
 if (vpos > 1.0) vpos = 1.0;
 Otk_SetSlider( tmpobj, vpos, 1.0 );
 Otk_scrolllist( vpos, container );
 Otk_fb_clear_last_selected();
}



OtkWidget Otk_Add_Selection_Item( OtkWidget container, char *text, void (*callback)(void *x), void *parameter )
{
 OtkWidget tmpobj, tcpt1;
 float dy, ctht, thickness;
 int nentries;

 if (container->superclass == Otk_SC_Select_List_Item) container = container->parent;
 if (container->superclass != Otk_SC_Select_List) {printf("OtkError: adding selection item to non-selection-list.\n"); return 0;}
 dy = 100.0 / (float)container->nrows;
 tmpobj = OtkMakePanel( container, Otk_subtype_plane, Otk_White, 0.0, 0.0, 100.0, dy );
 nentries = container->nentries;
 container->nentries = nentries + 1;
 tmpobj->y1 = (float)nentries * dy;
 tmpobj->y2 = (float)(nentries+1) * dy;
 ctht = (container->ybottom - container->ytop) * 0.01;
 tmpobj->ytop    =  container->ytop + tmpobj->y1 * ctht;
 tmpobj->ybottom =  container->ytop + tmpobj->y2 * ctht;
 if (tmpobj->ybottom > container->ybottom) 
  { 
   // tmpobj->ytop = -101.0;  tmpobj->ybottom = -100.0;
   Otk_object_detach( tmpobj );
   Otk_object_quick_attach_hidden( container, tmpobj );

   if (container->verttextscroll < 0)	/* Add slider if needed. */
    { OtkWidget slider;
      container->verttextscroll = - container->verttextscroll;
      slider = OtkMakeSliderVertical( container, 100.0, 5.0, 90.0, Otk_scrolllist, container );
      Otk_SetSlider( slider, 0.0, 2.0 );
      thickness = slider->xright - slider->xleft;	/* Check the slider's absolute thickness. */
      thickness = 0.5 + 0.5 * (container->thickness / thickness);
      Otk_SetSlider_Thickness( slider, thickness );	/* If too thin or thick, adjust it. */
      thickness = slider->xright - slider->xleft;	/* Get the slider's actual resulting thickness. */
      slider->mouse_sensitive = mouse_groove_click;
      thickness = 100.0 * thickness / (container->xright - container->xleft);
      OtkMakeButton( container, 100.0, 0.0, thickness, 5.0, " ^", Otk_scrolllist_up, container );
      OtkMakeButton( container, 100.0, 95.0, thickness, 5.0, " v", Otk_scrolllist_down, container );
      otk_last_scrollable = container;
    }
  }
 tmpobj->superclass = Otk_SC_Select_List_Item;
 tmpobj->object_class = Otk_class_panel;
 tmpobj->mouse_sensitive = 1;
 tmpobj->callback = callback;
 tmpobj->callback_param = parameter;
 tcpt1 = OtkMakeTextLabel( tmpobj, text, Otk_Black, 1.0, 1.0, 2.0, 20.0 );
 tcpt1->scissor_to_parent = 1;
 if (!Otk_Vect_Font) Otk_Vect_Font = Otk_Build_Internal_Font(Otk_Font_Vect, Otk_FontSpacing_Mono);
 tcpt1->font = Otk_Vect_Font;
 tmpobj->children->superclass = Otk_SC_Select_List_Item;
 tmpobj->children->sqrtaspect =  tmpobj->parent->children->sqrtaspect;
 tmpobj->children->scale = tmpobj->parent->children->scale;
 return tmpobj;
}


void Otk_Coordinate_Selection_Lists( OtkWidget master_list, OtkWidget subordinate_list )
{ /* Makes subordinate selection list(s) operate under master's scroll-bar. */
 OtkWidget tmpobj;

 if (master_list->superclass != Otk_SC_Select_List) {printf("OtkError: Coordinating selection lists, master is not selection-list.\n"); return;}
 if (subordinate_list->superclass != Otk_SC_Select_List) {printf("OtkError: Coordinating selection lists of non-selection-list.\n"); return;}
 if (master_list->verttextscroll == -1) master_list->verttextscroll = -2;  else
 if (master_list->verttextscroll == 1) master_list->verttextscroll = 2;  else
 if (abs(master_list->verttextscroll) != 2)  {printf("OtkError: Coordinating selection lists, first list is not master.\n"); return;}
 if (subordinate_list->verttextscroll != -1) 
  printf("OtkError: Coordinating selection lists, second list already has scroll-bar state.\n");
 subordinate_list->verttextscroll = 3;

 /* Add subordinate to master's "controlled" list. */
 tmpobj = Otk_add_object( Otk_SC_Select_List_SubList, master_list );	/* Use as list-item to hold pointer to sub-list. */
 tmpobj->object_class = Otk_class_other;
 tmpobj->scissor_parent = subordinate_list;

 tmpobj = Otk_add_object( Otk_SC_Select_List_SubList, subordinate_list );
 tmpobj->object_class = Otk_class_other;
 tmpobj->scissor_parent = master_list;
}


/*********************************************************************************/
/* --- --- --- --- --- --- --- Tabbed Panels --- --- --- --- --- --- --- --- --- */

void Otk_tabbed_panel_select( void *ud )
{
 OtkTabbedPanelSelect *sel = (OtkTabbedPanelSelect *)ud;
 OtkTabbedPanel *tp = sel->tp;
 int i;

 DEBUG printf( "Switch to panel %d.\n", sel->selection );
 for( i = 0; i < tp->num; i++ )
  {
   Otk_object_detach_any( tp->panels[i] );
   if( i == sel->selection )
    {
     Otk_object_attach( tp->panel_top, tp->panels[i] );
     Otk_Set_Button_State( tp->buttons[i], 1 );
     // Otk_object_correct_position( tp->panels[i], 1 );
    }
   else
    {
     Otk_object_attach_hidden( tp->panel_top, tp->panels[i] );
     Otk_Set_Button_State( tp->buttons[i], 0 );
    }
  }
 tp->selection = sel->selection;
}


OtkWidget Otk_tabbed_panel_get_panel( OtkTabbedPanel *tp, int i )
{ return( tp->panels[i] ); }


OtkTabbedPanel *Otk_Tabbed_Panel_New( OtkWidget parent, int num, char **names,
				      OtkColor color, float left, float top,
				      float width, float height, float button_height )
{
 OtkTabbedPanel *tp = (OtkTabbedPanel *)calloc( 1, sizeof(OtkTabbedPanel) );
 int i;
 float dx = 100.0/(float)num;

 tp->num = num;
 tp->panel_height = 100.0 - button_height;
 tp->button_height = button_height;

 tp->top = OtkMakePanel( parent, Otk_Raised, color, left, top, width, height );
 tp->panel_top = OtkMakePanel( tp->top, Otk_Flat, color, 0.0, button_height,
			       100.0, tp->panel_height );
 tp->panel_top->color[3] = 0.0;
 Otk_Set_Object_Border_Thickness( tp->top, 0.2 );
 tp->names = (char **)malloc( sizeof(char *)*num );
 tp->panels = (OtkWidget *)malloc( sizeof(OtkWidget)*num );
 tp->buttons = (OtkWidget *)malloc( sizeof(OtkWidget)*num );
 tp->selects = (OtkTabbedPanelSelect *)malloc(sizeof(OtkTabbedPanelSelect)*num );
 for( i = 0; i < num; i++ )
  {
   tp->names[i] = strdup( names[i] );
   tp->panels[i] = OtkMakePanel( tp->panel_top, Otk_Recessed, color, 0.0, 0.0, 100.0, 100.0 );
   Otk_Set_Object_Border_Thickness( tp->panels[i], 0.3 );
   tp->selects[i].tp = tp;
   tp->selects[i].selection = i;
   tp->buttons[i] = OtkMakeButton( tp->top, ((float)i)*dx, 0, dx,
				   button_height, tp->names[i],
				   Otk_tabbed_panel_select, &(tp->selects[i]) );
   tp->buttons[i]->superclass = Otk_SC_TabHandle;
  }
 if( tp->num > 0 )
  {
   Otk_tabbed_panel_select( &(tp->selects[0]) );
   Otk_Set_Button_State( tp->buttons[0], 1 );
  }
 return( tp );
}

/* --- --- --- --- --- --- End Tabbed Panels --- --- --- --- --- --- --- --- --- */



/* --- --- --- --- --- --- Begin Hover Box --- --- --- --- --- --- --- --- --- */

struct otk_hover_object
 {
   OtkWidget obj;	/* Button or other object sensitive to this hover-box. */
   char *text;
   struct otk_hover_object *nxt;
 } *otk_hover_list=0;

OtkWidget otk_hover_active_box=0;
float otk_hover_timeout=0.75;	/* Hover time-out in seconds. */
double otk_hover_last_mousemove_time;
float otk_hover_last_x=-1.0, otk_hover_last_y=-1.0, otk_hover_text_size=1.4;
OtkColor otk_hover_panel_color, otk_hover_text_color;
int otk_hoverboxes_enabled=1;


void Otk_Register_Hover_Box( OtkWidget obj, char *text )
{
 struct otk_hover_object *newbox;
 if (obj->attribs | Otk_hover_mask) Otk_Unregister_Hover_Box( obj );
 newbox = (struct otk_hover_object *)malloc( sizeof(struct otk_hover_object) );
 newbox->nxt = otk_hover_list;
 otk_hover_list = newbox;
 newbox->obj = obj;
 newbox->text = strdup(text);
 obj->attribs = obj->attribs | Otk_hover_mask;
 if (Otk_verbose) printf("Registered Hover object = %lx\n", (unsigned long)obj);
}


void Otk_Unregister_Hover_Box( OtkWidget obj )
{
 struct otk_hover_object *boxptr, *boxprv=0;
 boxptr = otk_hover_list;
 while ((boxptr != 0) && (boxptr->obj != obj))
  { boxprv = boxptr;  boxptr = boxptr->nxt; }
 if (boxptr != 0)
  {
   if (boxprv == 0) otk_hover_list = otk_hover_list->nxt;
   else boxprv->nxt = boxptr->nxt;
   free( boxptr->text );
   free( boxptr );
  }
 obj->attribs = obj->attribs & 0xe;
}


void Otk_Set_HoverBox_Style( OtkColor box_color, OtkColor text_color, float text_size )
{
 otk_hover_panel_color = box_color;
 otk_hover_text_color = text_color;
 if (text_size > 0.0) otk_hover_text_size = text_size;
}

void Otk_Enable_Hoverboxes() { otk_hoverboxes_enabled = 1; }
void Otk_Disable_Hoverboxes() { otk_hoverboxes_enabled = 0; }


void kill_hoverbox()
{
 if (otk_hover_active_box)
  {
       Otk_RemoveObject( otk_hover_active_box );
       otk_hover_active_box = 0;
  }
}


void otk_hover_box_check()
{
 double curtime;
 OtkWidget objpt, topobj;
 float minz, ypos, horiz_size, vert_size;
 struct otk_hover_object *hitem;
 int j, k, col, maxcols, rows;
 char *tstr, mark;

 if (!otk_hoverboxes_enabled) return;
 if (otk_hover_list == 0) return;
 curtime = otk_report_time();
 if ((fabs(Otk_MouseX - otk_hover_last_x) > 0.75) || (fabs(Otk_MouseY - otk_hover_last_y) > 0.75))
  { /*mouse_moved*/
    // printf("	Mouse Moved (%g, %g)\n", Otk_MouseX-otk_hover_last_x, Otk_MouseY-otk_hover_last_y );
    otk_hover_last_x = Otk_MouseX;
    otk_hover_last_y = Otk_MouseY;
    otk_hover_last_mousemove_time = curtime;
    kill_hoverbox();
  } /*mouse_moved*/
 else
 if (curtime - otk_hover_last_mousemove_time > otk_hover_timeout)
  { /*mouse_stationary*/

    if (otk_hover_active_box)	/* If hoverbox is already up (for 30-seconds), remove it. */
     {
      kill_hoverbox();
      otk_hover_last_mousemove_time = curtime + 36000.0;  /* Avoid future checks, unless moved again. */
     }
    else
     { /*hover_timeout*/
      /* First check if mouse is over any object that is on the hover-list. */
      hitem = otk_hover_list;
      while ((hitem != 0) && ((Otk_MouseX < hitem->obj->xleft) || (Otk_MouseX > hitem->obj->xright)
		|| (Otk_MouseY < hitem->obj->ytop) || (Otk_MouseY > hitem->obj->ybottom)))
       hitem = hitem->nxt;

      if (hitem != 0)
       { /*over_hover_item*/
         /* Determine which object the mouse is presently over, if any, considering any obscurations. */
         topobj = 0;  minz = -9e9;
         objpt = OtkRootObject;
         while (objpt!=0)
          { /*while0bj*/
	    if ((objpt->object_class == Otk_class_panel && !objpt->invisible) || (objpt->object_class == Otk_class_button) || 
	        (objpt->superclass == Otk_SC_RadioButton) || (objpt->object_class == Otk_class_togglebutton) ||
	        (objpt->superclass == Otk_SC_Menu_DropDown_Button) || (objpt->object_class == Otk_class_circle))
	     {
	      if ((Otk_MouseX >= objpt->xleft) && (Otk_MouseX <= objpt->xright) &&
		     (Otk_MouseY >= objpt->ytop) && (Otk_MouseY <= objpt->ybottom))
	       if (objpt->z > minz) { minz = objpt->z;  topobj = objpt; }
	     }
	    /* Advance */
	    if ( objpt->children )  objpt = objpt->children;		/* Descend. */
	    else
	     {
	      while( objpt && !objpt->nxt ) 	/* Ascend, as needed. */
	      objpt = objpt->parent;
	      if ( objpt ) objpt = objpt->nxt;	/* Advance. */
	     }
          } /*while0bj*/

         /* Check if same object is on the hover-list. */
         if ((topobj != 0) && (((topobj->attribs) & Otk_hover_mask) == 1))  /* If OK, then pop-up Hover-Box. */
          { /*Pop-up_hover-box.*/
	    float x1, y1;
	    if (Otk_verbose) printf("Hover timeout on hover item.\n");
	    if (otk_hover_active_box) kill_hoverbox();	/* Remove any previous hoverbox. */
	    /* Determine needed size of text-box. */
	    j = 0;  maxcols = 0;  col = 0;  rows = 1;  ypos = 5.0;
	    while (hitem->text[j] != '\0')
	     {
	      if (hitem->text[j] == '\n') { rows++;  if (col > maxcols) maxcols = col;  col = 0; }
	      else col++;
	      j++;
	     }
	    if (col > maxcols) maxcols = col;
	    x1 = hitem->obj->xleft;
	    y1 = hitem->obj->ytop;
	    horiz_size = otk_hover_text_size * 1.3 * (float)maxcols + 2.5;
            vert_size  = otk_hover_text_size * 6.0;
	    if (x1 + horiz_size > 97.0) x1 = x1 - (x1 + horiz_size - 97.0);
	    if (y1 + vert_size > 97.0) y1 = y1 - (y1 + vert_size - 97.0);
	    if (x1 < 3.0) x1 = x1 + 3.0;
	    if (y1 < 3.0) y1 = y1 + 3.0;
	    otk_hover_active_box = OtkMakePanel( OtkOuterWindow, Otk_Flat, otk_hover_panel_color, x1, y1,
						 horiz_size, otk_hover_text_size * 3.5 * (float)rows + 1.0 );
	    otk_hover_active_box->z = Otk_window_level + 3.0;
	    // Otk_Display_Changed++;
	    tstr = strdup( hitem->text );
	    j = 0;
	    while (tstr[j] != '\0')	/* Place text-lines onto hover-box. */
	     { float width, height;
	       k = j;
	       while ((tstr[k] != '\0') && (tstr[k] != '\n')) k++;		/* Find end of current line. */
	       mark = tstr[k];
	       tstr[k] = '\0';
	       if (k - j > 0)
	        {
	         objpt = OtkMakeTextLabel( otk_hover_active_box, &(tstr[j]), otk_hover_text_color, 1.0, 1, 0.5, ypos );
	         Otk_Get_Text_Size( objpt, &width, &height );
	         width = (float)maxcols * width / (float)(k-j);
	         objpt->sqrtaspect = sqrt( (height * horiz_size) / (width * 0.8 * vert_size) );
	         objpt->scale = 0.925 * horiz_size / (width * objpt->sqrtaspect);
		}
	       ypos = ypos + 90.0 / (float)rows;
	       if (mark == '\n')  j = k + 1;  else  { j = k;  tstr[k] = '\0'; }	   
	     }
	    free( tstr );
	    if (! Otk_Display_Changed)
	     otk_update_draw_object( otk_hover_active_box );
          } /*Pop-up_hover-box.*/
       } /*over_hover_item*/
      otk_hover_last_mousemove_time = curtime + 5.0;  /* Maximum hover-note persist2nce. */
     } /*hover_timeout*/
  } /*mouse_stationary*/
}

/* --- --- --- --- --- --- End Hover Box --- --- --- --- --- --- --- --- --- */


struct Otk_word_list_item
 {
  char *word;
  struct Otk_word_list_item *nxt;
 };

void Otk_clear_word_list( struct Otk_word_list_item **list )
{ struct Otk_word_list_item *item;
  while (*list != 0)
   {
    item = *list;
    *list = item->nxt;
    free( item->word );
    free( item );
   }
}

void Otk_add_word_to_end_of_list( struct Otk_word_list_item **list, char *word )
{
 struct Otk_word_list_item *newitem, *lastitem;
 newitem = (struct Otk_word_list_item *)malloc( sizeof(struct Otk_word_list_item) );
 newitem->word = strdup(word);
 newitem->nxt = 0;
 if (*list == 0)
  *list = newitem;
 else
  {
   lastitem = *list;
   while (lastitem->nxt != 0) lastitem = lastitem->nxt;
   lastitem->nxt = newitem;
  }
}


/*********************************************************************************/
/* --- --- --- --- --- --- --- File Browser --- --- --- --- --- --- --- --- --- */

struct Otk_dlist
 {
  char kind, month[30], year[30], *name;
  int day, sz;
  struct Otk_dlist *nxt;
 };


OtkWidget Otk_fbwindow, Otk_fb_filename_formbox, Otk_fb_wildcard_formbox;
int Otk_fbwindow_state=0, Otk_fb_maxlen, Otk_fb_dirsonly, Otk_fb_allowfiles=0, Otk_fb_showdotfiles=1;
char Otk_fb_filename[2048], Otk_fb_dirname[2048], Otk_fb_wildcard[500], *Otk_fb_prompt;
char *Otk_fb_fnptr, *Otk_fb_dnptr, *Otk_fb_wcptr, Otk_fb_Selected[2][2048]={"",""};
float otk_fb_width=-1.0, otk_fb_height, otk_fb_hcenter_offset=0.0, otk_fb_vcenter_offset=0.0;
struct Otk_word_list_item *Otk_fb_altdirs=0;	/* List of alternate directories or files to suggest. */
void (*Otk_fb_callback)(char *fname);           /* Callback function for filebrowser. */

void Otk_fbkilled() { Otk_fbwindow_state = 0; }
OtkWidget Otk_BrowseFiles0( char *prompt, int maxlength, char *directory, char *wildcards, char *filename, void (*callback)(char *fname) );


void Otk_fb_add_optional_dirfile( char *dir_or_file )	/* Provides method to suggest alternate directory(s) via pull-down. */
{ Otk_add_word_to_end_of_list( &Otk_fb_altdirs, dir_or_file ); }

void Otk_fb_clear_optional_dirfiles()			/* Clears all previous list entries. */
{ Otk_clear_word_list( &Otk_fb_altdirs ); }

void Otk_fb_clear_last_selected() { Otk_fb_Selected[0][0] = '\0'; }

void Otk_fb_cancel()
{
 Otk_fbkilled();
 Otk_RemoveObject( Otk_fbwindow );
 Otk_Display_Changed++;
 strcpy(Otk_fb_Selected[0],"");
}


void Otk_fb_wildcard_accept()
{
 Otk_Get_Text( Otk_fb_wildcard_formbox, Otk_fb_wildcard, 500 );
 DEBUG printf("accpted wildcard '%s'\n", Otk_fb_wildcard);
 Otk_BrowseFiles0( Otk_fb_prompt, Otk_fb_maxlen, Otk_fb_dirname, Otk_fb_wildcard, Otk_fb_filename, Otk_fb_callback );
 Otk_Display_Changed++;
}


/*.......................................................................
  .  Otk_NEXT_WORD - accepts a line of text, and returns with the        .
  . next word in that text in the second parameter, the original line   .
  . is shortened so that the word is removed. If the line encountered   .
  . is empty, then the word returned will be empty.                     .
  . NEXTWORD can parse on an arbitrary number of delimiters.         	.
  .......................................................................*/
void Otk_next_word( char *line, char *word, char *delim )
{
 int i=0, j=0, m=0, nodelim=1;

 /* Consume any preceding white-space. */
 while ((line[i]!='\0') && (nodelim))
  {
   j = 0;
   while ((delim[j]!='\0') && (line[i]!=delim[j])) j = j + 1;
   if (line[i]==delim[j]) { i = i + 1; } else  nodelim = 0;
  }
 /* Copy the word until the next delimiter. */
 while ((line[i]!='\0') && (!nodelim))
  {
   word[m++] = line[i++];
   if (line[i]!='\0')
    {
     j = 0;
     while ((delim[j]!='\0') && (line[i]!=delim[j])) j = j + 1;
     if (line[i]==delim[j]) nodelim = 1;
    }
  }
 /* Shorten line. */
 j = 0;
 while (line[i]!='\0') { line[j++] = line[i++]; }
 /* Terminate the char-strings. */
 line[j] = '\0';
 word[m] = '\0';
}


#if (PLATFORM_KIND==Posix_Platform)
 #define OTK_DIRSEP_CHR '/'	/* File directory delimiter. */
 #define OTK_DIRSEP_STR "/"
 #define OTK_RTN_CHR '\n'	/* Character returned by carriage-return key. */
#else
 #define OTK_DIRSEP_CHR '\\'	/* File directory delimiter. */
 #define OTK_DIRSEP_STR "\\"
 #define OTK_RTN_CHR '\n'	/* Character returned by carriage-return key. */
#endif

 
/********************************************************************************/
/* strcpy_safe - Copy src string to dst string, up to maxlen characters. 	*/
/* Safer than strncpy, because it does not fill destination string, 		*/
/* but only copies up to the length needed.  Src string should be 		*/
/* null-terminated, and must-be if its allocated length is shorter than maxlen.	*/
/* Up to maxlen-1 characters are copied to dst string. The dst string is always	*/
/* null-terminated.  The dst string should be pre-allocated to at least maxlen	*/
/* bytes.  However, this function will work safely for dst arrays that are less	*/
/* than maxlen, as long as the null-terminated src string is known to be 	*/
/* shorter than the allocated length of dst, just like regular strcpy.		*/
/********************************************************************************/
void strcpy_safe( char *dst, const char *src, int maxlen )
{ 
  int j=0, oneless;
  oneless = maxlen - 1;
  while ((j < oneless) && (src[j] != '\0')) { dst[j] = src[j];  j++; }
  dst[j] = '\0';
}


void otk_shorten_string_at_front( char *string, int n )
{ int k=0;
  do { string[k] = string[k+n];  k++; }
  while (string[k-1] != '\0');
}

void otk_eliminate_leading_dot( char *fname )	/* Remove leading dot-slash (./), not needed. */
{
 while ((fname[0] == '.') && (fname[1] == OTK_DIRSEP_CHR))
  otk_shorten_string_at_front( &(fname[0]), 2 );
}


#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>


void otk_reduce_pathname( char *fname )
{
 int k=0, match=1;
 char twrd1[2048], twrd2[1024], owrd[1024], twrd3[2048];

 DEBUG printf("REDUCING '%s'\n", fname);
 while (fname[k] != '\0')		/* Unify to proper type of slash (dir-separator) on host system. */
  { if ((fname[k] == '/') || (fname[k] == '\\')) fname[k] = OTK_DIRSEP_CHR;  k++; }

 if (strcmp(fname,".")==0) fname[0] = '\0';	/* Do not need dot (.) explicitly for current directory. */

 strcpy( twrd1, OTK_DIRSEP_STR );  strcat( twrd1, OTK_DIRSEP_STR );
 while ( strstr(fname, twrd1) )			/* Remove any double-slashes. */
  { otk_shorten_string_at_front( strstr(fname,twrd1), 1 ); }

 k = strlen( fname ) - 1;	/* Check for and remove any trailing slash. */
 #if (PLATFORM_KIND!=Posix_Platform)
  if ((k == 1) && (fname[k] == ':')) { strcat(fname, "\\");  return; }
  if ((k == 2) && (fname[k-1] == ':') && (fname[k] == OTK_DIRSEP_CHR))  return;
 #endif
 if ((k > 0) && (fname[k] == OTK_DIRSEP_CHR)) fname[k] = '\0';

 do				/* Remove obvious "go-down/go-up" combinations. */
  {
   match = 0;
   twrd3[0] = '\0';
   strcpy(twrd1,fname);
   Otk_next_word(twrd1,owrd,OTK_DIRSEP_STR);
   Otk_next_word(twrd1,twrd2,OTK_DIRSEP_STR);
   while (strlen(twrd2) > 0)
    { 
     if ((strcmp(owrd,"..") != 0) && (strcmp(twrd2,"..")==0))
      { match = 1;  twrd2[0] = '\0'; }
     else
     if (strlen(owrd) > 0)
      { if (strlen(twrd3) > 0) strcat(twrd3, OTK_DIRSEP_STR); 
	strcat(twrd3, owrd);
      }
     strcpy(owrd,twrd2);
     Otk_next_word(twrd1,twrd2,OTK_DIRSEP_STR);
    }
   if (strlen(owrd) > 0) { if (strlen(twrd3) > 0) strcat(twrd3,OTK_DIRSEP_STR); strcat(twrd3,owrd); }
   if (fname[0] == OTK_DIRSEP_CHR) strcpy(fname,OTK_DIRSEP_STR); else fname[0] = '\0';
   strcat(fname, twrd3);
  }
 while (match != 0);
 if (fname[0] == '\0') strcat(fname,".");
}


void Otk_EnvironVarFilter( char *fname )
{ /* Intercept and replace any defined environment variables in file-names. */
 int k=0, j, m, n, p, seppt=1; 
 char twrd[2048], *evar;

 if (strchr(fname,'$') == 0) return;
 while ((k < 2048-1) && (fname[k] != '\0'))
  { /*lookfor$*/
    if ((seppt) && (fname[k] == '$'))
     { /*replace_envvar*/
      k++;	/* Grab just the env-var name. */
      j = 0;  m = k;
      while ((m < 2048-1) && (fname[m] != '/') && (fname[m] != '\\') && (fname[m] != '\0'))
	{ twrd[j++] = fname[m++]; }
      twrd[j] = '\0';
      evar = getenv(twrd);
      if (evar)
       {
	k--;
	n = strlen(evar);
	p = n - m + k;
	if (p > 0)
	 { /*expand string*/
	   n = strlen(fname);
	   while (n >= m) { fname[n+p] = fname[n];  n--; }
	 }
	else
	if (p < 0)
	 { /*shrink string.*/
	   n = m + p;
	   do { fname[n] = fname[n-p];  n++; } while (fname[n-1] != '\0');
	 }
	p = 0;  /* Copy evar into position. */
	while (evar[p] != '\0') fname[k++] = evar[p++];
       }
      else 
       k = m;
     } /*replace_envvar*/
    else
     {
      if ((fname[k] == '/') || (fname[k] == '\\')) seppt = 1; else seppt = 0;
      k++;
     }
  } /*lookfor$*/
}



void Otk_fb_accept( void *data ) /* Determine if selected file is a directory or regular file. */
{			/* If it is a directory, then open browser to that directory, otherwise return file's-name. */
 char pathname[5000];
 struct stat buf;
 int k;

 Otk_Get_Text( Otk_fb_filename_formbox, Otk_fb_filename, 2048 );
 DEBUG printf("accepted file '%s/%s'\n", Otk_fb_dirname, Otk_fb_filename);

 if (strcmp(Otk_fb_filename,".") == 0) { Otk_fb_dirname[0] = '\0'; }
 Otk_EnvironVarFilter( Otk_fb_filename );
 otk_reduce_pathname( Otk_fb_filename );

 if ((Otk_fb_filename[0] == OTK_DIRSEP_CHR) || ((Otk_fb_filename[0] != '\0') && (Otk_fb_filename[1] == ':')))
  Otk_fb_dirname[0] = '\0';	/* If filename contains full absolute path, then erase old path from dirname. */
 else				/* Otherwise, a relative path was given. */
  otk_eliminate_leading_dot( Otk_fb_filename );

 /* Construct the new full-path name of the file or directory that was selected. */
 strcpy_safe( pathname, Otk_fb_dirname, 4000);
 k = strlen(pathname) - 1;  /* Remove the trailing slash from the directory name, if any. */
 if ((k > 0) && (pathname[k] == OTK_DIRSEP_CHR)) pathname[k] = '\0';
 if (pathname[0] != '\0') strcat(pathname, OTK_DIRSEP_STR);  /* Add trailing slash to directory name. */
 strcat( pathname, Otk_fb_filename );	/* Append the file-name portion to create full path name. */
 otk_eliminate_leading_dot( pathname );
 otk_reduce_pathname( pathname );

 stat( pathname, &buf );
 if ((S_ISDIR(buf.st_mode)) && ((Otk_fb_dirsonly == 0) || (data != 0)))
  { /*Traverse directories*/
    DEBUG printf("Traversing directory\n");
    Otk_fb_cancel();	/* Close the fbrowser window. */
    strcpy_safe(Otk_fb_dirname, pathname, Otk_fb_maxlen);
    strcpy_safe(Otk_fb_filename,"", Otk_fb_maxlen);
    Otk_BrowseFiles0( Otk_fb_prompt, Otk_fb_maxlen, Otk_fb_dirname, Otk_fb_wildcard, Otk_fb_filename, Otk_fb_callback );
  }
 else
  { /*Regular_file*/
   if ((Otk_fb_dirsonly) && (!S_ISDIR(buf.st_mode)) && (! Otk_fb_allowfiles)) return;
   DEBUG printf("Returning filename\n");
   Otk_fb_cancel();	/* Close the fbrowser window. */
   /* Separate directory path-name and 'potential' file-name, into appropriate parts. */
   k = strlen( pathname ) - 1;	/* Find last slash, if any. */
   while ((k >= 0) && (pathname[k] != OTK_DIRSEP_CHR)) k--;
   if (k < 1)	/* If no slash(es) after leading position. */
    { Otk_fb_dirname[0] = '\0';  strcpy( Otk_fb_filename, pathname ); }  /* Place all in filename. */
   else
    { strcpy( Otk_fb_dirname, pathname );  Otk_fb_dirname[k] = '\0';  strcpy( Otk_fb_filename, &(pathname[k+1]) ); }
   strcpy_safe(Otk_fb_fnptr, Otk_fb_filename, Otk_fb_maxlen);
   strcpy_safe(Otk_fb_dnptr, Otk_fb_dirname,  Otk_fb_maxlen);
   strcpy_safe(Otk_fb_wcptr, Otk_fb_wildcard, Otk_fb_maxlen);
   Otk_fb_callback( pathname ); 
  }
 Otk_Display_Changed++;
}


void Otk_fb_accept2( char *s, void *data ) { Otk_fb_accept( data ); }


void Otk_fb_select( void *fname )
{ char *name=(char *)fname;
 // printf("selected file item '%s'\n", name );
 if ((strcmp(name,Otk_fb_Selected[0])==0) /* && (strcmp(name,Otk_fb_Selected[1])==0) */ )
  {
   strcpy(Otk_fb_Selected[0],"");
   strcpy(Otk_fb_Selected[1],"");
   Otk_fb_accept("1");  /* Double-click. */
  }
 else
  {
   Otk_Modify_Text( Otk_fb_filename_formbox, name );
   strcpy(Otk_fb_Selected[1],Otk_fb_Selected[0]);
   strcpy(Otk_fb_Selected[0],name);
  }
 Otk_Display_Changed++;
}


void Otk_fb_select_and_go( void *fname )
{
 char *name=(char *)fname;
 Otk_Modify_Text( Otk_fb_filename_formbox, name );
 strcpy(Otk_fb_Selected[1],Otk_fb_Selected[0]);
 strcpy(Otk_fb_Selected[0],name);
 Otk_fb_select( fname );
}


void otk_truncate_fname( char *truncname, char *origfname, int n )	/* Limit file names to no longer than n characters. */
{
 if (strlen(origfname) > n)
  {
    strcpy_safe( truncname, origfname, n - 2 );		/* Truncate and add ".." indication that is was truncated. */
    strcat(truncname, ".." );
  }
 else strcpy( truncname, origfname );
}



struct otk_directory_item
 {
  char *filename;
  time_t file_date;
  int sz;
  struct otk_directory_item *nxt;
 } *otk_file_ignore_lst=0;

void otk_format_minutes( int value, char *buf )
{
 if (value < 10)
  { char buf2[10];
    sprintf(buf2,"%d", value);
    strcpy(buf,"0"); strcat(buf,buf2);
  } else sprintf(buf,"%2d", value);
}

int otk_fb_current_day=0;


struct otk_directory_item *otk_new_dirlist_item( char *fname, int sz, time_t filedate )
{
  struct otk_directory_item *newitem;

  newitem = (struct otk_directory_item *)malloc( sizeof(struct otk_directory_item) );
  newitem->filename = strdup( fname );
  newitem->sz = sz;
  newitem->file_date = filedate;
  return newitem;
}


int otk_wildcard_match( char *fname, char *wildcards[] )
{
 int j=0; 
 while ((wildcards[j] != 0) && (strstr(fname, wildcards[j]) == 0)) j++;
 if (wildcards[j] != 0) return 1; else return 0;
}


void Otk_ignore_file( char *name, int exact_match_required )
{
 struct otk_directory_item *new_item;

 new_item = (struct otk_directory_item *)malloc( sizeof( struct otk_directory_item ) );
 new_item->filename = strdup( name );
 new_item->sz = exact_match_required;
 new_item->nxt = otk_file_ignore_lst;
 otk_file_ignore_lst = new_item;
}

void Otk_clear_ignore_file_list()
{
 struct otk_directory_item *old_item;

 while (otk_file_ignore_lst != 0)
  {
   old_item = otk_file_ignore_lst;
   otk_file_ignore_lst = otk_file_ignore_lst->nxt;
   free( old_item->filename );
   free( old_item );
  }
}

void Otk_clear_ignored_file( char *name )
{
 struct otk_directory_item *item, *old_item;

 item = otk_file_ignore_lst;
 while (item != 0)
  {
   old_item = item;
   item = item->nxt;
   if (strcmp( old_item->filename, name ) == 0)
    {
     if (old_item == otk_file_ignore_lst) otk_file_ignore_lst = item;
     free( old_item->filename );
     free( old_item );
    }
  }
}

int otk_ignored_file( char *name )	/* Returns true is file should be ignored.  Otherwise returns 0. */
{
 struct otk_directory_item *item;

 item = otk_file_ignore_lst;
 while (item != 0)
  { if (item->sz)
     { if (strcmp(item->filename, name) == 0) return 1; }
    else
     { if (strstr(name, item->filename) != 0) return 1; }
    item = item->nxt;
  }
 return 0;
}


OtkWidget Otk_BrowseFiles0( char *prompt, int maxlength, char *directory, char *wildcards, char *filename, void (*callback)(char *fname) )
{
 char line[1024], word[1024], *pathname, *wildcard_array[20], twrd1[500];
 OtkWidget slist, wdgt, pulldown;
 float dx, dy, asp, ypos=9.25, boxwidth;

 pathname = (char *)malloc( 5000 * sizeof(char) );
 if (Otk_fbwindow_state != 0) Otk_RemoveObject( Otk_fbwindow );
 Otk_fbwindow_state = 1;
 otk_reduce_pathname( directory );
 if (otk_fb_width > 0.0)
  {
   dx = otk_fb_width;
   dy = otk_fb_height;
  }
 else
  {
   asp = sqrt( (float)OtkWindowSizeY / ((float)OtkWindowSizeX+0.001) );
   dx = /* (0.5 + 400.0/(float)(OtkWindowSizeY+OtkWindowSizeX)) * */ 85.0 * asp;  if (dx>95.0) dx = 95.0;
   dy = /* (0.5 + 400.0/(float)(OtkWindowSizeY+OtkWindowSizeX)) * */ 80.0 / asp;  if (dy>95.0) dy = 95.0;
  }
 Otk_fbwindow = OtkMakeWindow( 2, Otk_Blue, Otk_LightGray, 
			       50.0-0.5*dx + otk_fb_hcenter_offset,
			       50.0-0.5*dy + otk_fb_vcenter_offset, dx, dy );
 Otk_Set_Object_Border_Thickness( Otk_fbwindow, 0.25 );
 Otk_RegisterWindowKillEventFunction( Otk_fbwindow, Otk_fbkilled, 0 );
 // otk_truncate_fname( pathname, directory, 55 );	/* Limit directory title to no longer than 55 characters. */
 sprintf(line,"Directory: %s", directory );
 Otk_Set_Text_Aspect( 0.6 );
 wdgt = OtkMakeTextLabel( Otk_fbwindow, line, Otk_Black, 1.6, 1, 3, 3 );
 wdgt->scissor_to_parent = 1;
 OtkMakeTextLabel( Otk_fbwindow, "File       Size", Otk_Black, 1.1, 1, 4, ypos );
 OtkMakeTextLabel( Otk_fbwindow, "Date", Otk_Black, 1.1, 1, 24, ypos );
 OtkMakeTextLabel( Otk_fbwindow, "Name", Otk_Black, 1.1, 1, 41, ypos );

 strcpy(Otk_fb_dirname, directory);

 { int j=0;	/* Filter any '*' from wildcards, and parse into an array. */
   strcpy_safe( line, wildcards, 500 );
   Otk_next_word( line, word, " \t*" );
   while ((word[0] != '\0') && (j < 18))
    {
     wildcard_array[j++] = strdup( word );
     Otk_next_word( line, word, " \t*" );
    }
   wildcard_array[j] = 0;	/* Terminate wild-card list. */
 }

 if (otk_fb_current_day == 0)
  { /*Get current date.*/
    const struct tm *tm;
    #ifdef __MINGW32__
     time_t T;
     T = time(0);
     tm = localtime( &T );
    #else
     struct timeval tp;
     gettimeofday(&tp,0);
     tm = localtime( &(tp.tv_sec) );
    #endif
   /* Compute days since 1900, assuming all months have 31 days. */
   otk_fb_current_day = tm->tm_year * (12 * 31) + tm->tm_mon * 31 + tm->tm_mday;
  }

 ypos = 13.25;		/* Make the file selection list. */
 //      Otk_Make_SelectionList( Otk_fbwindow,  rows,  cols,  x,    y,   width, height )
 slist = Otk_Make_Selection_List( Otk_fbwindow,  13,   60,   3.5,  ypos,  91,   60 );
 Otk_Set_Text_Aspect( 0.35 );

    { /*listdirectory*/
      DIR *dirpt;
      struct stat buf;
      struct dirent *dir_entry;
      struct tm *time_struct;
      char month_name[13][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec","BAD"};
      int month_value, day;
      struct otk_directory_item *dir_sublist_hd, *dir_filelist_hd, *newitem, *flstptr, *olditem, *previtem;
      char minfrmt[30], yeartime[50];

      // printf("Listing dir: %s\n", directory );
      dir_sublist_hd = 0;
      dir_filelist_hd = 0;
      dirpt = opendir( directory );
      if (dirpt == 0) {printf("Error: Could not open directory '%s'.\n", directory );  /* exit(1); */ }
      else
       { /*ok*/
         dir_entry = readdir(dirpt);
         while (dir_entry != 0)
          { /*direntry*/
	    if ((Otk_fb_showdotfiles) || ((dir_entry->d_name[0] != '.') || (strcmp(dir_entry->d_name,"..") == 0)))
            if ((strcmp(dir_entry->d_name,".") != 0) && (! otk_ignored_file( dir_entry->d_name )))
             { /*fileorsubdir*/
	       strcpy_safe(pathname, directory, 4000);  strcat(pathname,"/");
	       strcat( pathname, dir_entry->d_name );
	       stat( pathname, &buf );
               if (S_ISDIR(buf.st_mode))	/* If subdirectory, then */
 	        {				/*  Enqueue onto subdirectory list. */
		 newitem = otk_new_dirlist_item( dir_entry->d_name, buf.st_size, buf.st_mtime );
		 previtem = 0;			/* Insert in alphabetic sorted order. */
		 flstptr = dir_sublist_hd;
		 while ((flstptr != 0) && (strcmp(flstptr->filename, dir_entry->d_name) < 0 ))
		  { previtem = flstptr;  flstptr = flstptr->nxt; }
		 if (previtem == 0) { dir_sublist_hd = newitem; }
		 else { previtem->nxt = newitem; }
		 newitem->nxt = flstptr;
                }
               else
	       // if ((!Otk_fb_dirsonly) &&
	       if ( ((wildcard_array[0] == 0) || (otk_wildcard_match( dir_entry->d_name, wildcard_array ))))
 	        {				/*  Otherwise, Enqueue on regular files list. */
		 newitem = otk_new_dirlist_item( dir_entry->d_name, buf.st_size, buf.st_mtime );
		 previtem = 0;			/* Insert in alphabetic sorted order. */
		 flstptr = dir_filelist_hd;
		 while ((flstptr != 0) && (strcmp(flstptr->filename, dir_entry->d_name) < 0 ))
		  { previtem = flstptr;  flstptr = flstptr->nxt; }
		 if (previtem == 0) { dir_filelist_hd = newitem; }
		 else { previtem->nxt = newitem; }
		 newitem->nxt = flstptr;
                }

             } /*fileorsubdir*/
            dir_entry = readdir(dirpt);
          } /*direntry*/
         closedir( dirpt );
       } /*ok*/

      /* First list the directories at this level. */
      flstptr = dir_sublist_hd;
      while (flstptr != 0)
       {
	// otk_truncate_fname( word, flstptr->filename, 46 );	/* Limit file names to no longer than 45 characters. */
	/* If file is older than six months, list year, else list time of day. */
	time_struct = localtime( &(flstptr->file_date) );
	day = time_struct->tm_year * (12 * 31) + time_struct->tm_mon * 31 + time_struct->tm_mday;
	if (otk_fb_current_day - day > 6 * 31)
	 sprintf(yeartime,"%d", time_struct->tm_year + 1900 );
	else
	 {
	  otk_format_minutes( time_struct->tm_min, minfrmt );
	  sprintf(yeartime,"%2d:%s", time_struct->tm_hour, minfrmt );
	 }
	month_value = time_struct->tm_mon;
	if ((month_value < 0) || (month_value > 11)) { printf("ERROR: BAD MONTH %d for file %s\n", month_value, pathname );  month_value = 12; }
	strcpy(line,"d ");
	sprintf(twrd1,"%8d  ", flstptr->sz );
	strcat(line, twrd1);  strcat(line, month_name[month_value]);
	sprintf(twrd1," %2d ", time_struct->tm_mday );
	strcat(line, twrd1);
        sprintf(twrd1," %5s  ", yeartime );
	strcat(line, twrd1);  strcat(line, flstptr->filename);
        // sprintf(line, "d %8d  %s %2d %5s  %s", flstptr->sz, month_name[month_value], time_struct->tm_mday, yeartime, flstptr->filename );
	Otk_Add_Selection_Item( slist, line, Otk_fb_select, flstptr->filename );
	olditem = flstptr;
        flstptr = flstptr->nxt;		/* Free file entries as we list them. */
	free( olditem );
       }

      /* Second list the regular files at this level. */
      flstptr = dir_filelist_hd;
      while (flstptr != 0)
       {
	// otk_truncate_fname( word, flstptr->filename, 46.0 );	/* Limit file names to no longer than 45 characters. */
	/* If file is older than six months, list year, else list time of day. */
	time_struct = localtime( &(flstptr->file_date) );
	day = time_struct->tm_year * (12 * 31) + time_struct->tm_mon * 31 + time_struct->tm_mday;
	if (otk_fb_current_day - day > 6 * 31)
	 sprintf(yeartime,"%d", time_struct->tm_year + 1900 );
	else
	 {
	  otk_format_minutes( time_struct->tm_min, minfrmt );
	  sprintf(yeartime,"%2d:%s", time_struct->tm_hour, minfrmt );
	 }
	month_value = time_struct->tm_mon;
	if ((month_value < 0) || (month_value > 11)) { printf("ERROR: BAD MONTH %d for file %s\n", month_value, pathname );  month_value = 12; }
	sprintf(line,"  %8d  ", flstptr->sz );
	strcat(line, month_name[month_value]);
	sprintf(twrd1," %2d ", time_struct->tm_mday );
	strcat(line, twrd1);
        sprintf(twrd1," %5s  ", yeartime );
	strcat(line, twrd1);  strcat(line, flstptr->filename);
        // sprintf(line, "  %8d  %s %2d %5s  %s", flstptr->sz, month_name[month_value], time_struct->tm_mday, yeartime, flstptr->filename );
	if ((!Otk_fb_dirsonly) || (Otk_fb_allowfiles))
	 Otk_Add_Selection_Item( slist, line, Otk_fb_select, flstptr->filename );
	else
	 Otk_Add_Selection_Item( slist, line, 0, 0 );
	olditem = flstptr;
        flstptr = flstptr->nxt;		/* Free file entries as we list them. */
	free( olditem );
       }
    } /*listdirectory*/

 /* Cleanup any temporary wildcards. */
 { int j=0;
   while (wildcard_array[j] != 0) { free(wildcard_array[j]);  j++; }
 }

 Otk_Set_Text_Aspect( 0.6 );
 ypos = 75.5;
 OtkMakeTextLabel( Otk_fbwindow, prompt, Otk_Black, 1.6, 1, 3, ypos );

 Otk_Set_Text_Aspect( 1.0 );
 ypos = ypos + 5.0;
 if (Otk_fb_altdirs)	/* If alternate-directories list is populated, then place selector button. */
  { struct Otk_word_list_item *altdir=Otk_fb_altdirs;
   boxwidth = 91;
   pulldown = Otk_Make_Menu( Otk_fbwindow, 4.5+boxwidth, ypos, 2.5, 6, "v" );
   while (altdir != 0)
    {
     Otk_Add_Menu_Item( pulldown, altdir->word, Otk_fb_select_and_go, altdir->word );
     altdir = altdir->nxt;
    }
   Otk_Frame_Item( Otk_fbwindow, pulldown, 2.0 );
  }
 else boxwidth = 92;
 Otk_fb_filename_formbox = OtkMakeTextFormBox( Otk_fbwindow, filename, 60, 4, ypos, boxwidth, 6, Otk_fb_accept2, 0 );

 ypos = 89.75;
 OtkMakeButton( Otk_fbwindow, 4, ypos, 14, 5.5, " OK ", Otk_fb_accept, 0 );

 Otk_Set_Text_Aspect( 0.7 );
 OtkMakeTextLabel( Otk_fbwindow, "File Types:", Otk_Black, 1.2, 1, sqrt(dx) * (29.4/8.9), 91 );
 Otk_Set_Text_Aspect( 1.0 );
 Otk_fb_wildcard_formbox = OtkMakeTextFormBox( Otk_fbwindow, wildcards, 10, 41, ypos, 22, 5.5, Otk_fb_wildcard_accept, 0 );
 OtkMakeButton( Otk_fbwindow, 64.75, ypos, 10, 5.5, "Filter", Otk_fb_wildcard_accept, 0 );

 OtkMakeButton( Otk_fbwindow, 84, ypos, 11, 5.5, "Cancel ", Otk_fb_cancel, 0 );
 Otk_Display_Changed++;
 free(pathname);
 return Otk_fbwindow;
}


OtkWidget Otk_Browse_Files( char *prompt, int maxlength, char *directory, char *wildcards, char *filename, void (*callback)(char *fname) )
{
 if (Otk_fbwindow_state != 0) Otk_RemoveObject( Otk_fbwindow );
 Otk_fbwindow_state = 0;
 Otk_fb_callback = callback;
 Otk_fb_prompt = strdup( prompt );
 Otk_fb_maxlen = maxlength;
 Otk_fb_dnptr = directory;
 strcpy_safe(Otk_fb_wildcard,wildcards,500);
 Otk_fb_wcptr = wildcards;
 Otk_fb_fnptr = filename;
 Otk_fb_dirsonly = 0;
 return Otk_BrowseFiles0( Otk_fb_prompt, maxlength, directory, wildcards, filename, callback );
}


OtkWidget Otk_Browse_Dirs( char *prompt, int maxlength, char *directory, char *wildcards, char *dirname, void (*callback)(char *fdname) )
{
 if (Otk_fbwindow_state != 0) Otk_RemoveObject( Otk_fbwindow );
 Otk_fbwindow_state = 0;
 Otk_fb_callback = callback;
 Otk_fb_prompt = strdup( prompt );
 Otk_fb_maxlen = maxlength;
 Otk_fb_dnptr = directory;
 strcpy_safe(Otk_fb_wildcard,wildcards,500);
 Otk_fb_wcptr = wildcards;
 Otk_fb_fnptr = dirname;
 Otk_fb_dirsonly = 1;
 return Otk_BrowseFiles0( Otk_fb_prompt, maxlength, directory, wildcards, dirname, callback );
}

void Otk_fbrowse_allowfiles( int state )   /* Sets whether Otk_Browse_Dirs can return path */
{					   /* to a regular file, or only directories.	   */
 Otk_fb_allowfiles = state;  /* Zero=false (no-files, dirs-only), 1=true (allow files or directories). */
}

void Otk_fbrowse_showdotfiles( int state )   /* Sets whether file-browser shows files beginning with dot. */
{ Otk_fb_showdotfiles = state; }	     /* 0=no.  1=yes. */


void Otk_fbrowse_size_hint( float width, float height, float horiz_center_offset,  float vert_center_offset )
{
 otk_fb_width = width;
 otk_fb_height = height;
 otk_fb_hcenter_offset = horiz_center_offset;
 otk_fb_vcenter_offset = vert_center_offset;
}

/* --- --- --- --- --- --- End File Browser --- --- --- --- --- --- --- --- --- */
/********************************************************************************/

/* Enhancements for Animating Objects with Timer Mechanism. */

typedef struct OtkTimer OtkTimer;
struct OtkTimer
 {
  double time;	  // execution time
  double dt;	  // delay time between repeats
  double repeats; // number of repeats  -1 = infinite
  void (*func)( void *, int ); //function
  void *ud; 	  //function data
  OtkTimer *next; // ascending order of time
 } *otk_timers = NULL;


void otk_queue_timer( OtkTimer *timer )
{
  OtkTimer *tmr = otk_timers;

  if ( !otk_timers || otk_timers->time > timer->time )
   {
    timer->next = otk_timers;
    otk_timers = timer;
   }
  else
   {
    tmr = otk_timers;
    while ( tmr->next && tmr->next->time <= timer->time )
      tmr = tmr->next;
    if ( timer != tmr )
     {
        timer->next = tmr->next;
        tmr->next = timer;
     }
   }
}

void *otk_set_timer( double delay, double repeats, void (*func)(void*,int), void *ud )
{
  OtkTimer *timer = c_new0( OtkTimer, 1 );
  #if (PLATFORM_KIND==Posix_Platform)
   struct timeval tv;
   gettimeofday( &tv, NULL );
   timer->time = (double)tv.tv_sec + 1e-6*(double)tv.tv_usec + delay;
  #else
   SYSTEMTIME tv;
   GetSystemTime( &tv );
   timer->time = (double)tv.wSecond + 1e-6*(double)tv.wMilliseconds + delay;
  #endif
  timer->dt = delay;
  timer->repeats = repeats;
  timer->func = func;
  timer->ud = ud;
  otk_queue_timer( timer );
  return( (void *)timer );
}


void otk_cancel_timer( void *timer )
{
  OtkTimer *tmr;

  if ( !otk_timers ) return;
  if ( otk_timers == timer )
   {
    otk_timers = otk_timers->next;
    free( timer );
   }
  else
   {
    tmr = otk_timers;
    while( tmr->next && tmr->next != timer )
      tmr = tmr->next;
    if ( tmr->next )
     {
      tmr->next = tmr->next->next;
      free( timer );
     }
   }
}


void otk_timers_do()
{
  struct timeval tv;
  double tnow;
  OtkTimer *timer = otk_timers;

  #if (WinGLEnabled==0)
   gettimeofday( &tv, NULL );
  #endif
  tnow = tv.tv_sec + 1e-6*(double)tv.tv_usec;

  while( otk_timers && otk_timers->time <= tnow )
   {
    //dequeue head
    timer = otk_timers;
    otk_timers = otk_timers->next;
    timer->next = NULL;

    //execute and decide how/if to reinsert
    timer->func( timer->ud, timer->repeats );

    if ( timer->repeats > 0 )
     {
      timer->repeats--;
      timer->time += timer->dt;
      otk_queue_timer( timer );
     }
    else
     if ( timer->repeats == -1 )
      {
       timer->time += timer->dt;
       otk_queue_timer( timer );
      }
     else { otk_cancel_timer( timer ); }
   }
}



void otk_text_throb_func( OtkWidget text, int running )
{
#define FLT_EPSILONLRG 0.0001
#define FLT_EQ(x,y) ((y) - FLT_EPSILONLRG < (x) && (y) + FLT_EPSILONLRG > (x))
  if ( !running ) { text->color[3] = 1.0; } 
  else
  if ( FLT_EQ( fmod( text->color[3]*10.0, 1.0 ), 0.0 ) ||
	     FLT_EQ( fmod( text->color[3]*10.0, 1.0 ), 1.0 ) )
   {
    if ( text->color[3] > .1 ) { text->color[3] -= 0.1; } else { text->color[3] += .05; }
   }
  else
  if ( FLT_EQ( fmod( text->color[3]*10.0, 1.0 ), .5 ) )
   {
    if ( text->color[3] < .9 ) { text->color[3] += .1; } else { text->color[3] += .05; }
  } else {
    text->color[3] = .9;
  }
}


/**********************/
/* Otk layout widget. */
/**********************/


OtkWidget otk_layout_new( OtkWidget parent, float x, float y, float width,
			  float height, int ncols, int nrows, float xspacing,
			  float yspacing, float row_height )
{
  OtkWidget layout = OtkMakePanel( parent, 0, Otk_Black, x, y, width, height );
  layout->invisible = 1;
  layout->ncols = ncols;
  layout->nrows = nrows;
  layout->thickness = xspacing;
  layout->slant = yspacing;
  layout->sqrtaspect = row_height; // default row height
  layout->text = (char *)c_new0( float, ncols ); // widths
  layout->horiztextscroll = 0;	// current col
  layout->verttextscroll = 0;	// current row
  return( layout );
}

int otk_layout_row_index_get( OtkWidget layout )
{ return( layout->verttextscroll ); }

int otk_layout_col_index_get( OtkWidget layout )
{ return( layout->horiztextscroll ); }

OtkWidget otk_layout_add_row( OtkWidget layout, float height )
{
  OtkWidget lastrow = (OtkWidget)layout->font, row;
  float yspacing = layout->slant;
  float y = 0;

  if ( layout->verttextscroll >= layout->nrows )  layout->nrows++;
  if ( height == 0.0 )  height = layout->sqrtaspect;
  if ( lastrow )  y = lastrow->y2 + yspacing;

  // make row pallet
  row = OtkMakePanel( layout, 0, Otk_Black, 0.0, y, 100.0, height );
  Otk_object_detach( row );
  Otk_object_attach_at_end( layout, row );
  row->invisible = 1;
  layout->font = (OtkFont *)row; // save as current row

  layout->verttextscroll++;
  layout->horiztextscroll = 0;
  return( row );
}

OtkWidget otk_layout_add_col( OtkWidget layout, OtkWidget row, float width )
{
  OtkWidget cell;
  float *widths = (float *)layout->text;
  float xspacing = layout->thickness, usedwidth = 0.0, default_width, x = 0.0;
  int i, unspec = 0;

  if ( !row ) row = (OtkWidget)layout->font;

  if ( layout->horiztextscroll >= layout->ncols )
   {
    layout->ncols++;
    layout->text = realloc( widths, sizeof(float)*layout->ncols );
    widths = (float *)layout->text;
    widths[layout->ncols - 1] = 0.0;
   }

  if ( width > 0.0 || widths[layout->horiztextscroll] > 0.0 )
   {
    if( width > widths[layout->horiztextscroll] )
      widths[layout->horiztextscroll] = width;
    else
      width = widths[layout->horiztextscroll];
   }

  for ( i = 0; i < layout->ncols; i++ )
   {
    if( widths[i] == 0.0 ) unspec++;
    else usedwidth += widths[i];
   }
  usedwidth += (layout->ncols - 1)*xspacing;
  default_width = (100.0 - usedwidth)/(float)unspec;

  if( width <= 0.0 ) width = default_width;

  for( i = 0; i < layout->horiztextscroll; i++ )
    x += widths[i] == 0.0 ? widths[i] : default_width;
  x += layout->horiztextscroll*xspacing;

  // make cell
  cell = OtkMakePanel( row, 0, Otk_Black, x, 0.0, width, 100.0 );
  Otk_object_detach( cell );
  Otk_object_attach_at_end( row, cell );
  cell->invisible = 1;
  row->font = (OtkFont *)cell; // save as current row

  layout->horiztextscroll++;
  return( cell );
}

void otk_layout_reflow( OtkWidget layout )
{
  OtkWidget row, cell;
  float *widths = (float *)layout->text;
  float xspacing = layout->thickness, yspacing = layout->slant;
  float usedwidth = 0.0, default_width, width, x = 0.0, y = 0.0;
  int i, unspec = 0;

  if ( !layout->children ) return;

  for ( i = 0; i < layout->ncols; i++ )
   {
    if ( widths[i] == 0.0 ) unspec++;
    else usedwidth += widths[i];
   }
  usedwidth += (layout->ncols - 1)*xspacing;
  default_width = (100.0 - usedwidth)/(float)unspec;

  row = layout->children;
  while ( row )
   {
    if ( row->y1 != y )  Otk_move_object( row, 1, 0.0, y );
    x = 0.0;
    i = 0;
    cell = row->children;
    while ( cell )
     {
      width = widths[i] > 0.0 ? widths[i] : default_width;
      if ( cell->x1 != x || cell->x2 - cell->x1 != width )
       {
	cell->x1 = x;
	cell->x2 = x + width;
	Otk_object_correct_position( cell, 1 );
       }
      x += width + xspacing;
      i++;
      cell = cell->nxt;
     }
    y += row->y2 - row->y1 + yspacing;
    row = row->nxt;
   }
}



void Otk_check_window_level()
{ 
 OtkWidget objpt=OtkRootObject;
 float maxdepth=0.0;

 while (objpt!=0)
  { 
   if( !objpt->invisible )
    { /*!invisibile*/
      if (objpt->z > maxdepth) maxdepth = objpt->z;
    } /*!invisibile*/

   if( objpt->children )
     objpt = objpt->children;		/* Descend. */
   else
    {
     while( objpt && ! objpt->nxt )	/* Ascend if- and as- needed. */
       objpt = objpt->parent;
     if( objpt ) objpt = objpt->nxt;	/* Advance, if able. */
    }
  }

 Otk_window_level = maxdepth;	/* Maintain the minimum window-level. */
 if (Otk_window_level < 200.0) Otk_window_level = 200.0;
}


/********************************************************************************/


/* Removes all descendents and object. Returns next remaining object in list. */
OtkWidget Otk_RemoveObject( OtkWidget objt )
{
 OtkWidget tmppt, ptr;

 /* If window, remove it from top bar. */
 if ((objt->parent != 0) && (objt->superclass == Otk_SC_WindowPane)) objt = objt->parent;

 /* Now remove all children and children's children of the object. */
 tmppt = objt->children;
 while ( tmppt && tmppt != objt )
  {
   if( tmppt->children || tmppt->hidden_children )
    {
     /* Handle any hidden-children by adding them to the object's children list. */
     if( tmppt->hidden_children )
      {
       ptr = tmppt->hidden_children;
       while( ptr->nxt ) ptr = ptr->nxt;
       ptr->nxt = tmppt->children;
       tmppt->children = tmppt->hidden_children;
       tmppt->hidden_children = NULL;
      }
     tmppt = tmppt->children;	/* Descend. */
    }
   else
    {
     tmppt->parent->children = tmppt->nxt;
     ptr = tmppt;
     if( tmppt->nxt )
        tmppt = tmppt->nxt;	/* Advance the list. */
     else
        tmppt = tmppt->parent;	/* Ascend. */
       
     if (ptr==Otk_Selected_Item) Otk_Selected_Item = 0;
     if (ptr==Otk_ClickedObject) Otk_ClickedObject = 0;
     if (ptr==Otk_keyboard_focus) Otk_keyboard_focus = 0;
     if (ptr==otk_highlighted_text) otk_highlighted_text = 0;
     if (ptr==otk_last_scrollable) otk_last_scrollable = 0;
     if (ptr->attribs & Otk_hover_mask) Otk_Unregister_Hover_Box( ptr );
     if (ptr->text) free( ptr->text );
     free( ptr );
    }
  }

 /* Finally, remove the base object itself. */
 if (objt->attribs & Otk_hover_mask) Otk_Unregister_Hover_Box( objt );
 Otk_object_detach_any( objt );
 if (objt->superclass == Otk_SC_TextLabel) free( objt->text );
 if (objt==Otk_Selected_Item) Otk_Selected_Item = 0;
 if (objt==Otk_ClickedObject) Otk_ClickedObject = 0;
 if (objt==Otk_keyboard_focus) Otk_keyboard_focus = 0;
 if (objt==otk_highlighted_text) otk_highlighted_text = 0;
 if (objt==otk_last_scrollable) otk_last_scrollable = 0;
 tmppt = objt->nxt;
 free(objt);
 Otk_Display_Changed++;
 return tmppt;
}


OtkWidget Otk_PermObjBoundary=0;

void Otk_ClearAll()
{
 printf("\nCLEARING ALL OBJECTS\n");
 if (Otk_PermObjBoundary!=0)
  {
   while (OtkRootObject!=Otk_PermObjBoundary)
    Otk_RemoveObject( OtkRootObject );
  }
 else
  {
   while (OtkRootObject != 0)
    Otk_RemoveObject( OtkRootObject ); 
  }
}


void Set_PermanentObjs()
{
 Otk_PermObjBoundary = OtkRootObject;
 // printf("Boundary = %x\n", Otk_PermObjBoundary );
}

/********************************************************************************/


struct otk_tmp_callitem
 {
  int callitem;
  struct otk_tmp_callitem *nxt;
 } *otk_tmp_callitems=0;


void otk_push_callitem( int id )
{
 struct otk_tmp_callitem *newitem;
 newitem = (struct otk_tmp_callitem *)malloc(sizeof(struct otk_tmp_callitem));
 newitem->nxt = otk_tmp_callitems;
 otk_tmp_callitems = newitem;
 newitem->callitem = id;
}


int otk_snapshot_size=1, otk_make_movie=0, otk_movieframe=0;


int Otk_SnapShot_Size( int sz )		/* Sets snap-shot size: 1=full-size, 2=half-size. */
{
 int prevsz=otk_snapshot_size;
 otk_snapshot_size = sz;
 return prevsz;
}


void Otk_Snapshot( char *fname )	/* Write PPM image file of current screen. */
{
 FILE *fp;
 GLvoid *pixels;
 unsigned char *curpix;
 GLint swapbytes, lsbfirst, rowlength;
 GLint skiprows, skippixels, alignment;
 int j, k, m, n, p, q, r, nrows, ncols, sum;

 pixels = (GLvoid *)malloc( OtkWindowSizeX * OtkWindowSizeY * 3 );
 if (pixels==0) { printf("Otk: Cannot allocate memory.\n");  return; }
 glGetIntegerv( GL_PACK_SWAP_BYTES, &swapbytes );
 glGetIntegerv( GL_PACK_LSB_FIRST, &lsbfirst );
 glGetIntegerv( GL_PACK_ROW_LENGTH, &rowlength );
 glGetIntegerv( GL_PACK_SKIP_ROWS, &skiprows );
 glGetIntegerv( GL_PACK_SKIP_PIXELS, &skippixels );
 glGetIntegerv( GL_PACK_ALIGNMENT, &alignment );
 glPixelStorei( GL_PACK_SWAP_BYTES, GL_FALSE );
 glPixelStorei( GL_PACK_LSB_FIRST, GL_FALSE );
 glPixelStorei( GL_PACK_ROW_LENGTH, 0 );
 glPixelStorei( GL_PACK_SKIP_ROWS, 0 );
 glPixelStorei( GL_PACK_SKIP_PIXELS, 0 );
 glPixelStorei( GL_PACK_ALIGNMENT, 1 );
 glReadPixels( 0, 0, OtkWindowSizeX, OtkWindowSizeY, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)pixels );
 glPixelStorei( GL_PACK_SWAP_BYTES, swapbytes);
 glPixelStorei( GL_PACK_LSB_FIRST, lsbfirst );
 glPixelStorei( GL_PACK_ROW_LENGTH, rowlength );
 glPixelStorei( GL_PACK_SKIP_ROWS, skiprows );
 glPixelStorei( GL_PACK_SKIP_PIXELS, skippixels );
 glPixelStorei( GL_PACK_ALIGNMENT, alignment );
 curpix = (unsigned char *)pixels;
 m = 3 * OtkWindowSizeX;

 fp = fopen( fname, "wb" );
 if (fp==0) { printf("Otk: Cannot open '%s' for writing.\n", fname );  return; }
 fprintf(fp,"P6\n");
 if (otk_snapshot_size == 1)
  { /*Full-size*/
   fprintf(fp,"%d %d\n255\n", OtkWindowSizeX, OtkWindowSizeY );
   n = (OtkWindowSizeY-1) * m;
   for (k=OtkWindowSizeY-1; k >= 0; k--)
    {
     for (j=0;  j < m;  j++)  fputc( curpix[n + j], fp );
     n = n - m;
    }
  } /*Full-size*/
 else
  { /* Write half-sized PPM image file of current screen. */
    nrows = (OtkWindowSizeY - 1) >> 1;	/* Half-sized save is faster, and anti-aliased for good quality appearance. */
    ncols = (OtkWindowSizeX - 1) >> 1;
    fprintf(fp,"%d %d\n255\n", ncols, nrows );
    n = (OtkWindowSizeY - 2) * m;
    r = 2 * m;
    for (k=0; k < nrows; k++)
     { /*k*/
       p = n + 3;
       for (j=0;  j < ncols;  j++)
        { /*j*/
          for (q=0; q < 3; q++)
	   { /*q*/
	     sum = (curpix[p-3+m] + curpix[p+3+m] + curpix[p-3-m] + curpix[p+3-m]) >> 1;
	     sum = (curpix[p] + ((curpix[p-3] + curpix[p+3] + curpix[p-m] + curpix[p+m] + sum) >> 1)) >> 2;
	     fputc( sum, fp );
	     p++;
	   } /*q*/
	  p = p + 3;
	} /*j*/
       n = n - r;
     } /*k*/
  }
 free(pixels);
 fclose(fp);
}


void Otk_MakeMovie( int on_notoff )  /* Turns movie frame-capture on=1 or off=0. */
{
 otk_make_movie = on_notoff;
}


void otk_snapmovieframe()
{
 char fname[128]="frame0000000", wrd[64];
 int j;
 sprintf(wrd,"%d.ppm", otk_movieframe++ );
 j = strlen(wrd);
 if (j > 12) j = 12;
 fname[17-j] = '\0';
 strcat( fname, wrd );
 Otk_Snapshot( fname ); 
}


unsigned char *otk_aapix=0;
int otk_aabuff_sz=-1;
double otk_aatime=0.0;
int otk_render_calllist=1938738, otk_last_render_quality=0;


void otk_antialias()	/* Guass-blur. */
{
 GLint swapbytes, lsbfirst, rowlength;
 GLint skiprows, skippixels, alignment, upckalignment;
 int row, col, nrows, ncols, k, drow, aasize, kk;
 float rastpos[4];
 double t0;

 // printf("	CPU AntiAliasing\n");
 aasize = OtkWindowSizeX * OtkWindowSizeY * 3;
 if (aasize != otk_aabuff_sz)
  {
   if (otk_aapix != 0) { free( otk_aapix ); }
   glGetFloatv( GL_CURRENT_RASTER_POSITION, rastpos );
   if ((rastpos[1] != 0.0) || (rastpos[0] != 0.0))
    {
     otk_render_qual_level = 0;
     otk_aabuff_sz = -1;
     return;
    }
   otk_aabuff_sz = aasize; 
   otk_aapix = (unsigned char *)malloc( otk_aabuff_sz );
  }
 if (otk_aapix==0) {printf("Otk: Cannot allocate.\n");  return;}

 glGetIntegerv( GL_PACK_SWAP_BYTES, &swapbytes );
 glGetIntegerv( GL_PACK_LSB_FIRST, &lsbfirst );
 glGetIntegerv( GL_PACK_ROW_LENGTH, &rowlength );
 glGetIntegerv( GL_PACK_SKIP_ROWS, &skiprows );
 glGetIntegerv( GL_PACK_SKIP_PIXELS, &skippixels );
 glGetIntegerv( GL_PACK_ALIGNMENT, &alignment );
 glPixelStorei( GL_PACK_SWAP_BYTES, GL_FALSE );
 glPixelStorei( GL_PACK_LSB_FIRST, GL_FALSE );
 glPixelStorei( GL_PACK_ROW_LENGTH, 0 );
 glPixelStorei( GL_PACK_SKIP_ROWS, 0 );
 glPixelStorei( GL_PACK_SKIP_PIXELS, 0 );
 glPixelStorei( GL_PACK_ALIGNMENT, 1 );
 t0 = otk_report_time();

 glReadPixels( 0, 0, OtkWindowSizeX, OtkWindowSizeY, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)otk_aapix );
 if ((otk_aatime + otk_report_time() - t0) * otk_render_quality_hint < 0.5 )
  { unsigned char oldred0, oldgrn0, oldblu0, oldred1, oldgrn1, oldblu1;
   drow = OtkWindowSizeX + OtkWindowSizeX + OtkWindowSizeX;
   // dcol = drow * (OtkWindowSizeY - 1);
   nrows = OtkWindowSizeY - 1;
   ncols = OtkWindowSizeX - 1;
   k = drow + 3;
   kk = k + drow ;
   for (row=1;  row != nrows;  row++)
    {
     oldred0 = otk_aapix[k-3];
     oldgrn0 = otk_aapix[k-2];
     oldblu0 = otk_aapix[k-1];
     for (col=1;  col != ncols;  col++)
      {
       oldred1 = otk_aapix[k];
       otk_aapix[k] = (otk_aapix[k] + ((otk_aapix[k+3] + otk_aapix[kk] + otk_aapix[k] + oldred0) >> 2)) >> 1;
       k++;
       oldred0 = oldred1;
       kk++;
       oldgrn1 = otk_aapix[k];
       otk_aapix[k] = (otk_aapix[k] + ((otk_aapix[k+3] + otk_aapix[kk] + otk_aapix[k] + oldgrn0) >> 2)) >> 1;
       k++;
       oldgrn0 = oldgrn1;
       kk++;
       oldblu1 = otk_aapix[k];
       otk_aapix[k] = (otk_aapix[k] + ((otk_aapix[k+3] + otk_aapix[kk] + otk_aapix[k] + oldblu0) >> 2)) >> 1;
       k++;
       oldblu0 = oldblu1;
       kk++;
      }
     k = k + 6;
     kk = kk + 6;
    }
   glGetIntegerv( GL_PACK_ALIGNMENT, &upckalignment );
   glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

   if ( (otk_aatime + otk_report_time() - t0) * otk_render_quality_hint < 0.8 )
     glDrawPixels( OtkWindowSizeX, OtkWindowSizeY, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)otk_aapix );
   else
     otk_render_qual_level = 0;
   glPixelStorei( GL_PACK_ALIGNMENT, upckalignment );
  }
 else otk_render_qual_level = 0;

 if (otk_render_qual_level) 
  otk_aatime  = 0.5 * (otk_aatime + otk_report_time() - t0);	/* Average out time performance measurements. */
 glPixelStorei( GL_PACK_SWAP_BYTES, swapbytes);
 glPixelStorei( GL_PACK_LSB_FIRST, lsbfirst );
 glPixelStorei( GL_PACK_ROW_LENGTH, rowlength );
 glPixelStorei( GL_PACK_SKIP_ROWS, skiprows );
 glPixelStorei( GL_PACK_SKIP_PIXELS, skippixels );
 glPixelStorei( GL_PACK_ALIGNMENT, alignment );
}



#define Otk_refresh 1	 /* For stand-alone use.  Otk refreshes canvas each frame. */
#define Otk_norefresh 0	 /* For use within other OpenGL apps.  Other app refreshes frame. */


void Otk_swap_buffers()
{
   #if (GlutEnabled==0)
    #if (WinGLEnabled==0)
     if (OtkDoubleBuffer) glXSwapBuffers( Otkdpy, Otkwin );  else  glFlush();
    #else
      if (OtkDoubleBuffer) SwapBuffers( Otkdpy );  else  glFlush();
    #endif
   #else
    glutSwapBuffers();
   #endif
}





void OtkDrawAll_scene( int refresh )
{ int errt, clipping=0;
 OtkWidget objpt=OtkRootObject;
 float maxdepth=0.0;
 OtkClipList *cliplist=NULL, *clipto;

 if (refresh)
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

 // printf("RenderQual = %d\n", otk_render_qual_level );
 DEBUG printf("		Redrawing %d\n", Otk_Display_Changed);
 otk_last_render_quality = otk_render_qual_level;

 glEnable( GL_SCISSOR_TEST );
 glScissor( 0, 0, OtkWindowSizeX, OtkWindowSizeY );

 while (objpt!=0)
  { 
   if ( objpt->scissor_to_parent && objpt->parent )
    {
     clipto = (OtkClipList *)malloc( sizeof(OtkClipList) );
     clipto->nxt = cliplist;
     if ( cliplist )
      {
       clipto->xleft = MAX( cliplist->xleft, objpt->parent->xleft );
       clipto->xright = MIN( cliplist->xright, objpt->parent->xright );
       clipto->ytop = MAX( cliplist->ytop, objpt->parent->ytop );
       clipto->ybottom = MIN( cliplist->ybottom, objpt->parent->ybottom );
      }
     else
      {
       clipto->xleft = objpt->parent->xleft;
       clipto->xright = objpt->parent->xright;
       clipto->ytop = objpt->parent->ytop;
       clipto->ybottom = objpt->parent->ybottom;
      }
     cliplist = clipto;

     glPushAttrib( GL_SCISSOR_BIT );
     glScissor( cliplist->xleft*((float)OtkWindowSizeX)*.01,
		(100.0 - cliplist->ybottom)*((float)OtkWindowSizeY)*.01,
		(cliplist->xright - cliplist->xleft)*((float)OtkWindowSizeX)*.01,
		(cliplist->ybottom - cliplist->ytop)*((float)OtkWindowSizeY)*.01 );
    }
   else
    if (objpt->scissor_parent != 0)
     {
       glPushAttrib( GL_SCISSOR_BIT );
       glScissor( objpt->scissor_parent->xleft * ((float)OtkWindowSizeX) * 0.01,
                  (100.0 - objpt->scissor_parent->ybottom) * ((float)OtkWindowSizeY) * 0.01,
                  (objpt->scissor_parent->xright - objpt->scissor_parent->xleft) * ((float)OtkWindowSizeX) * 0.01,
                  (objpt->scissor_parent->ybottom - objpt->scissor_parent->ytop) * ((float)OtkWindowSizeY) * 0.01 );
        clipping = 1;
     }

   if( !objpt->invisible )
    { /*!invisibile*/
     glPushMatrix();
     // if (Otk_snifferrors(2002)) {printf("ERROR: Drawing %s\n",objpt->name); }
     //     glCallList( objpt->TypeId );

     switch (objpt->object_class)	/* 1=panel, 2=text_label, 3=button, 4=form-box, 5=line. */
      {
       case Otk_class_panel:	/* Panel. */
		if (((refresh==Otk_refresh) || (objpt!=OtkOuterWindow)) && (objpt->object_subtype!=Otk_Invisible))
		 Otk_Draw_Panel( objpt );
		break;
       case Otk_class_text:  /* Text. */
		Otk_Draw_Text( objpt );
		break;
       case Otk_class_button:	/* Button. */
		Otk_Draw_Panel( objpt );
		break;
       case Otk_class_line:  /* Line. */
		Otk_Draw_Line( objpt );
		break;
       case Otk_class_box:  /* Bounding box. */
		Otk_Draw_Box( objpt );
		break;
       case Otk_class_radiobutton1: case Otk_class_radiobutton2: case Otk_class_togglebutton: Otk_Draw_Panel( objpt );
		break;
       case Otk_class_triangle: Otk_Draw_Triangle( objpt );
		break;
       case Otk_class_disk: Otk_Draw_Disk( objpt );
		break;
       case Otk_class_circle: Otk_Draw_Circle( objpt );
		break;
       case Otk_class_userdefd:  otk_push_callitem( objpt->Id );
		break;
       case Otk_class_other:
		break;
       default: printf("unknown draw object %d\n", objpt->object_class);
      }

     glPopMatrix();
     if (Otk_snifferrors(2003)) 
      { printf("Otk ERROR: Drawing %d\n",objpt->object_class); 
        printf("%d %d\n", glGetError(), GL_NO_ERROR);
      }
    } /*!invisibile*/

   if (objpt->z > maxdepth) maxdepth = objpt->z;
   if( objpt->children )
    {
     objpt = objpt->children;					/* Descend. */
    }
   else
    {
     while( objpt && ! objpt->nxt )	/* Ascend if- and as- needed. */
      {
       if( objpt && objpt->scissor_to_parent && objpt->parent )
	{
	 glPopAttrib();
	 clipto = cliplist;
	 cliplist = cliplist->nxt;
	 free( clipto );
	 if (Otk_snifferrors(2004)) 
	  {
	   printf("Otk ERROR: removing clip\n"); 
	   printf("%d %d\n", glGetError(), GL_NO_ERROR);
	  }
	}
       objpt = objpt->parent;
      }
     if( objpt && objpt->scissor_to_parent && objpt->parent )
      {
       glPopAttrib();
       clipto = cliplist;
       cliplist = cliplist->nxt;
       free( clipto );
       if (Otk_snifferrors(2005)) 
	{
	 printf("Otk ERROR: removing clip\n"); 
	 printf("%d %d\n", glGetError(), GL_NO_ERROR);
	}
      }
     if( objpt ) objpt = objpt->nxt;				/* Advance, if able. */
    }
    if (clipping != 0)
     {
       clipping = 0;
       glPopAttrib();
     }
  }

 if (otk_tmp_callitems)
  {
   /* Enable transparency. */
   glEnable( GL_BLEND );
   while (otk_tmp_callitems)
    { struct otk_tmp_callitem *oldptr;
      glCallList( otk_tmp_callitems->callitem );
      oldptr = otk_tmp_callitems;
      otk_tmp_callitems = otk_tmp_callitems->nxt;
      free(oldptr);  
    }
   glDisable( GL_BLEND );
  }

 Otk_window_level = maxdepth + 10.0;	/* Maintain the minimum window-level. */
 if (Otk_window_level < 200.0) Otk_window_level = 200.0;

 /* If the keyboard has focus, then handle the text-box cursor. */
 if ((Otk_keyboard_focus) && (!otk_highlighted_text))
  {
   DEBUG printf("			Checking keybrd focus\n"); 
   Otk_Keyboard_state.cursor_blink_count++;
//   if (Otk_Keyboard_state.cursor_blink_count > 1)
    {	/* Display the text-cursor. */
     float w, ht, x, y, point[3], cursortop, cursorbottom, dv;

     Otk_Get_Character_Size( Otk_keyboard_focus, &w, &ht );

     x = ((float)(Otk_Keyboard_state.textform_insertion_column - Otk_keyboard_focus->horiztextscroll) - 0.07) * w + Otk_keyboard_focus->xleft;
     if (x > Otk_keyboard_focus->parent->xright) 
      { x = Otk_keyboard_focus->parent->xright - 0.2 * w;
	Otk_keyboard_focus->horiztextscroll++;
      } else
     if (x < Otk_keyboard_focus->parent->xleft) 
      { x = Otk_keyboard_focus->parent->xleft + 0.2 * w;
	Otk_keyboard_focus->horiztextscroll--;
      }

     if (Otk_keyboard_focus->nrows <= 1)
      { /*formbox*/
	cursortop = - Otk_keyboard_focus->ytop + 0.05 * ht;
        cursorbottom = - Otk_keyboard_focus->ytop - 0.55 * ht;
      } /*formbox*/
     else
      { /*texteditbox*/

	ht = 0.4 * ht;	/* Consider text height to be smaller than reported, to magnify it slightly. */
	dv = (Otk_keyboard_focus->parent->ybottom - Otk_keyboard_focus->parent->ytop) / (float)((Otk_keyboard_focus->nrows) + 0.5);

        y = ((float)(Otk_Keyboard_state.textform_insertion_row - Otk_keyboard_focus->verttextscroll) - 0.07) * dv + Otk_keyboard_focus->ytop;
        while (y > Otk_keyboard_focus->parent->ybottom) 
         { Otk_keyboard_focus->verttextscroll++;
	   y = ((float)(Otk_Keyboard_state.textform_insertion_row - Otk_keyboard_focus->verttextscroll) - 0.07) * dv + Otk_keyboard_focus->ytop;
	 }
        while (y < Otk_keyboard_focus->parent->ytop)
	 { Otk_keyboard_focus->verttextscroll--;
	   y = ((float)(Otk_Keyboard_state.textform_insertion_row - Otk_keyboard_focus->verttextscroll) - 0.07) * dv + Otk_keyboard_focus->ytop;
	 }

	cursortop = - Otk_keyboard_focus->ytop + (0.05 - (Otk_Keyboard_state.textform_insertion_row - Otk_keyboard_focus->verttextscroll)) * dv;
	cursorbottom = - Otk_keyboard_focus->ytop - (0.55 + (Otk_Keyboard_state.textform_insertion_row - Otk_keyboard_focus->verttextscroll)) * dv;
      } /*texteditbox*/


     glColor4fv( Otk_BLACK );	/* Place and Display the Text-Cursor. */
     glLineWidth( 1.0 );
     glBegin( GL_LINES );	/* Top of cursor. */
      point[1] = cursortop;
      point[2] = Otk_keyboard_focus->z;

      point[0] = x - 0.15 * w;
      glVertex3fv( point );
      point[0] = x + 0.2 * w;
      glVertex3fv( point );

      point[0] = x;
      glVertex3fv( point );	/* Bottom of cursor. */
      point[1] = cursorbottom; 
      glVertex3fv( point );

      point[0] = x - 0.15 * w;
      glVertex3fv( point );
      point[0] = x + 0.2 * w;
      glVertex3fv( point );
     glEnd();
    }
//   else
//    if (Otk_Keyboard_state.cursor_blink_count>2)
//     Otk_Keyboard_state.cursor_blink_count = 0;
  }

 glDisable( GL_SCISSOR_TEST );

 if (otk_render_qual_level == 1)
  otk_antialias();
 
 if (refresh)
  {
   errt = glGetError();
   if (errt!=0) printf("GLError: %d\n", errt);
   #if (GlutEnabled==0)
    #if (WinGLEnabled==0)
     if (OtkDoubleBuffer) glXSwapBuffers( Otkdpy, Otkwin );  else  glFlush();
    #else
      if (OtkDoubleBuffer) SwapBuffers( Otkdpy );  else  glFlush();
    #endif
   #else
    glutSwapBuffers();
   #endif
  }
}





int otk_fullscrn_antialias[]={4,9,16,0}, otk_antialias_indx;	/* 0=direct, 1=cpu-antialias, 2=4pt-accumbuf, 3=9pt-accumbuf, 4=16pt-accumbuf. */
float otk_aa_jitter_xarray[3][16]=
	{
	 {0.25, 0.75, 0.25, 0.75},					/* 2x2 kernel (4) */
	 {0.167, 0.5, 0.833, 0.167, 0.5, 0.833, 0.167, 0.5, 0.833 },	/* 3x3 kernel (9) */
	 {0.125, 0.375, 0.625, 0.875, 0.125, 0.375, 0.625, 0.875, 
	  0.125, 0.375, 0.625, 0.875, 0.125, 0.375, 0.625, 0.875, }	/* 4x4 kernel (16) */
	},
       otk_aa_jitter_yarray[3][16]=
	{
	 {0.25, 0.25, 0.75, 0.75},					/* 2x2 kernel (4) */
	 {0.167, 0.167, 0.167, 0.5, 0.5, 0.5, 0.833, 0.833, 0.833 },	/* 3x3 kernel (9) */
	 {0.125, 0.125, 0.125, 0.125, 0.375, 0.375, 0.375, 0.375,
	  0.625, 0.625, 0.625, 0.625, 0.875, 0.875, 0.875, 0.875 }	/* 4x4 kernel (16) */
	};
int otk_suspend_quality_adjustments=0;



void OtkDrawAll( int refresh )
{ int jndx;
  float hscale, vscale, ascale, frametime;
  GLint otk_viewport[4];
  double T0;

 T0 = otk_report_time();
 if (otk_render_qual_level > otk_highest_qual_tried) otk_highest_qual_tried = otk_render_qual_level;

 if ((otk_render_qual_level < 2) || (refresh == 0))
  OtkDrawAll_scene( refresh );
 else
  { /* Accumulation-buffer based anti-aliasing to improve rendering quality. */
   // printf("\nAccumBuf Antialiasing: %d\n", otk_fullscrn_antialias[otk_antialias_indx]);
   if (otk_render_calllist == 1938738) otk_render_calllist = glGenLists( 1 );
   otk_antialias_indx = otk_render_qual_level - 1;
   glGetIntegerv( GL_VIEWPORT, otk_viewport );
   glClear( GL_ACCUM_BUFFER_BIT );
   hscale = 100.0 / otk_viewport[2];
   vscale = 100.0 / otk_viewport[3];
   ascale = 1.0 / (float)otk_fullscrn_antialias[otk_antialias_indx];
   for (jndx=0;  jndx < otk_fullscrn_antialias[otk_antialias_indx];  jndx++)
    { /*jitter*/
     glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
     glPushMatrix();
     glTranslatef( otk_aa_jitter_xarray[otk_antialias_indx][jndx] * hscale, otk_aa_jitter_yarray[otk_antialias_indx][jndx] * vscale, 0.0 );
     if (jndx == 0)
      { /*buildcalllist*/
	glNewList( otk_render_calllist, GL_COMPILE ); 
	 OtkDrawAll_scene(0);
	glEndList();
	glCallList( otk_render_calllist );
       } /*buildcalllist*/
      else
       glCallList( otk_render_calllist );
     glPopMatrix();
     glAccum( GL_ACCUM, ascale );
    } /*jitter*/		 
   glAccum( GL_RETURN, 1.0 );
   Otk_swap_buffers();
  }
 // printf("FrameTime = %g mS, OTK_FRAME_PERIOD=%g\n", 1000.0 * (otk_report_time() - T0), OTK_FRAME_PERIOD );

 if ((! otk_suspend_quality_adjustments) && (refresh))
  { /* Update display quality settings. */
   frametime = otk_report_time() - T0;
   if (frametime > OTK_FRAME_PERIOD)
    frametime = otk_render_quality_hint * (0.65 * OTK_FRAME_PERIOD + 0.35 * frametime);	 /* Benefit of the doubt. */
   else
    frametime = otk_render_quality_hint * (0.4 * (OTK_FRAME_PERIOD + 0.6 * frametime));	 /* Fast decay. */
   if (frametime < 0.05) 
    { /*quick*/
      if ((allow_cpu_antialiasing) && (otk_render_qual_level == otk_highest_qual_tried) && (otk_render_qual_level < 3) && ((otk_got_accumbuf) || (otk_render_qual_level == 0)) )
       { /* Fast display, try improving quality. */
	 otk_render_qual_level++;
	 if (frametime < 0.025) OtkDrawAll( refresh );	/* If really-fast, re-draw immediately at higher quality. */
       }
    } /*quick*/
   else
   if ((otk_render_qual_level > 0) && (frametime > 0.2))	/* Slow display, reduce quality. */
    { /*slow*/
     otk_render_qual_level--;
    } /*slow*/
  }

 #if (WinGLEnabled)
  SetCursor( otk_mouse_cursor_font[ otk_current_mouse_cursor_style ] );   
 #endif
}



void otk_update_draw_object( OtkWidget objpt )	/* Redraw just the given object.  For rapid updates when */
{						/* nothing else changed, and object is guaranteed to be on top. */
 struct otk_tmp_callitem *old_callitems;	/* Leverage regular draw routine, by temporarily setting object */
 OtkWidget old_rootobjpt, old_parent, old_nxt;	/* as the entire object-tree. */
 int qual_level;

 otk_suspend_quality_adjustments = 1;
 if (otk_render_qual_level == 1)
  {
   if (otk_render_quality_hint * OTK_FRAME_PERIOD < 0.1)
    {
      OtkDrawAll( Otk_refresh );	/* Fast enough to update + antialias whole screen. */
      Otk_Display_Changed = 0;
      otk_suspend_quality_adjustments = 0;
      return;
    }
   else
    {
     qual_level = otk_render_qual_level;
     otk_render_qual_level--;		/* Just quick-display this object. */
    }
  }
 else qual_level = otk_render_qual_level;

 /* Update just the object which must be at top layer. */
 old_callitems = otk_tmp_callitems;
 otk_tmp_callitems = 0;
 old_parent = objpt->parent;
 objpt->parent = 0;
 old_nxt = objpt->nxt;
 objpt->nxt = 0;
 old_rootobjpt = OtkRootObject;
 OtkRootObject = objpt;
 glClear( GL_DEPTH_BUFFER_BIT );
 Otk_swap_buffers();
 OtkDrawAll( 0 ); 		/* Re-draw object and its children. */
 Otk_swap_buffers();
 glFlush();
 OtkRootObject = old_rootobjpt;
 objpt->parent = old_parent;
 objpt->nxt = old_nxt;
 otk_tmp_callitems = old_callitems;
 otk_render_qual_level = qual_level;
 otk_suspend_quality_adjustments = 0;
}






void OtkPrintfObjectTreeRecursive(OtkWidget objpt, int level)
{
 OtkWidget tmpobj;
 int i;

 for (i=0; i<=level; i++) printf("  ");		/* Indent. */

 switch (objpt->superclass)	/* 1=panel, 2=text_label, 3=button, 4=form-box, 5=line. */
  {
       case Otk_SC_Panel:	/* Panel. */
		printf("Panel (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_TextLabel:  /* Text. */
		printf("Text (%d %d %d) [%lx] = '%s'", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt, objpt->text );
		break;
       case Otk_SC_Button:	/* Button. */	case Otk_SC_TabHandle:
		printf("Button (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_FormBox:	/* Form-Box. */
		printf("FormBox (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_Line:  /* Line. */
		printf("Line (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_Box:  /* Box. */
		printf("Box (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_hSlider:	/* Horizontal Slider. */
		printf("Horizontal Slider (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_vSlider:	/* Vertical Slider. */
		printf("Vertical Slider (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_Menu_DropDown_Button: /* Drop-down Menu Button */
		printf("Menu (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_Menu_Item: /* Menu item. */
		printf("Menu Item (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		printf("	Coords %g %g %g", objpt->xleft, objpt->ytop, objpt->z);
		if (objpt->object_class == Otk_class_text) printf("	'%s'\n", objpt->text );
		break;
       case Otk_SC_Menu_Submenu: 
		printf("SubMenu (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_Select_List: /* Selection List */
		printf("Selection List (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_Select_List_Item: /* Selection list item. */
		printf("Selection List  Item (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		printf("	Coords %g %g %g", objpt->xleft, objpt->ytop, objpt->z);
		break;
       case Otk_SC_RadioButton: 
		printf("Radio Button (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_ToggleButton: 
		printf("Toggle Button (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_textscrollbar: 
		printf("Text Scrollbar (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_SC_Window:  case Otk_SC_WindowPane:
		printf("Window (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_class_disk:
		printf("Disk (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       case Otk_class_circle:
		printf("Circle (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
		break;
       default: printf("Unknown  (%d %d %d) [%lx]", objpt->superclass, objpt->object_class, objpt->object_subtype, (unsigned long)objpt);
  }

 printf(" {xl=%g,xr=%g,yt=%g,yb=%g,z=%g}", objpt->xleft, objpt->xright, objpt->ytop, objpt->ybottom, objpt->z );
 if (objpt==Otk_keyboard_focus) printf("    <==== KeyBoardFocus !!!");
 printf("\n");

 if (level>1000) {printf("\n--- Truncating recursion, exceeded depth of 1,000. ---\n\n"); return;}
 tmpobj = objpt->children;
 while (tmpobj!=0)
  {
   OtkPrintfObjectTreeRecursive( tmpobj, level + 1 );
   tmpobj = tmpobj->nxt;
  }
}


void OtkDrawObjectTree()	/* Diagnostic.  For debugging. */
{ /*DrawObjectTree*/
 printf("\nOTK Object Tree:\n");
 OtkPrintfObjectTreeRecursive( OtkOuterWindow, 1 );
 printf("\n");
} /*DrawObjectTree*/






/*================================================================*/

 char Otk_tmpstr[20000];
 int Otk_MenuDepth=0;
 OtkWidget otk_pulldown_mousehighlight=0;
 OtkColorVector otk_pulldown_mhlight_prev_panel_color, otk_pulldown_mhlight_prev_text_color;

/*================================================================*/


void (*Otk_MouseClick_Callback)(int state)=0;
void (*Otk_MouseMove_Callback)()=0;
int (*Otk_Keyboard_Callback)(char key)=0;	/* Return 1 if application callback wants to intercept key-press, */
						/* else return 0 if app wants to pass key-press onto Otk. */

void Otk_Register_MouseClick_Callback( void (*callback)(int state) )
{ Otk_MouseClick_Callback = callback; }

void Otk_Register_MouseMove_Callback( void (*callback)() )
{ Otk_MouseMove_Callback = callback; }

void Otk_Register_Keyboard_Callback( int (*callback)(char key) )
{ Otk_Keyboard_Callback = callback; }


int otk_restore_mousemenuhighlight()
{
 OtkWidget kid;

 if (otk_pulldown_mousehighlight)
  { /* Restore old colors to previously highlighted item. */
   OtkCopyColor( otk_pulldown_mousehighlight->color, otk_pulldown_mhlight_prev_panel_color );
   kid = otk_pulldown_mousehighlight->children;
   while (kid != 0)
    {
     if (kid->object_class == Otk_SC_TextLabel)
      {
       OtkCopyColor( kid->color, otk_pulldown_mhlight_prev_text_color );
      }
     kid = kid->nxt;
    }
   otk_pulldown_mousehighlight = 0;
   return 1;
  }
 else return 0;
}


void Otk_InflateMenu( OtkWidget objpt )		/* Expand or 'elaborate' menu. */
{
 OtkWidget tmplst, tp1 = NULL, tp2;
 int k, maxlen=0, count=0, first=1;
 float width, w, v, lside, rside;

 otk_restore_mousemenuhighlight();

 /* First move any children out of hiding. */
 if( objpt->children )
  {
   tp2 = objpt->children;
   while( tp2->nxt ) tp2 = tp2->nxt;
   /* Now tp2 points to the last item on the visible-children list.  Attach the hidden ones to it. */
   tp2->nxt = objpt->hidden_children;
  }
 else
   objpt->children = objpt->hidden_children;
 objpt->hidden_children = NULL;

 /* Now determine the menu width. */
 tmplst = objpt->children;
 while (tmplst!=0)
  {
   if ((tmplst->superclass==Otk_SC_Menu_DropDown_Button) || (tmplst->superclass==Otk_SC_Menu_Item))
    {
	k = strlen(tmplst->children->text);
	if (k>maxlen) maxlen = k;
	count++;
    }
   else if ((tmplst->superclass==Otk_SC_TextLabel) || (tmplst->superclass==Otk_SC_Menu_Submenu)) tp1 = tmplst;	/* Base text. */
   tmplst = tmplst->nxt;
  }
 if (tp1==0) return;
 Otk_Get_Character_Size( tp1, &w, &v );
 width = (float)(maxlen + 1) * w;

 /* Now adjust their coordinates. */
 k = 0;
 lside = objpt->xleft + 0.25;
 rside = lside + width + 1.7;  // ???;
 if (rside > 100.0)
  {
   lside = lside + (100.0 - rside);
   if (lside < 1.0) lside = 1.0;
   rside = lside + width + 1.7;
  }
 tmplst = objpt->children;
 while (tmplst!=0)
  {
   tp2 = tmplst;
   if (tp2->superclass==Otk_SC_Panel)
    {
       Otk_Set_Object_Border_Thickness( tp2, 0.4 );
       first = 0;
       tp2->object_subtype = Otk_subtype_raised;
       tp2->xleft = lside;
       tp2->xright = rside;
       tp2->ytop = objpt->ybottom + 0.25;
       tp2->ybottom = objpt->ybottom + 1.5 + (float)(count) * (objpt->ybottom - objpt->ytop);
       tp2->z = 380.5 + 4.0 * (float)Otk_MenuDepth;
    }
   else
   if ((tp2->superclass==Otk_SC_Menu_DropDown_Button) || (tp2->superclass==Otk_SC_Menu_Item))
    {
     if (first)
      { /*first*/
       Otk_Set_Object_Border_Thickness( tp2, 0.4 );
       first = 0;
       tp2->object_subtype = Otk_subtype_raised;
       tp2->xleft = lside;
       tp2->xright = rside;
       tp2->ytop = objpt->ybottom + 0.25;
       tp2->ybottom = objpt->ybottom + 1.5 + (float)(count) * (objpt->ybottom - objpt->ytop);
       tp2->z = 380.5 + 4.0 * (float)Otk_MenuDepth;
      } /*first*/
     else
      { /*others*/
       tp2->object_subtype = Otk_subtype_plane;
       tp2->xleft = lside + 0.25;
       tp2->xright = rside - 0.25;
       tp2->ytop = objpt->ybottom + 0.8 + (float)k * (objpt->ybottom - objpt->ytop);
       tp2->ybottom = objpt->ybottom + 0.5 + (float)(k+1) * (objpt->ybottom - objpt->ytop);
       tp2->z = 381.0 + 4.0 * (float)Otk_MenuDepth;
      } /*others*/

     if (tp2->children != 0)
      {
       tp2 = tp2->children;	/* Assumes each list entry has first child: it's text. */
       tp2->sqrtaspect = tp1->sqrtaspect;
       tp2->scale = tp1->scale;
       tp2->xleft = lside + 0.95;
       tp2->xright = rside - 0.75;
       tp2->ytop = objpt->ybottom + 1.0 + (float)k * (objpt->ybottom - objpt->ytop);
       tp2->ybottom = objpt->ybottom + 0.7 + (float)(k+1) * (objpt->ybottom - objpt->ytop);
       tp2->z = 381.0 + Otk_DZ + 4.0 * (float)Otk_MenuDepth;
       if (tp2->children != 0)	/* Handle submenu's '>' symbol. */
	{
	 tp2 = tp2->children;
	 tp2->sqrtaspect = tp1->sqrtaspect;
	 tp2->scale = tp1->scale;
	 tp2->xleft = lside - 0.25 + 0.98 * width;
	 tp2->xright = rside - 0.75; 
	 tp2->ytop = objpt->ybottom + 1.0 + (float)k * (objpt->ybottom - objpt->ytop);
	 tp2->ybottom = objpt->ybottom + 0.7 + (float)(k+1) * (objpt->ybottom - objpt->ytop);
	 tp2->z = 381.0 + Otk_DZ + 4.0 * (float)Otk_MenuDepth;
	}
       k++;
      }
    }
   tmplst = tmplst->nxt;
  }
 Otk_MenuDepth++;
}






void otk_paste_text( char *pastebuf )
{
 int k=0, m, row, col, insertpoint, newstrnlen;
 static char *oldstr;

 Otk_Display_Changed++;

 if (pastebuf == 0) {printf("Error: Null pastebuffer.\n"); return;}
 if (Otk_verbose) printf("Pasting '%s'\n", pastebuf );
 strcpy(Otk_tmpstr,Otk_keyboard_focus->text);
 oldstr = Otk_keyboard_focus->text;
 /* Find insertion point. */
 m = 0;  row = 0;  col = 0;
 while ((Otk_tmpstr[m]!='\0') && ((row < Otk_Keyboard_state.textform_insertion_row) || (col < Otk_Keyboard_state.textform_insertion_column)))
  { 
    if (Otk_tmpstr[m]==OTK_RTN_CHR) {col = 0; row++;} else col++;
    m++; 
  }
 if (row < Otk_Keyboard_state.textform_insertion_row) { Otk_tmpstr[m++] = OTK_RTN_CHR;  Otk_tmpstr[m] = '\0'; }
 insertpoint = m;

 newstrnlen = strlen(pastebuf);
 if (newstrnlen<1) return;

 /* Shift remainder of string right by newstrnlen. */
 k = strlen(Otk_tmpstr) + newstrnlen;
 do 
  {
   k--;
   Otk_tmpstr[k+1] = Otk_tmpstr[k];
  }
 while ((insertpoint<=k) && (k>0));

 for (k=0; k<newstrnlen; k++)
  Otk_tmpstr[insertpoint+k] = pastebuf[k];  /* Insert new pastebuf chars into opened spot. */
 Otk_keyboard_focus->text = (char *)strdup(Otk_tmpstr);
 free(oldstr);
 Otk_Keyboard_state.textform_insertion_column = Otk_Keyboard_state.textform_insertion_column + newstrnlen;
}



void otk_paste_textbuf()
{
 char *string;
 int bytes;

 if (Otk_keyboard_focus==0) return;
#if (PLATFORM_KIND==Posix_Platform)
 {
  Display *display;
  display = XOpenDisplay("");
  XConvertSelection(display, XA_PRIMARY, XA_STRING, XA_CUT_BUFFER0, XDefaultRootWindow(display), CurrentTime);
  string = XFetchBytes(display, &bytes);
  otk_paste_text( string );
  XFree(string);
  XCloseDisplay(display);
 }
#else
 if ( OpenClipboard(0) )
  {
   HANDLE hData = GetClipboardData( CF_TEXT );
   char *buffer = (char*)GlobalLock( hData );
   otk_paste_text( buffer );
   GlobalUnlock( hData );
   CloseClipboard();
  }
#endif
}



void otk_store_pastebuf( char *str )
{
 if (Otk_verbose) printf("Copying '%s'\n", str );
 if (str[0] == '\0') return;
#if (PLATFORM_KIND == Posix_Platform)
 {
  Display *display;
  display = XOpenDisplay("");
  XConvertSelection(display, XA_PRIMARY, XA_STRING, XA_CUT_BUFFER0, XDefaultRootWindow(display), CurrentTime);
  XStoreBytes( display, str, strlen(str) );
  XCloseDisplay(display);
 }
#else
 {
  int k = strlen(str);
  OpenClipboard(0);
  HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (k + 1) * sizeof(TCHAR)); 
  LPTSTR lptstrCopy = GlobalLock(hglbCopy); 
  memcpy(lptstrCopy, str, k * sizeof(TCHAR)); 
  lptstrCopy[k] = (TCHAR) 0;    // null character 
  GlobalUnlock(hglbCopy); 
  SetClipboardData(CF_TEXT, hglbCopy); 
  CloseClipboard();
 }
#endif
}



#define OTK_SHIFT_KEY	    1000	/* Special key designations. */
#define OTK_CNTL_KEY	    1001
#define OTK_RIGHT_ARROW_KEY 2000
#define OTK_LEFT_ARROW_KEY  2001
#define OTK_UP_ARROW_KEY    2002
#define OTK_DOWN_ARROW_KEY  2003
#define OTK_ESCAPE_KEY	    2100
#define OTK_TAB_KEY	    2101
#define OTK_KP_NumLock	    3000	/* Keypad key designations. */
#define OTK_KP_Divide	    3001
#define OTK_KP_Multiply	    3002
#define OTK_KP_Minus	    3003
#define OTK_KP_7	    3004
#define OTK_KP_8	    3005
#define OTK_KP_9	    3006
#define OTK_KP_Plus	    3007
#define OTK_KP_4	    3008
#define OTK_KP_5	    3009
#define OTK_KP_6	    3010
#define OTK_KP_1	    3011
#define OTK_KP_2	    3012
#define OTK_KP_3	    3013
#define OTK_KP_Enter	    3014
#define OTK_KP_0	    3015
#define OTK_KP_Dot	    3015
#define OTK_F1_Key	    4000
#define OTK_F2_Key	    4001
#define OTK_F3_Key	    4002
#define OTK_F4_Key	    4003
#define OTK_F5_Key	    4004
#define OTK_F6_Key	    4005
#define OTK_F7_Key	    4006
#define OTK_F8_Key	    4007
#define OTK_F9_Key	    4008
#define OTK_F10_Key	    4009
#define OTK_F11_Key	    4010
#define OTK_F12_Key	    4011


void otk_intercept_special_keys( int *ks )
{
 switch (*ks)
  {
   #if (PLATFORM_KIND==Posix_Platform)

    case XK_Shift_L:  case XK_Shift_R:	  /* Shift-key */
	*ks = OTK_SHIFT_KEY;	    break;
    case XK_Control_L:  case XK_Control_R:  /* Control-key */
	*ks = OTK_CNTL_KEY;	    break;
    case XK_Left:	*ks = OTK_LEFT_ARROW_KEY;   break;
    case XK_Right:	*ks = OTK_RIGHT_ARROW_KEY;  break;
    case XK_Up:		*ks = OTK_UP_ARROW_KEY;	    break;
    case XK_Down:	*ks = OTK_DOWN_ARROW_KEY;   break;
    case XK_Escape:	*ks = OTK_ESCAPE_KEY;	    break;
    case XK_Tab:	*ks = OTK_TAB_KEY;	    break;
    case XK_Return:	*ks = '\n';		    break;
    case XK_Num_Lock:	*ks = OTK_KP_NumLock;	    break;
    case XK_KP_Divide:	*ks = '/';	break;
    case XK_KP_Multiply: *ks = '*';	break;
    case XK_KP_Subtract: *ks = '-';	break;
    case XK_KP_Home:	 *ks = '7';	break;
    case XK_KP_Up:	 *ks = '8';	break;
    case XK_KP_Page_Up:	 *ks = '9';	break;
    case XK_KP_Add:	 *ks = '+';	break;
    case XK_KP_Left:	 *ks = '4';	break;
    case XK_KP_Begin:	 *ks = '5';	break;
    case XK_KP_Right:	 *ks = '6';	break;
    case XK_KP_End:	 *ks = '1';	break;
    case XK_KP_Down:	 *ks = '2';	break;
    case XK_KP_Page_Down: *ks = '3';	break;
    case XK_KP_Enter:	 *ks = '\n';	break;
    case XK_KP_Insert:	 *ks = '0';	break;
    case XK_KP_Delete:	 *ks = '.';	break;

   #else	/* MS-Win */

    case VK_SHIFT:  case VK_LSHIFT:  case VK_RSHIFT:
	*ks = OTK_SHIFT_KEY;	    break;
    case VK_CONTROL:  case VK_LCONTROL:  case VK_RCONTROL:
	*ks = OTK_CNTL_KEY;	    break;
    case VK_LEFT+2000:
	*ks = OTK_LEFT_ARROW_KEY;   break;
    case VK_RIGHT+2000:
	*ks = OTK_RIGHT_ARROW_KEY;  break;
    case VK_UP+2000:
	*ks = OTK_UP_ARROW_KEY;	    break;
    case VK_DOWN+2000:
	*ks = OTK_DOWN_ARROW_KEY;   break;
    case VK_ESCAPE:
	*ks = OTK_ESCAPE_KEY;	    break;
    case VK_TAB:
	*ks = OTK_TAB_KEY;	    break;
    case  1: *ks = *ks + 96;  break;	/*A*/
    case  3: *ks = *ks + 96;  break;	/*C*/
    case 19: *ks = *ks + 96;  break;	/*S*/
    case 22: *ks = *ks + 96;  break;	/*V*/
    case 24: *ks = *ks + 96;  break;	/*X*/

   #endif
  }
}


void Otk_handle_key_release( int ks )
{
  otk_intercept_special_keys( &ks );
  if (ks == OTK_SHIFT_KEY) 
   Otk_Keyboard_state.shiftkey = 0;
  else
  if (ks == OTK_CNTL_KEY)
   Otk_Keyboard_state.ctrlkey = 0;
  if (otk_test_file)
   {
    fprintf(otk_test_file,"<t t=\"%f\"/>\n", otk_report_relative_time() );
    fprintf(otk_test_file,"<key release=\"%d\"/>\n", ks );
   }
}



void otk_delete_selected_text()
{ 
 int j, k1, k2;

 if (! otk_highlighted_text) return;
 k1 = Otk_Keyboard_state.mouse_select_start_column;
 k2 = Otk_Keyboard_state.mouse_select_end_column;
 if (k1 > k2) { j = k1;  k1 = k2;  k2 = j; }
 if (k2 > strlen( Otk_keyboard_focus->text )) k2 = strlen( Otk_keyboard_focus->text );
 Otk_Keyboard_state.textform_insertion_column = k1;
 do Otk_keyboard_focus->text[k1++] = Otk_keyboard_focus->text[k2++];
 while (Otk_keyboard_focus->text[k1 - 1] != '\0');
} 





int Otk_handle_key_input( int ks )	/* Return 1 if key intercepted by Otk, else return 0. */
{					/*  (Useful when combining or linking with other apps.) */
 int j, k=0, m, row, col, insertpoint, cntrl=0;

 Otk_Display_Changed++;
 otk_intercept_special_keys( &ks );
 if (Otk_showkey) printf("ks=%d\n", ks);
 if (otk_test_file) 
  { /*capturing*/
   if (Otk_Keyboard_state.ctrlkey && (ks == 'y'))	/* ^y - Record pause/resume (y/n) marker. */
    {
     fprintf(otk_test_file,"<t t=\"%f\"/>\n", otk_report_relative_time() );
     fprintf(otk_test_file,"<pause />\n" );
     printf("Inserting 'pause' marker into capture file.\n");
     otk_started_time = otk_report_time();
     return 1;
    }
   else
   if (Otk_Keyboard_state.ctrlkey && (ks == 'b'))	/* ^b - Split (break) the capture file. */
    { char fname[100];
     fclose(otk_test_file);
     sprintf(fname,"otk_test_capture%d.xml", otk_test_capnum++ );
     printf("Splitting capture to file:  %s\n", fname );
     otk_started_time = otk_report_time();
     otk_test_file = fopen(fname,"wb");
     return 1;
    }
   else
   if (Otk_Keyboard_state.ctrlkey && (ks == 'm'))	/* ^m - Record a marker. */
    {
     fprintf(otk_test_file,"<t t=\"%f\"/>\n", otk_report_relative_time() );
     fprintf(otk_test_file,"<mark />\n" );
     printf("Inserting marker into capture file.\n");
     return 1;
    }
   else
    {
     fprintf(otk_test_file,"<t t=\"%f\"/>\n", otk_report_relative_time() );
     fprintf(otk_test_file,"<key press=\"%d\"/>\n", ks );
    }
  } /*capturing*/

 switch (ks)
  {
   // case 101: /*e*/     if (Otk_keyboard_focus==0) if (Otk_verbose==0) Otk_verbose = 3; else Otk_verbose = 0;
   //	break;
   case 47:  case '?':  /* "?" */
	if ((Otk_keyboard_focus==0) && (Otk_Keyboard_state.shiftkey))
	 {
	  Otk_showkey = ! Otk_showkey;
	  if (Otk_showkey) printf("Now displaying keystrokes.\n"); else printf("Keystroke display OFF.\n");
	 }
	break;
   // case 113: /*q*/     if (Otk_keyboard_focus==0) exit(0);  break;
   // case 116: /*t*/     OtkDrawObjectTree();  break;
   case OTK_SHIFT_KEY:
	Otk_Keyboard_state.shiftkey = 1;	cntrl = 1;
	return 1;
	break;
   case OTK_CNTL_KEY:
	Otk_Keyboard_state.ctrlkey = 1;		cntrl = 1;
	// printf("controlkey = %d\n", Otk_Keyboard_state.ctrlkey);
	break;
   case OTK_KP_NumLock:
	Otk_Keyboard_state.keypad_numlock = ! Otk_Keyboard_state.keypad_numlock;
	break;
  }

 // printf("Key was %d, ", ks);
 // if (ks==65293) ks = '\n';
 if (ks == '\r') ks = '\n';
 // printf("now %d, (And /n=%d, /r=%d)\n", ks, '\n', '\r' );

 if ((!cntrl) && (Otk_keyboard_focus==0) && (Otk_Keyboard_Callback!=0))
  { if (Otk_Keyboard_Callback( ks )) return 0; }

 if (Otk_keyboard_focus != 0)
  { /*in_text_box*/
   static char *oldstr;

   if (Otk_Keyboard_state.ctrlkey == 1)  /* Ctrl-key */
    { /*ctrl*/
      switch (ks)
       {
	case 117:  /* Ctrl-U, Kill-line*/
		Otk_keyboard_focus->text[0] = '\0';
		Otk_Keyboard_state.textform_insertion_column = 0;
		otk_highlighted_text = 0;
		break;
       }
    } /*ctrl*/
   else
   if ((ks>31) && (ks<123))
    { /*addchar*/
     if (otk_highlighted_text) otk_delete_selected_text();
     if (Otk_Keyboard_state.shiftkey)
      { /*shiftkey*/
       if ((ks>=97) && (ks<=122)) ks = ks - 32; else
       switch (ks)
        {
	 case 44: ks = 60; break;
	 case 46: ks = 62; break;
	 case 47: ks = 63; break;
	 case 59: ks = 58; break;
	 case 39: ks = 34; break;
	 case 91: ks = 123; break;
	 case 93: ks = 125; break;
	 case 92: ks = 124; break;
	 case 96: ks = 126; break;
	 case 49: ks = 33; break;
	 case 50: ks = 64; break;
	 case 51: ks = 35; break;
	 case 52: ks = 36; break;
	 case 53: ks = 37; break;
	 case 54: ks = 94; break;
	 case 55: ks = 38; break;
	 case 56: ks = 42; break;
	 case 57: ks = 40; break;
	 case 48: ks = 41; break;
	 case 45: ks = 95; break;
	 case 61: ks = 43; break;
        }
      } /*shiftkey*/

     strcpy(Otk_tmpstr,Otk_keyboard_focus->text);
     oldstr = Otk_keyboard_focus->text;
     /* Find insertion point. */
     m = 0;  row = 0;  col = 0;
     while ((Otk_tmpstr[m]!='\0') && ((row < Otk_Keyboard_state.textform_insertion_row) || (col < Otk_Keyboard_state.textform_insertion_column)))
	{ 
	 if (Otk_tmpstr[m]==OTK_RTN_CHR) {col = 0; row++;} else col++;
	 m++; 
	}
     if (row < Otk_Keyboard_state.textform_insertion_row) { Otk_tmpstr[m++] = OTK_RTN_CHR;  Otk_tmpstr[m] = '\0'; }
     insertpoint = m;
     /* Shift remainder of string right by one. */
     k = strlen(Otk_tmpstr) + 1;
     do 
      {
	k--;
	Otk_tmpstr[k+1] = Otk_tmpstr[k];
      }
     while ((insertpoint <= k) && (k >= 0));
     Otk_tmpstr[insertpoint] = (char)ks;  /* Insert new char into opened spot. */
     Otk_keyboard_focus->text = (char *)strdup(Otk_tmpstr);
     free(oldstr);
     Otk_Keyboard_state.textform_insertion_column++;
    } /*addchar*/
   else
   if ((otk_highlighted_text) && ((ks == 65288) || (ks == 8) || (ks == 65535))) /* Delete key with highlighted text. */
    otk_delete_selected_text();
   else
   if ((ks==65288) || (ks==8))	/* Delete. (Really the back-space key). */
    {
     if ((Otk_Keyboard_state.textform_insertion_column>0) || (Otk_Keyboard_state.textform_insertion_row>0))
      {
	/* First get up to the deletion point. */
	k = 0; j = 0; 
	while ((Otk_keyboard_focus->text[j]!='\0') && (k<Otk_Keyboard_state.textform_insertion_row))
	 { if (Otk_keyboard_focus->text[j]==OTK_RTN_CHR) k++; j++; }
	/* Now j points to first character of insertion/deletion line. */
	k = 0; while ((Otk_keyboard_focus->text[j]!='\0') && (k<Otk_Keyboard_state.textform_insertion_column)) { j++; k++; }
	/* Now j points to exact deletion spot. But store it in k, temporarily to first adjust cursor. */
	k = j;
	Otk_Keyboard_state.textform_insertion_column--;
	if (Otk_Keyboard_state.textform_insertion_column<0)
	 { 
	   Otk_Keyboard_state.textform_insertion_row--;
	   m = 0; j = 0; 
	   while ((Otk_keyboard_focus->text[j]!='\0') && (m<Otk_Keyboard_state.textform_insertion_row))
	     { if (Otk_keyboard_focus->text[j]==OTK_RTN_CHR) m++; j++; }
	   /* Now j points to first character of insertion/deletion line. */
	   m = 0; while ((Otk_keyboard_focus->text[j]!='\0') && (Otk_keyboard_focus->text[j]!=OTK_RTN_CHR)) { j++; m++; }
	   Otk_Keyboard_state.textform_insertion_column = m;
	   if (Otk_keyboard_focus->text[k-1] == OTK_RTN_CHR) m = 1; else m = 0;
	 } else m = 1;
	j = k;

	/* Now actually delete the character, if valid. */
	if (m) do { Otk_keyboard_focus->text[j-1] = Otk_keyboard_focus->text[j]; j++;} while (Otk_keyboard_focus->text[j-1]!='\0');
      }
    }
   else
   if (ks==65535) 	/* Delete-Ahead. (The actual delete-space key).  Deletes character to right of insertion point. */
    {
	/* First get up to the deletion point. */
	k = 0; j = 0; 
	while ((Otk_keyboard_focus->text[j]!='\0') && (k<Otk_Keyboard_state.textform_insertion_row))
	 { if (Otk_keyboard_focus->text[j]==OTK_RTN_CHR) k++;  j++; }
	/* Now j points to first character of insertion/deletion line. */
	k = 0; while ((Otk_keyboard_focus->text[j]!='\0') && (k<Otk_Keyboard_state.textform_insertion_column)) { j++; k++; }
	/* Now j points to exact deletion spot. */
	/* Now actually delete the character, if valid. */
	while (Otk_keyboard_focus->text[j] != '\0')
	 { Otk_keyboard_focus->text[j] = Otk_keyboard_focus->text[j+1];  j++; } 
    }
   else
   if ((ks==65293) || (ks==OTK_RTN_CHR))	/* <Ret> or Return - Callback. */
    {
      if (Otk_keyboard_focus->nrows < 2) 
       {
	if (Otk_keyboard_focus->parent->functval1 != 0)
	 Otk_keyboard_focus->parent->functval1( Otk_keyboard_focus->text, Otk_keyboard_focus->parent->callback_param );
       }
      else
       { /* Multi-row text box. */

   	strcpy(Otk_tmpstr,Otk_keyboard_focus->text);
	oldstr = Otk_keyboard_focus->text;
	/* Find insertion point. */
	m = 0;  row = 0;  col = 0;
	while ((m<20000) && (Otk_tmpstr[m] != '\0') && ((row < Otk_Keyboard_state.textform_insertion_row) 
		|| (col < Otk_Keyboard_state.textform_insertion_column)))
	 { 
	  if (Otk_tmpstr[m] == OTK_RTN_CHR) {col = 0; row++;} else col++;
	  m++; 
	 }
	k = strlen(Otk_tmpstr) + 1;
	insertpoint = m;
	/* Shift remainder of string right by one. */
	do 
	 {
	  k--;
	  Otk_tmpstr[k+1] = Otk_tmpstr[k];
	 }
	while ((insertpoint <= k) && (k >= 0));
	Otk_tmpstr[insertpoint] = '\n';  /* Insert new char into opened spot. */
	Otk_keyboard_focus->text = (char *)strdup(Otk_tmpstr);
	free(oldstr);
	Otk_Keyboard_state.textform_insertion_column = 0;
        Otk_Keyboard_state.textform_insertion_row++;
	/* Handle any scrollbar issues. */

	DEBUG
	{
	 OtkWidget tmpobj;
	 printf("My %d children are:\n", Otk_keyboard_focus->parent->superclass);
	 tmpobj = Otk_keyboard_focus->parent->children;
	 while (tmpobj!=0)
	  {
	   printf("	Childsuperclass=%d\n", tmpobj->superclass);
	   tmpobj = tmpobj->nxt;
	  }
	}

	if (Otk_keyboard_focus->parent->children->superclass == Otk_SC_textscrollbar)
	 { /*shortenscrollbar*/
	   // printf("scrollbar update\n");
	   m = 0;  k = 0;	/* Count the number of rows. */
	   while (Otk_tmpstr[k]!=0) {if (Otk_tmpstr[k]==OTK_RTN_CHR) m++; k++;}
	   // printf("rows=%d\nText is: '%s'\n", m, Otk_tmpstr );
	   if (m<Otk_keyboard_focus->nrows) m=Otk_keyboard_focus->nrows;
	   OtkResizePanel( Otk_keyboard_focus->parent->children->children, 5, 5,  90, 
			   95.0 * (float)(Otk_keyboard_focus->parent->nrows) / (float)m );
	   // printf("	Resized scrollbar to 95.0 * %d / %d =  %g\n", Otk_keyboard_focus->nrows, m, 95.0 * (float)(Otk_keyboard_focus->parent->nrows) / (float)m);
	 } /*shortenscrollbar*/
	else
	 printf("no scrollbar\n");
       } /* Multi-row text box. */
    } /* <Ret> or Return - Callback. */
   else
   if (ks == OTK_LEFT_ARROW_KEY) /*leftarrow*/
    {
	Otk_Keyboard_state.textform_insertion_column = Otk_Keyboard_state.textform_insertion_column - 1;
	if (Otk_Keyboard_state.textform_insertion_column<0)
	{ /*negcol*/
	 if (Otk_keyboard_focus->nrows<2) 
	  { /*SimpleTextFormBox*/
	    Otk_Keyboard_state.textform_insertion_column = 0;
	  }
	 else
	  { /*text_edit_box*/
	    Otk_Keyboard_state.textform_insertion_row--;
	    if (Otk_Keyboard_state.textform_insertion_row<0) 
	     { Otk_Keyboard_state.textform_insertion_row = 0;   Otk_Keyboard_state.textform_insertion_column = 0; }
	    else
	    {
	     /* Determine the length of the new current line. */
	     k = 0; j = 0; 
	     while ((Otk_keyboard_focus->text[j]!='\0') && (k<Otk_Keyboard_state.textform_insertion_row))
	      { if (Otk_keyboard_focus->text[j]==OTK_RTN_CHR) k++; j++; }
	     /* Now j points to first character of insertion line. */
	     if (Otk_keyboard_focus->text[j]=='\0') { printf("Very Unusual event 2\n"); }
	     else
	      {
	       m = 0; while ((Otk_keyboard_focus->text[j]!='\0') && (Otk_keyboard_focus->text[j]!=OTK_RTN_CHR)) { j++; m++; }
	       if (Otk_Keyboard_state.textform_insertion_column < m) 
	        {
	         Otk_Keyboard_state.textform_insertion_column = m;
	        }
	      }
	    }
	  } /*text_edit_box*/
	} /*negcol*/
    }
   else
   if (ks == OTK_RIGHT_ARROW_KEY) /*rightarrow*/
    {
	Otk_Keyboard_state.textform_insertion_column = Otk_Keyboard_state.textform_insertion_column + 1;
	if (Otk_keyboard_focus->nrows<2) 
	 { /*SimpleTextFormBox*/
          if (Otk_Keyboard_state.textform_insertion_column > strlen(Otk_keyboard_focus->text))
           Otk_Keyboard_state.textform_insertion_column = strlen(Otk_keyboard_focus->text);
	 }
	else
	 { /*text_edit_box*/
	   /* Determine the length of the current line. */
	   k = 0; j = 0; 
	   while ((Otk_keyboard_focus->text[j]!='\0') && (k<Otk_Keyboard_state.textform_insertion_row))
	    { if (Otk_keyboard_focus->text[j]==OTK_RTN_CHR) k++; j++; }
	   /* Now j points to first character of insertion line, or EOL if not enough lines. */
	   if (Otk_keyboard_focus->text[j]=='\0') { Otk_Keyboard_state.textform_insertion_column = 0; printf("Unusual event 1\n"); }
	   else
	    {
	     m = 0; while ((Otk_keyboard_focus->text[j]!='\0') && (Otk_keyboard_focus->text[j]!=OTK_RTN_CHR)) { j++; m++; }
	     if (Otk_Keyboard_state.textform_insertion_column>m) 
	      {
	       Otk_Keyboard_state.textform_insertion_column = 0;
	       Otk_Keyboard_state.textform_insertion_row++;
	      }
	    }
	 }
    }
   else
   if (ks == OTK_UP_ARROW_KEY) /*uparrow*/
    {
     if (Otk_keyboard_focus->nrows < 2) 
      Otk_Keyboard_state.textform_insertion_column = 0;
     else
     { /*text_edit_box*/
      Otk_Keyboard_state.textform_insertion_row = Otk_Keyboard_state.textform_insertion_row - 1;
      if (Otk_Keyboard_state.textform_insertion_row < 0) Otk_Keyboard_state.textform_insertion_row = 0;   else
      /* Determine the length of the new current line. */
      k = 0; j = 0; 
      while ((Otk_keyboard_focus->text[j]!='\0') && (k<Otk_Keyboard_state.textform_insertion_row))
	{ if (Otk_keyboard_focus->text[j]==OTK_RTN_CHR) k++; j++; }
      /* Now j points to first character of insertion line. */
      m = 0; while ((Otk_keyboard_focus->text[j]!='\0') && (Otk_keyboard_focus->text[j]!=OTK_RTN_CHR)) { j++; m++; }
      if (Otk_Keyboard_state.textform_insertion_column > m) Otk_Keyboard_state.textform_insertion_column = m;
     }
    }
   else
   if (ks == OTK_DOWN_ARROW_KEY) /*downarrow*/
    {
     if (Otk_keyboard_focus->nrows < 2)
      Otk_Keyboard_state.textform_insertion_column = strlen(Otk_keyboard_focus->text);
     else
      { /*text_edit_box*/
       Otk_Keyboard_state.textform_insertion_row = Otk_Keyboard_state.textform_insertion_row + 1;
       /* Determine the length of the new current line. */
       k = 0; j = 0; 
       while ((Otk_keyboard_focus->text[j]!='\0') && (k<Otk_Keyboard_state.textform_insertion_row))
	{ if (Otk_keyboard_focus->text[j]==OTK_RTN_CHR) k++; j++; }
       /* Now j points to first character of insertion line, or EOL if not enough lines. */
       if (Otk_keyboard_focus->text[j]=='\0')
	{ printf("Only %d lines\n",k); Otk_Keyboard_state.textform_insertion_column = 0;  Otk_Keyboard_state.textform_insertion_row = k + 1; }
       else
	{
	 m = 0; while ((Otk_keyboard_focus->text[j]!='\0') && (Otk_keyboard_focus->text[j]!=OTK_RTN_CHR)) { j++; m++; }
	 if (Otk_Keyboard_state.textform_insertion_column > m) 
	  Otk_Keyboard_state.textform_insertion_column = m;
	}
      }
    }
   else
   if (ks == OTK_TAB_KEY) /*Tab*/
    { /*Tab*/
      if (Otk_keyboard_focus->nrows < 2)
       { /*jump2nextbox*/
	 DEBUG printf("Tabbing in simple form-box ...\n");
	 OtkWidget objpt=Otk_keyboard_focus->parent->parent->children, lastbox=0;
	 while (objpt != 0)
	  { /*while0bj*/
	    if ((objpt->mouse_sensitive == 2) && (objpt->object_class == Otk_class_panel))  /* A formbox. */
	     { /*formbox*/
	      if (objpt != Otk_keyboard_focus->parent)
	       {
	        lastbox = objpt->children;
		objpt = objpt->nxt;
	       }
	      else
	       {
	        if (lastbox != 0) objpt = 0;  else  objpt = objpt->nxt;
	       }
	     } /*formbox*/
	    else 
	     objpt = objpt->nxt;
	  } /*while0bj*/
	 if (lastbox != 0)
	  {
	    while ((lastbox != 0) && (lastbox->superclass != Otk_SC_FormBox)) lastbox = lastbox->nxt;
	    if (lastbox == 0) {printf("Otk error: no text box child.\n"); return 1;}
	    Otk_keyboard_focus = lastbox;
	    Otk_Keyboard_state.textform_insertion_column = 0;
	    Otk_Keyboard_state.textform_insertion_row = 0;
          }
       } /*jump2nextbox*/
    } /*Tab*/
  
   Otk_Keyboard_state.mouse_select_start_column = -1;
   if (otk_highlighted_text) { Otk_RemoveObject( otk_highlighted_text );  otk_highlighted_text = 0; }
   return 1;		/* Key was intercepted by Otk. */
  } /*in_text_box*/
 else return 0;		/* Key was not intercepted by Otk. */
}



int Otk_Get_Shift_Key()
 {
  return Otk_Keyboard_state.shiftkey;
 }

int Otk_Get_Control_Key()
 { return Otk_Keyboard_state.ctrlkey; }

int Otk_Get_Keypad_NumLock()
 { return Otk_Keyboard_state.keypad_numlock; }


int Otk_PreviousMouseState=1;	/* 1 = mouse button is up, 0 = down-clicked. */


void otk_pulldownmenu_highlight_check()	  /* Check for and highlight menu item, as mouse moves over it. */
{ /* Pull-Down Menu Highlighting. */	  /* Just provides visual feedback. */
 OtkWidget menu, kid, overitem=0, top_menu=0;
 int changed;

 menu = Otk_OpenMenu;
 while ((menu != 0) && (!overitem))
  { /*menu*/
    top_menu = menu;
    kid = menu->children;
    while ((kid != 0) && (! overitem))
     { /*kid*/
      if ((kid->superclass == Otk_SC_Menu_Item) || (kid->superclass == Otk_SC_Menu_DropDown_Button))
       {
        if ((Otk_MouseX > kid->xleft) && (Otk_MouseX < kid->xright) && (Otk_MouseY > kid->ytop) && (Otk_MouseY < kid->ybottom))
	 overitem = kid;
       }
      kid = kid->nxt;
     } /*kid*/
     menu = menu->parent;
     if ((!menu) || (menu->superclass != Otk_SC_Menu_DropDown_Button)) menu = 0;
   } /*menu*/
 // printf("Highlighcheck on (%x)	otk_pulldown_mousehighlight=%d,  Overitem=%d\n", Otk_OpenMenu, otk_pulldown_mousehighlight, overitem);
 if (otk_pulldown_mousehighlight == overitem) return;
 changed = otk_restore_mousemenuhighlight();
 if (overitem)
  {
   OtkCopyColor( otk_pulldown_mhlight_prev_panel_color, overitem->color );
   OtkTranslateColor( OtkSetColor( 0.3, 0.4, 0.65 ), overitem->color );
   kid = overitem->children;
   while (kid != 0)
    {
     if (kid->object_class == Otk_SC_TextLabel)
      {
       OtkCopyColor( otk_pulldown_mhlight_prev_text_color, kid->color );
       OtkTranslateColor( Otk_LightGray, kid->color );
      }
     kid = kid->nxt;
    }
   otk_pulldown_mousehighlight = overitem;
   changed = 1;
  }
 if ((changed) && (top_menu != 0))
  otk_update_draw_object( top_menu );
}




void otk_highlight_selected_text( int on_notoff )
{
 char *tstr;
 int j, k1, k2;

 k1 = Otk_Keyboard_state.mouse_select_start_column;
 k2 = Otk_Keyboard_state.mouse_select_end_column;
 if (k1 > k2) { j = k1;  k1 = k2;  k2 = j; }

 if (Otk_keyboard_focus->object_class != Otk_class_text)
  printf("Object not text as expected ??\n");
 else
  {
   if (otk_highlighted_text == 0)
    {
     otk_highlighted_text = OtkMakeTextLabel( Otk_keyboard_focus->parent, "", OtkSetColor(0.0, 0.0, 0.8), 1.0, 2.0, 3.0, 22.0 );
     if (!Otk_Vect_Font) Otk_Vect_Font = Otk_Build_Internal_Font(Otk_Font_Vect, Otk_FontSpacing_Mono);
     otk_highlighted_text->font = Otk_Vect_Font;

     otk_highlighted_text->superclass = Otk_SC_FormBox;
     otk_highlighted_text->scissor_to_parent = 1;
     otk_highlighted_text->sqrtaspect = Otk_keyboard_focus->sqrtaspect;
     otk_highlighted_text->scale = Otk_keyboard_focus->scale;
     otk_highlighted_text->nrows = 1;
     otk_highlighted_text->ncols = Otk_keyboard_focus->ncols;
     otk_highlighted_text->horiztextscroll = Otk_keyboard_focus->horiztextscroll;
     otk_highlighted_text->verttextscroll = 0;
     otk_highlighted_text->z = otk_highlighted_text->z + 0.5;
    }
   if (k2 >= strlen(Otk_keyboard_focus->text)) {k2 = strlen(Otk_keyboard_focus->text); }
   tstr = strdup( Otk_keyboard_focus->text );
   j = 0;
   while (tstr[j] != '\0')
    {
     if (j < k1) tstr[j] = ' '; else
     if (j >= k2) tstr[j] = ' ';
     j++;
    }
   free( otk_highlighted_text->text );
   otk_highlighted_text->text = tstr;
  }
}



int Otk_handle_mouse_move( int MouseDx, int MouseDy )	/* Handle any widget control functions of mouse-move. */
{							/*  (ex. scroll, slider-move, window-drag, etc..) */
 OtkWidget objpt=OtkRootObject, topobj=0, tmpobj;	/* Return 1 if move affected a widget, else return 0. */
 float dx, dy, parentinvsz;

 Otk_MouseX = 100.0 * (float)Otk_MousePixX / (float)OtkWindowSizeX;
 Otk_MouseY = 100.0 * (float)Otk_MousePixY / (float)OtkWindowSizeY;
 dx = 100.0 * (float)MouseDx / (float)OtkWindowSizeX;
 dy = 100.0 * (float)MouseDy / (float)OtkWindowSizeY;
 if (Otk_PreviousMouseState==0) Otk_Display_Changed++;
 if ((Otk_showkey) && (Otk_verbose > 9)) printf("	MousePos (%g, %g)\n", Otk_MouseX, Otk_MouseY );
 if (otk_test_file)
  {
   fprintf(otk_test_file,"<t t=\"%f\"/>\n", otk_report_relative_time() );
   fprintf(otk_test_file,"<mouse_move x=\"%g\" y=\"%g\" dx=\"%g\" dy=\"%g\"/>\n", Otk_MouseX, Otk_MouseY, dx, dy );
  }
 topobj = Otk_ClickedObject;
 if (topobj == 0) 
  {
   // printf("Mouse move: %8g %8g	ButtonState = %d\n", Otk_MouseX, Otk_MouseY, Otk_PreviousMouseState );
   otk_hover_box_check();
   if (Otk_OpenMenu) otk_pulldownmenu_highlight_check();
   if (Otk_MouseMove_Callback != 0) Otk_MouseMove_Callback();
   return 0;
  }
 else
  {
    DEBUG printf("\nDrag superclass=%d objectclass=%d parent_superclass=%d	Sensitive=%d\n", topobj->superclass, topobj->object_class, topobj->parent->superclass, topobj->mouse_sensitive);
    if ((topobj->mouse_sensitive == 1) && (topobj->object_class == Otk_class_panel) && (topobj->superclass == Otk_SC_Window))
    {
	objpt = topobj;
	objpt->xleft = objpt->xleft + dx;
	objpt->xright = objpt->xright + dx;
	objpt->ytop = objpt->ytop + dy;
	objpt->ybottom = objpt->ybottom + dy;

	tmpobj = objpt->children;
	while (tmpobj!=objpt)
	 {
	  tmpobj->xleft = tmpobj->xleft + dx;
	  tmpobj->xright = tmpobj->xright + dx;
	  tmpobj->ytop = tmpobj->ytop + dy;
	  tmpobj->ybottom = tmpobj->ybottom + dy;
	  if (tmpobj->children != 0)
	    tmpobj = tmpobj->children;				/* Descend. */
	  else
	   {
	    while( tmpobj && !tmpobj->nxt && tmpobj != objpt )	/* Ascend, as needed. */
	       tmpobj = tmpobj->parent;
	    if( tmpobj && tmpobj != objpt )			/* Advance. */
	       tmpobj = tmpobj->nxt;
	   }
	 }
     }
    else
    if ((topobj->mouse_sensitive == 3) && (topobj->object_class == Otk_class_panel))
     { /*SliderH*/
	objpt = topobj;
	objpt->xleft = objpt->xleft + dx;
	objpt->xright = objpt->xright + dx;
	dx = objpt->xright - objpt->xleft;
	if (objpt->xleft < objpt->parent->xleft) {objpt->xleft = objpt->parent->xleft; objpt->xright = objpt->parent->xleft + dx;}
	if (objpt->xright > objpt->parent->xright) {objpt->xright = objpt->parent->xright;  objpt->xleft = objpt->parent->xright - dx;}
	parentinvsz = 100.0 / (objpt->parent->xright - objpt->parent->xleft);
	objpt->x1 = (objpt->xleft - objpt->parent->xleft) * parentinvsz;
	objpt->x2 = (objpt->xright - objpt->parent->xleft) * parentinvsz;
	if (objpt->parent->functval2!=0) 
	 objpt->parent->functval2( 
		(objpt->xleft - objpt->parent->xleft) / (objpt->parent->xright - objpt->parent->xleft - (objpt->xright - objpt->xleft)), 
		objpt->parent->callback_param );
     }
    else
    if ((topobj->mouse_sensitive == 4) && (topobj->object_class == Otk_class_panel))
     { /*SliderV*/
	DEBUG printf("Vertical slider move\n");
	objpt = topobj;
	objpt->ytop = objpt->ytop + dy;
	objpt->ybottom = objpt->ybottom + dy;
	dy = objpt->ybottom - objpt->ytop;
	if (objpt->ytop < objpt->parent->ytop) {objpt->ytop = objpt->parent->ytop; objpt->ybottom = objpt->parent->ytop + dy;}
	if (objpt->ybottom > objpt->parent->ybottom) {objpt->ybottom = objpt->parent->ybottom;  objpt->ytop = objpt->parent->ybottom - dy;}
	parentinvsz = 100.0 / (objpt->parent->ybottom - objpt->parent->ytop);
	objpt->y1 = (objpt->ytop - objpt->parent->ytop) * parentinvsz;
	objpt->y2 = (objpt->ybottom - objpt->parent->ytop) * parentinvsz;
	if (objpt->parent->functval2!=0) 
	 objpt->parent->functval2( 
		(objpt->ytop - objpt->parent->ytop) / (objpt->parent->ybottom - objpt->parent->ytop - (objpt->ybottom - objpt->ytop)), 
		objpt->parent->callback_param );
     }
    else
    if ((topobj->mouse_sensitive == 5) && (topobj->object_class == Otk_class_panel))
     { /*TextScrollbar*/
	objpt = topobj;
	objpt->ytop = objpt->ytop + dy;
	objpt->ybottom = objpt->ybottom + dy;
	dy = objpt->ybottom - objpt->ytop;
	if (objpt->ytop < objpt->parent->ytop) {objpt->ytop = objpt->parent->ytop; objpt->ybottom = objpt->parent->ytop + dy;}
	if (objpt->ybottom > objpt->parent->ybottom) {objpt->ybottom = objpt->parent->ybottom;  objpt->ytop = objpt->parent->ybottom - dy;}
	if (objpt->parent->functval2!=0) 
	 objpt->parent->functval2( 
		(objpt->ytop - objpt->parent->ytop) / (objpt->parent->ybottom - objpt->parent->ytop - (objpt->ybottom - objpt->ytop)), 
		objpt->parent->callback_param );
     }
    else
    if ((topobj->mouse_sensitive == 2) && (topobj->object_class == Otk_class_panel))
     { /*TextBox*/
	float w, h;
	int end;

	Otk_Get_Character_Size( Otk_keyboard_focus, &w, &h );
	end = ((Otk_MouseX - Otk_keyboard_focus->parent->xleft) / w) + Otk_keyboard_focus->horiztextscroll;
	if (end < 0) end = 0; else
	if (end > strlen(Otk_keyboard_focus->text))
	 end = strlen(Otk_keyboard_focus->text);

	if (Otk_keyboard_focus->nrows < 2)
	 { /*formbox*/
	   Otk_Keyboard_state.mouse_select_end_column = end;
	   if (end != Otk_Keyboard_state.mouse_select_old_column)
	    {
	     otk_highlight_selected_text(1);
	     Otk_Keyboard_state.mouse_select_old_column = end;
	    }
	   // printf(" Hilite text select = %d to %d\n", Otk_Keyboard_state.mouse_select_start_column, Otk_Keyboard_state.mouse_select_end_column );
	 } /*formbox*/
	else
	 { /*editbox*/
	   // printf(" Hilite text select = %d to %d\n", Otk_Keyboard_state.mouse_select_start_column, Otk_Keyboard_state.mouse_select_end_column );
	 } /*editbox*/
     }
    else
     if (Otk_MouseMove_Callback != 0) Otk_MouseMove_Callback();
  }

 return 1;
}



void Otk_close_pulldown()	/* Close, deflate, or collapse, a pulled-down menu. */
{
 OtkWidget tpt, tpt2;

 otk_restore_mousemenuhighlight();
 Otk_MenuDepth = 0;
 Otk_Set_Button_State( Otk_OpenMenu, 0 );  
 tpt = Otk_OpenMenu;
 while ((tpt != 0) && (tpt->superclass == Otk_SC_Menu_DropDown_Button))
  { /*while*/
   Otk_Set_Button_State( tpt, 0 );  

   // Move the children into hiding
   if ( tpt->hidden_children )
    {
     tpt2 = tpt->hidden_children;
     while ( tpt2->nxt ) tpt2 = tpt2->nxt;
     tpt2->nxt = tpt->children->nxt;
    }
   else
    tpt->hidden_children = tpt->children->nxt;	/* Assumes first child (only) is top-button text that must stay. */
   tpt->children->nxt = NULL;

   tpt = tpt->parent; 
  } /*while*/

 Otk_OpenMenu = 0;
 Otk_check_window_level();
}




int otk_textbox_strlen( char *str )	/* Determines length of given line (by entry-point) within a mult-lie text-edit buffer. */
{
 int j=0;
 while ((str[j] != '\0') && (str[j] != '\n')) j++;
 return j;
}



void set_parallel_selection_list_items( OtkWidget topobj, int raise )
{ OtkWidget masterlist, tcpt1, tcpt2;
  int j, k=0, m=0;

 if (abs(topobj->parent->verttextscroll) == 2)	/* If master scroll-bar, */
  masterlist = topobj->parent;
 else
 if (topobj->parent->verttextscroll == 3)	/* If subrdinate list. */
  {
   tcpt1 = topobj->parent->children;
   while ((tcpt1 != 0) && (tcpt1->superclass != Otk_SC_Select_List_SubList))
    tcpt1 = tcpt1->nxt;
   if ((tcpt1 != 0) && (tcpt1->object_class == Otk_class_other)) masterlist = tcpt1->scissor_parent;
   else return;
   // notmaster = 1;
  }
 else return;; 

 /* Then select the subordinate lists too. */
 /* First determine which displayed item was selected. */
 tcpt1 = topobj->parent->children;
 while ((tcpt1 != 0) && (tcpt1 != topobj))
  {
   if (tcpt1->superclass==Otk_SC_Select_List_Item) k++;
   tcpt1 = tcpt1->nxt;
  }
 /* Now set that item in the other lists. */
 tcpt1 = masterlist->children;
 while (tcpt1 != 0)
  {
   if ((tcpt1->superclass==Otk_SC_Select_List_SubList) && (tcpt1->object_class == Otk_class_other))
    {
     tcpt2 = tcpt1->scissor_parent->children;  j = 0;
     while ((tcpt2 != 0) && (j < k))
      {
	if (tcpt2->superclass==Otk_SC_Select_List_Item) j++;
	tcpt2 = tcpt2->nxt;
      }
     if (tcpt2 != 0)
      {
       if (raise)
	{
	 Otk_Set_Button_State( tcpt2, 2 );
	 tcpt2->object_subtype = Otk_subtype_raised;
	}
       else
	{
	 tcpt2->object_subtype = Otk_subtype_plane;
	}
      }
    }
   else
   if (tcpt1->superclass==Otk_SC_Select_List_Item)
    {
     if (m == k)
      { if (raise) tcpt1->object_subtype = Otk_subtype_raised; else tcpt1->object_subtype = Otk_subtype_plane; }
     m++;
    }
   tcpt1 = tcpt1->nxt;
  }	    
}



int Otk_Get_MouseButton_Clicked()	/* Returns the number of last mouse button clicked, 1-3. */
{ return Otk_Clicked_MouseButton; }




int Otk_handle_mouse_click( int state, int button_number )   /* State:  0=mouse-button-down-press, 1=mouse-button-release. */
{					  /* Return 0 if unchanged state or not down-click on a widget, else 1. */
 OtkWidget objpt, topobj=0;
 float minz=-9e9, eleft, eright, etop, ebottom;
 int undermouse, j;
 OtkClipList *cliplist = NULL, *clipto;

 Otk_MouseX = 100.0 * (float)Otk_MousePixX / (float)OtkWindowSizeX;
 Otk_MouseY = 100.0 * (float)Otk_MousePixY / (float)OtkWindowSizeY;
 Otk_ClickedX = Otk_MouseX;
 Otk_ClickedY = Otk_MouseY;
 Otk_Clicked_MouseButton = button_number;
 if (otk_test_file)
  {
   fprintf(otk_test_file,"<t t=\"%f\"/>\n", otk_report_relative_time() );
   fprintf(otk_test_file,"<mouse_click x=\"%g\" y=\"%g\" state=\"%d\" button=\"%d\"/>\n", 
			    Otk_MouseX, Otk_MouseY, state, button_number );
  }
 if (Otk_verbose) printf("MouseClick: State=%d, PreviousMouseState=%d, Button=%d\n", state, Otk_PreviousMouseState, button_number );
 if (state==Otk_PreviousMouseState) return 0;
 else Otk_PreviousMouseState = state;

 Otk_Display_Changed++;
 kill_hoverbox();

 if (button_number > 3)  /* Handle scroll-wheel. */
  {
   if (state == 0) { if (otk_last_scrollable) return 1;  else  return 0; }
   Otk_keyboard_focus = 0;
   if (otk_highlighted_text) { Otk_RemoveObject( otk_highlighted_text );  otk_highlighted_text = 0; }
   if (otk_last_scrollable)
    {
     if (button_number == 4) Otk_scrolllist_up( otk_last_scrollable ); else
     if (button_number == 5) Otk_scrolllist_down( otk_last_scrollable );
     return 1; 
    }
   return 0;
  }

 if (state==1) 
  { /*mouse-up*/

    if ((Otk_keyboard_focus) && (Otk_Keyboard_state.mouse_select_start_column >= 0)
	&& (Otk_Keyboard_state.mouse_select_start_column != Otk_Keyboard_state.mouse_select_end_column))
     { char *tstr;
      if (Otk_Keyboard_state.mouse_select_start_column > Otk_Keyboard_state.mouse_select_end_column)
       {
	j = Otk_Keyboard_state.mouse_select_start_column;
	Otk_Keyboard_state.mouse_select_start_column = Otk_Keyboard_state.mouse_select_end_column;
	Otk_Keyboard_state.mouse_select_end_column = j;
       }
      tstr = strdup( &(Otk_keyboard_focus->text[ Otk_Keyboard_state.mouse_select_start_column ]) );
      tstr[ Otk_Keyboard_state.mouse_select_end_column - Otk_Keyboard_state.mouse_select_start_column ] = '\0';
      otk_store_pastebuf( tstr );
      free(tstr);
     }

   if (Otk_MouseClick_Callback!=0) Otk_MouseClick_Callback( state );
   if ((Otk_ClickedObject != 0) && (Otk_ClickedObject->superclass == Otk_SC_Button))
    {
     Otk_Set_Button_State( Otk_ClickedObject, 0 );
     if ((otk_test_file) && (Otk_ClickedObject->children) && (Otk_ClickedObject->children->object_class == Otk_class_text))
      fprintf(otk_test_file,"\n<!-- Button Clicked: %s -->\n\n", Otk_ClickedObject->children->text );
    }
   Otk_ClickedObject = 0;

   return 1;	/* Be sensitive to button-press only. */
  } /*mouse-up*/
 else
  { /*mouse-down*/
    if (otk_highlighted_text) { Otk_RemoveObject( otk_highlighted_text );  otk_highlighted_text = 0; }
  } /*mouse-down*/

 Otk_keyboard_focus = 0;
 if ((otk_last_scrollable) && (otk_last_scrollable->mouse_sensitive != 0)) otk_last_scrollable = 0;

 /* Determine which object, if any, were clicked on. */
 objpt = OtkRootObject;
 while (objpt!=0)
  { /*while0bj*/
   if ((objpt->object_class==Otk_class_panel && !objpt->invisible) || (objpt->object_class==Otk_class_button) || 
	 (objpt->superclass==Otk_SC_RadioButton) || (objpt->object_class==Otk_class_togglebutton) ||
	 (objpt->superclass==Otk_SC_Menu_DropDown_Button))
    {
     if ( objpt->scissor_to_parent && objpt->parent )
      {
       clipto = (OtkClipList *)malloc( sizeof(OtkClipList) );
       clipto->nxt = cliplist;
       if ( cliplist )
	{
	 clipto->xleft = MAX( cliplist->xleft, objpt->parent->xleft );
	 clipto->xright = MIN( cliplist->xright, objpt->parent->xright );
	 clipto->ytop = MAX( cliplist->ytop, objpt->parent->ytop );
	 clipto->ybottom = MIN( cliplist->ybottom, objpt->parent->ybottom );
	}
       else
	{
	 clipto->xleft = objpt->parent->xleft;
	 clipto->xright = objpt->parent->xright;
	 clipto->ytop = objpt->parent->ytop;
	 clipto->ybottom = objpt->parent->ybottom;
	}
       cliplist = clipto;
      }
     if ( cliplist )
      {
       eleft = MAX( objpt->xleft, cliplist->xleft );
       eright = MIN( objpt->xright, cliplist->xright );
       etop = MAX( objpt->ytop, cliplist->ytop );
       ebottom = MIN(objpt->ybottom, cliplist->ybottom );
       undermouse = ((Otk_MouseX >= eleft) && (Otk_MouseX <= eright) &&
		     (Otk_MouseY >= etop) && (Otk_MouseY <= ebottom));
      }
     else
      {
       undermouse = ((Otk_MouseX >= objpt->xleft) && (Otk_MouseX <= objpt->xright) &&
		     (Otk_MouseY >= objpt->ytop) && (Otk_MouseY <= objpt->ybottom));
      }
    }
   else
    {
     undermouse = 0;
    }

   // printf("Sens=%d supertyp=%d subtype=%d (%g < %g <%g) (%g < %g <%g)\n", objpt->mouse_sensitive, objpt->superclass, objpt->object_subtype, objpt->xleft, Otk_MouseX, objpt->xright, objpt->ytop, Otk_MouseY, objpt->ybottom);
   if (undermouse && objpt->z > minz) { minz = objpt->z;  topobj = objpt; }
   if( objpt->children )
    {
     objpt = objpt->children;		/* Descend. */
    }
   else
    {
     while( objpt && !objpt->nxt ) 	/* Ascend, as needed. */
      {
       if( objpt && objpt->scissor_to_parent && objpt->parent && cliplist )
	{
	 clipto = cliplist;
	 cliplist = cliplist->nxt;
	 free( clipto );
	}
       objpt = objpt->parent;
      }
     if( objpt && objpt->scissor_to_parent && objpt->parent && cliplist )
      {
       clipto = cliplist;
       cliplist = cliplist->nxt;
       free( clipto );
      }
     if( objpt ) objpt = objpt->nxt;	/* Advance. */
    }
  } /*while0bj*/

 /* Now topobj is the clicked object, if any. */
 if (topobj == OtkOuterWindow) 
  {
   if (Otk_OpenMenu) Otk_close_pulldown();	 /* Click-away. */
   return 0;
  }
 else if (topobj == 0) {printf("Null clicked obj. UNEXPECTED SERIOUS ERROR!!!  Aborting...\n"); exit(0);}
 Otk_ClickedObject = topobj;

 DEBUG printf("\nClick superclass=%d objectclass=%d parent_superclass=%d	Sensitive=%d\n", topobj->superclass, topobj->object_class, topobj->parent->superclass, topobj->mouse_sensitive);

if ((topobj->superclass==Otk_SC_Menu_DropDown_Button) && (topobj->object_class==Otk_class_text)) 
 {
  topobj = topobj->parent;
 }

 /* Clear any opened menu, on click-away. */
 if ((Otk_OpenMenu) /* && (Otk_OpenMenu != topobj->parent) */ )	/* Clear any opened menu. */
  if ((topobj->superclass!=Otk_SC_Menu_DropDown_Button) || (topobj->parent->superclass!=Otk_SC_Menu_DropDown_Button)
	|| (Otk_OpenMenu != topobj->parent) )
   Otk_close_pulldown();	 /* Click-away. */

 if (Otk_Selected_Item)
  {
   Otk_Selected_Item->object_subtype = Otk_subtype_plane;
   if ((Otk_Selected_Item->superclass==Otk_SC_Select_List_Item) && (Otk_Selected_Item->parent != 0) && (abs(Otk_Selected_Item->parent->verttextscroll) > 1))
    set_parallel_selection_list_items( Otk_Selected_Item, 0 );
   Otk_Selected_Item = 0;
  }

 if (topobj!=0) 
  {
    DEBUG printf("Clicked class=%d supersclass=%d mouse_sensitive=%d callbac=%lx\n", topobj->object_class, topobj->superclass, topobj->mouse_sensitive, (unsigned long)(topobj->callback));
    if ((topobj->mouse_sensitive==1) && (topobj->object_class==Otk_class_button))	/* Button click. */
     {
	objpt = topobj;
	DEBUG printf("Button press.\n");
	Otk_Set_Button_State( objpt, 1 );
	if (objpt->callback!=0)
	 objpt->callback( objpt->callback_param );
	else
	if (objpt->superclass==Otk_SC_Window)
	 { /*killwindow*/
	   if (objpt->parent->callback != 0) objpt->parent->callback( objpt->parent->callback_param );
	   Otk_RemoveObject( objpt->parent );
	 } /*killwindow*/
     } else
    if ((topobj->mouse_sensitive==1) && (topobj->superclass==Otk_SC_Menu_DropDown_Button))	/* Menu click. */
     {
	objpt = topobj;
	DEBUG printf("Menu press.\n");
	Otk_Set_Button_State( objpt, 2 );
	Otk_OpenMenu = objpt;
        // printf("INFLATING objectclass=%d\n", objpt->object_class);
	Otk_InflateMenu( objpt );
     } else
    if ((topobj->mouse_sensitive==1) && (topobj->superclass==Otk_SC_Menu_Item))	/* Menu click. */
     {
	objpt = topobj;
	DEBUG printf("Menu press.\n");
	Otk_Set_Button_State( objpt, 2 );
	if (objpt->callback!=0)
	   objpt->callback( objpt->callback_param );
     } else
    if ((topobj->mouse_sensitive==1) && (topobj->superclass==Otk_SC_RadioButton))	/* Radio Button click. */
     {
	if (topobj->state==1) return 1;	/* No change. */

	DEBUG printf("Radio Button press.\n");
	Otk_SetRadioButton( topobj );

	if (topobj->callback!=0)
	 topobj->callback( topobj->callback_param );
     } else
    if ((topobj->mouse_sensitive==1) && (topobj->superclass==Otk_SC_ToggleButton))	/* Toggle Button click. */
     {
	objpt = topobj;
	Otk_Set_Button_State( objpt, ! objpt->state );
	DEBUG printf("Toggle press. State = %d\n", objpt->state );
	if (objpt->functval3!=0)
	 objpt->functval3( objpt->state, objpt->callback_param );
     } else
    if ((topobj->mouse_sensitive==1) && (topobj->superclass==Otk_SC_Select_List_Item))	/* Selection List click. */
     {
	objpt = topobj;
	DEBUG printf("Selection List click.\n");
	Otk_Set_Button_State( objpt, 2 );
	objpt->object_subtype = Otk_subtype_raised;
	if (abs(topobj->parent->verttextscroll) > 1) set_parallel_selection_list_items( topobj, 1 );
	Otk_Selected_Item = topobj;
	if (topobj->callback!=0)
	 topobj->callback( topobj->callback_param );
     } else
    if ((topobj->mouse_sensitive==3) || (topobj->mouse_sensitive==4))	/* Slider Knob. */
     {
	if ((otk_last_scrollable == 0) || (otk_last_scrollable->mouse_sensitive == 0)) otk_last_scrollable = topobj;
     } else
    if ((topobj->mouse_sensitive==2) && (topobj->object_class==Otk_class_panel))	/* Click in Text Form Box. */
     {
	float w, ht, x;
        int j, k, row, curline_indx, strt;
	OtkWidget tmpchild;

	DEBUG printf("Form Box press.\n");
	tmpchild = topobj->children;
	while ((tmpchild!=0) && (tmpchild->superclass!=Otk_SC_FormBox)) tmpchild = tmpchild->nxt;
	if (tmpchild == 0) {printf("Otk error: no text box child.\n"); return 1;}
	Otk_keyboard_focus = tmpchild;
	Otk_Get_Character_Size( Otk_keyboard_focus, &w, &ht );
	x = ((Otk_MouseX - topobj->xleft) / w) - 0.5;
	Otk_Keyboard_state.textform_insertion_column = x + Otk_keyboard_focus->horiztextscroll;
        if (Otk_Keyboard_state.textform_insertion_column < 0) Otk_Keyboard_state.textform_insertion_column = 0;

	/* Now determine the row clicked. */
	if (Otk_keyboard_focus->nrows <= 1)
	 { /*formbox*/
	   strt = Otk_Keyboard_state.textform_insertion_column;
	   if (strt > strlen(Otk_keyboard_focus->text)) strt = strlen( Otk_keyboard_focus->text);
	   Otk_Keyboard_state.mouse_select_start_column = strt;
	   Otk_Keyboard_state.mouse_select_end_column = strt;
	   Otk_Keyboard_state.mouse_select_old_column = strt;

	   Otk_Keyboard_state.textform_insertion_row = 0;
	   if (Otk_Keyboard_state.textform_insertion_column > strlen(Otk_keyboard_focus->text))
	    Otk_Keyboard_state.textform_insertion_column = strlen(Otk_keyboard_focus->text);
	 } /*formbox*/
	else
	 { /*texteditbox*/
	   row = (float)Otk_keyboard_focus->nrows * (Otk_MouseY - Otk_keyboard_focus->parent->ytop) / (Otk_keyboard_focus->parent->ybottom - Otk_keyboard_focus->parent->ytop);
	   row = row + Otk_keyboard_focus->verttextscroll;
	   if (row<0) row = 0;
	   Otk_Keyboard_state.textform_insertion_row = row;
	   /* Determine if there is a text row there yet. */
	   j = 0;  k = 0;  curline_indx = 0;
	   while ((k < Otk_Keyboard_state.textform_insertion_row) && (Otk_keyboard_focus->text[j]!='\0'))
	    { 
	     if (Otk_keyboard_focus->text[j]=='\n') { curline_indx = j + 1;  k++; }
	     j++;
	    }

	   if (k < Otk_Keyboard_state.textform_insertion_row) 
	    {
	    	Otk_Keyboard_state.textform_insertion_column = otk_textbox_strlen( &(Otk_keyboard_focus->text[curline_indx]) );
	  	Otk_Keyboard_state.textform_insertion_row = k;
	    }
	   else
	    if (Otk_Keyboard_state.textform_insertion_column > otk_textbox_strlen( &(Otk_keyboard_focus->text[curline_indx]) ))
	   	Otk_Keyboard_state.textform_insertion_column = otk_textbox_strlen( &(Otk_keyboard_focus->text[curline_indx]) );
	 } /*texteditbox*/

	Otk_Keyboard_state.cursor_blink_count = 0;
     }
    else
    if ((topobj->mouse_sensitive==mouse_groove_click) && ((topobj->superclass==Otk_SC_vSlider) || (topobj->superclass==Otk_SC_hSlider)))	/* Sroll-bar groove-click. */
     { float vpos;
       OtkWidget knob;

       vpos = Otk_GetSlider( topobj );
       knob = topobj->children;
       while ((knob != 0) && (knob->mouse_sensitive != 3) && (knob->mouse_sensitive != 4)) knob = knob->nxt;
       if (knob == 0) {printf("Otk Error 305: Missing slider knob.\n");  return 1;}
       otk_last_scrollable = knob;
       if (knob->mouse_sensitive == 3)
	{ /*Horizontal_slider*/
	  if (knob->xleft - Otk_MouseX > 0.0)
	   {
	    vpos = vpos - 0.125;
	    if (vpos < 0.0) vpos = 0.0;
	   }
	  else
	   {
	    vpos = vpos + 0.125;
	    if (vpos > 1.0) vpos = 1.0;
	   }
	 Otk_SetSlider( topobj, vpos, 1.0 );
	 if (topobj->functval2 != 0)   topobj->functval2( vpos, topobj->callback_param );
	} /*Horizontal_slider*/
       else
       if (topobj->parent->superclass != Otk_SC_Select_List)
	{ /*Vertical_slider*/	
	  if (knob->ytop - Otk_MouseY > 0.0)
	   {
	    vpos = vpos - 0.125;
	    if (vpos < 0.0) vpos = 0.0;
	   }
	  else
	   {
	    vpos = vpos + 0.125;
	    if (vpos > 1.0) vpos = 1.0;
	   }
	 Otk_SetSlider( topobj, vpos, 1.0 );
	 if (topobj->functval2 != 0)  topobj->functval2( vpos, topobj->callback_param );
	} /*Vertical_slider*/
       else
	{ /*Scroll-List_Vertical_slider*/
	  // printf("Slider is at %g, Mouse clicked at %g.  Number of list entries = %d\n", vpos, knob->ytop - Otk_MouseY, topobj->parent->nentries );
	  if (knob->ytop - Otk_MouseY > 0.0)
	   {
	    vpos = vpos - topobj->parent->nrows / (float)(topobj->parent->nentries);
	    if (vpos < 0.0) vpos = 0.0;
	   }
	  else
	   {
	    vpos = vpos + topobj->parent->nrows / (float)(topobj->parent->nentries);
	    if (vpos > 1.0) vpos = 1.0;
	   }
	 Otk_SetSlider( topobj, vpos, 1.0 );
         Otk_scrolllist( vpos, topobj->parent );
	} /*Scroll-List_Vertical_slider*/
     }
    else
     if (Otk_MouseClick_Callback!=0)
      {
	DEBUG printf("Other press.\n");
	Otk_MouseClick_Callback( state );
      }
  }
else printf("Unnexpected A\n");
return 1;
}









/*---------------------*/
/* OtkDisplayFunct -   */
/*---------------------*/
void OtkDisplayFunct()
{ /*displayfunct*/
  DEBUG printf("	Otk_Display_Changed =%d\n", Otk_Display_Changed);
  if ((Otk_Display_Changed) || (Otk_nondraws > 500) /* || (Otk_Always_Update) */ )
   {
    OtkDrawAll(Otk_refresh);
    if ((Otk_verbose>1) && (Otk_snifferrors(3))) printf("OGL Error: Drawing objects.\n");
    Otk_Display_Changed = 0;
    Otk_nondraws = 0;
   } else Otk_nondraws++;
} /*displayfunct*/








#if (GlutEnabled==1)

void Otk_keyeventfunct( unsigned char key, int x, int y )
{
 int ks;
 // printf("key %c %d %d\n", key, x, y);
 if (Otk_showkey) printf("Key=%d\n", key);
 ks = key;
 if (ks==8) ks = 65288;
 Otk_handle_key_input(ks);
}
 
void Otk_speckeyeventfunct( int key, int x, int y )
{
 // printf("key %c %d %d\n", key, x, y);
 if (Otk_showkey) printf("Key=%d\n", key);

 switch( key)
  {
   case GLUT_KEY_LEFT:  /*leftarrow*/ 	key = 65361;  break;
   case GLUT_KEY_RIGHT: /*rightarrow*/	key = 65363;  break;
   case GLUT_KEY_UP:    /*uparrow*/ 	key = 65362;  break;
   case GLUT_KEY_DOWN:  /*downarrow*/	key = 65364;  break;
  }
 Otk_handle_key_input(key);
}
 
void Otk_mousemotion( int x, int y )
{
 int MouseDx, MouseDy;
 
 // printf("mouse move (%d, %d)\n", x, y);
 MouseDx = -(Otk_MousePixX - x);
 MouseDy = -(Otk_MousePixY - y);
 Otk_handle_mouse_move( MouseDx, MouseDy );
 Otk_MousePixX = x;
 Otk_MousePixY = y; 
}

void Otk_mousefunct( int button, int state, int x, int y )
{
 // printf("mouse button %d  state=%d  (%d, %d)\n", button, state, x, y);
 Otk_MousePixX = x;
 Otk_MousePixY = y;
 Otk_handle_mouse_click(state,1);
}


void Otk_reshapefunct( int w, int h )
{
 printf("resized window %d %d\n", w, h );
 OtkWindowSizeX = w;
 OtkWindowSizeY = h;
 glViewport( 0, 0, w, h );
 Otk_Display_Changed = 1;
}
#endif





double otk_report_time()	/* Reports time in seconds, accurate to millisecs, for checking time differences. */
{
 #ifdef __MINGW32__
  // SYSTEMTIME st;
  // GetSystemTime( &st );
  // return (((double)(st.wDay * 24.0 + (double)(st.wHour)) * 60.0 + (double)(st.wMinute)) * 60.0 + (double)(st.wSecond) + 1e-3 * (double)(st.wMilliseconds);
  return (double)GetTickCount() * 1.0e-3;
 #else
   struct timeval tp;
   gettimeofday(&tp,0);
   return (double)(tp.tv_sec -  1168625572) + 1e-6 * (double)(tp.tv_usec);
 #endif
}



double otk_report_relative_time()	/* Reports time since start of application, in seconds, accurate to milliseconds. */
{
 return otk_report_time() - otk_started_time;
}



double Otk_Report_Frame_Period()	/* Returns recent frame redraw-time in Seconds. */
{
 return OTK_FRAME_PERIOD;
}





int Otk_windowmapped_state=0;


void Otk_Force_Redraw()
{
 otk_last_redraw_time = -1.0;
}


void OtkUpdateCheck()
{

 #if (GlutEnabled==0)
 #if (WinGLEnabled==0)

 if (!Otk_windowmapped_state)
  {
   XMapWindow( Otkdpy, Otkwin );	/* Map the Window. */
   Otk_windowmapped_state = 1;
   otk_last_redraw_time = otk_report_time() - 1.0;
  }
 otk_hover_box_check();

  /* Intercept any X-Window user interactions. */
  while (XPending( Otkdpy ))
   { /*Xloop*/
      XEvent event;
      KeySym ks;
      static int MouseDx, MouseDy;

      XNextEvent( Otkdpy, &event );  /* XtDispatchEvent( event ); */
      switch ( event.type )
       { /*case*/
        case ConfigureNotify:   if (event.xconfigure.width * event.xconfigure.height < OtkWindowSizeX * OtkWindowSizeY)
				 { otk_highest_qual_tried = 0; }
				OtkWindowSizeX = event.xconfigure.width;
				OtkWindowSizeY = event.xconfigure.height;
				if (OtkWindowSizeX < 2) OtkWindowSizeX = 2;
				if (OtkWindowSizeY < 2) OtkWindowSizeY = 2;
				if (Otk_verbose) printf("Resized window %d %d\n",  OtkWindowSizeX, OtkWindowSizeY );
				glViewport( 0, 0, OtkWindowSizeX, OtkWindowSizeY );
				Otk_Display_Changed = 1;
		break;
        case Expose:		
				Otk_Display_Changed = 1;
		break;
	case ClientMessage:   if (event.xclient.data.l[0] == wmDeleteWindow)	/* Window forced closed by window manager. */
				{   if (OtkOuterWindow->callback != 0)
				     OtkOuterWindow->callback( OtkOuterWindow->callback );
				    else exit(0);
				}
                break;
	case MotionNotify:	MouseDx = -(Otk_MousePixX - event.xmotion.x);
				MouseDy = -(Otk_MousePixY - event.xmotion.y);
				Otk_handle_mouse_move( MouseDx, MouseDy );
				Otk_MousePixX = event.xbutton.x;
				Otk_MousePixY = event.xbutton.y;
		break;
	case ButtonPress:	Otk_MousePixX = event.xbutton.x;
				Otk_MousePixY = event.xbutton.y;
				// printf("Button Press (%d,%d)\n", Otk_MousePixX, Otk_MousePixY);
				Otk_handle_mouse_click(0, event.xbutton.button);
				if (event.xbutton.button==2)  otk_paste_textbuf();
                break;
	case ButtonRelease:	Otk_MousePixX = event.xbutton.x;
				Otk_MousePixY = event.xbutton.y;
				// printf("Button Press (%d,%d)\n", Otk_MousePixX, Otk_MousePixY);
				Otk_handle_mouse_click(1, event.xbutton.button);
                break;
	case KeyPress:
			{
			 ks = XLookupKeysym( (XKeyEvent *) &event, 0 );
			 if (Otk_showkey) printf("Key=%d\n", (int)ks);
			 Otk_handle_key_input(ks);
			}
                break;
	case KeyRelease:
			ks = XLookupKeysym( (XKeyEvent *) &event, 0 );
			// printf("KeyRelease %d\n", ks);
			Otk_handle_key_release(ks);
       } /*case*/

   } /*Xloop*/

  #else 

  /*WinGLEnabled==1*/
  static int char_event = 0, MouseDx, MouseDy;
  MSG event;
  DWORD ks;

  if (!Otk_windowmapped_state)
   {
    Otk_windowmapped_state = 1;
    otk_last_redraw_time = otk_report_time() - 1.0;
    OtkDisplayFunct();
   }
  otk_hover_box_check();

    while (PeekMessage( &event, Otkwin, 0, 0, PM_REMOVE )) 
    { /*HandleEvent*/ 

     switch( event.message )
      { /*case*/
       case WM_SIZE:       if (GET_X_LPARAM(event.lParam) * GET_Y_LPARAM(event.lParam) < OtkWindowSizeX * OtkWindowSizeY)
			    { otk_highest_qual_tried = 0; }
		           OtkWindowSizeX = GET_X_LPARAM(event.lParam);
		           OtkWindowSizeY = GET_Y_LPARAM(event.lParam);
			   if (OtkWindowSizeX < 2) OtkWindowSizeX = 2;
			   if (OtkWindowSizeY < 2) OtkWindowSizeY = 2;
			   if (Otk_verbose) printf("Resized window %d %d\n",  OtkWindowSizeX, OtkWindowSizeY );
		           glViewport( 0, 0, OtkWindowSizeX, OtkWindowSizeY );
			   //glGetIntegerv( GL_VIEWPORT, Otk->viewport );
			   Otk_Display_Changed++;
	       break;
       case WM_CLOSE:      /* Window forced closed by window manager. */
			   if (OtkOuterWindow->callback != 0)
			    OtkOuterWindow->callback( OtkOuterWindow->callback );
			   else exit(0);
               break;
       case WM_MOUSEMOVE:
	                   // if( Otk_MouseButtonState[1] )
			    {
                             MouseDx = -(Otk_MouseLastX - GET_X_LPARAM(event.lParam));
			     MouseDy = -(Otk_MouseLastY - GET_Y_LPARAM(event.lParam));
			     Otk_MousePixX = GET_X_LPARAM(event.lParam);
			     Otk_MousePixY = GET_Y_LPARAM(event.lParam);
			     Otk_handle_mouse_move( MouseDx, MouseDy );
			     Otk_MouseLastX = GET_X_LPARAM(event.lParam);
			     Otk_MouseLastY = GET_Y_LPARAM(event.lParam);
			    }
	       break;
       case WM_LBUTTONDOWN:
	                    Otk_MouseButtonState[1] = 1;
			    Otk_MousePixX = GET_X_LPARAM(event.lParam);
			    Otk_MousePixY = GET_Y_LPARAM(event.lParam);
	                    Otk_handle_mouse_click(0,1);
			    Otk_MouseLastX = Otk_MousePixX;
			    Otk_MouseLastY = Otk_MousePixY;
			    // printf( "got mouse down event %d %d\n", GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) );
	 fflush(stdout);
               break;
       case WM_LBUTTONUP:
	                    Otk_MouseButtonState[1] = 0;
	                    Otk_handle_mouse_click(1, 1);
			    // printf( "got mouse up event %d %d\n", GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) );
	 fflush(stdout);
               break;

       case WM_RBUTTONDOWN:
	                    Otk_MouseButtonState[3] = 1;
			    Otk_MousePixX = GET_X_LPARAM(event.lParam);
			    Otk_MousePixY = GET_Y_LPARAM(event.lParam);
	                    Otk_handle_mouse_click(0,3);
			    Otk_MouseLastX = Otk_MousePixX;
			    Otk_MouseLastY = Otk_MousePixY;
			    // printf( "got mouse down event %d %d\n", GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) );
	 fflush(stdout);
               break;
       case WM_RBUTTONUP:
	                    Otk_MouseButtonState[3] = 0;
	                    Otk_handle_mouse_click(1, 3);
			    // printf( "got mouse up event %d %d\n", GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) );
	 fflush(stdout);
               break;


       case WM_MBUTTONDOWN:
	                    Otk_MouseButtonState[2] = 1;
			    Otk_MousePixX = GET_X_LPARAM(event.lParam);
			    Otk_MousePixY = GET_Y_LPARAM(event.lParam);
	                    Otk_handle_mouse_click(0,2);
			    Otk_MouseLastX = Otk_MousePixX;
			    Otk_MouseLastY = Otk_MousePixY;
			    otk_paste_textbuf();
			    //printf( "got middle mouse down event %d %d\n", GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) );
	 fflush(stdout);
               break;
       case WM_MBUTTONUP:
	                    Otk_MouseButtonState[2] = 0;
	                    Otk_handle_mouse_click(1, 2);
			 fflush(stdout);
               break;

       case WM_KEYDOWN: if (Otk_verbose>100) printf( "got key down %c\n", (char)event.wParam );
	 fflush( stdout );
	                char_event = 1;
			switch (event.wParam)
			 {
			  case VK_UP:   case VK_LEFT:   case VK_DOWN:	case VK_RIGHT:  
					Otk_handle_key_input( (int)(event.wParam) + 2000 );
			     break;
			  case 16:  Otk_Keyboard_state.shiftkey = 1;	break;
			  case 17:  Otk_Keyboard_state.ctrlkey = 1;	break;
		 	  case 46:  Otk_handle_key_input( 65535 );	break;
			  default:  TranslateMessage( &event ); DispatchMessage( &event );
			     break;
			 }
	       break;
       case WM_KEYUP:
	                char_event = 2;
		        switch (event.wParam)
			 {
			  case 16:  Otk_Keyboard_state.shiftkey = 0;	break;
			  case 17:  Otk_Keyboard_state.ctrlkey = 0;	break;
			 }
			TranslateMessage( &event ); DispatchMessage( &event );
	       break;
       case WM_SYSCHAR:
       case WM_CHAR:
	                ks = event.wParam;
			if( char_event == 1 )
			 {
			  if (Otk_verbose>100) printf( "got key down %c\n", (char)event.wParam );
			  fflush( stdout );
			  if ((Otk_Keyboard_state.ctrlkey) && (ks == 22))
			   otk_paste_textbuf();
			  else
			   Otk_handle_key_input(ks);
			 }
			else if( char_event == 2 )
			 {
			  if (Otk_verbose>100) printf( "got key up %c\n", (char)event.wParam );
			  fflush( stdout );
			  Otk_handle_key_release( ks );
			 }
			char_event = 0;
		break;
       case WM_MOUSEWHEEL:
		  { int j;
			j = (int)(event.wParam);
			if (j > 0)
			 { /*scroll-wheel-up*/
	                    Otk_MouseButtonState[4] = 0;
	                    Otk_handle_mouse_click(0, 4);
	                    Otk_handle_mouse_click(1, 4);
			 } /*scroll-wheel-up*/
			else
			 { /*scroll-wheel-down*/
	                    Otk_MouseButtonState[5] = 0;
	                    Otk_handle_mouse_click(0, 5);
	                    Otk_handle_mouse_click(1, 5);
			 } /*scroll-wheel-down*/
		  }
		break;
       default: 
		TranslateMessage( &event ); DispatchMessage( &event );	// Dispatch The Message
	       break;
     } /*case*/
    } /*HandleEvent*/ 

   #endif

  // printf(" Otk_Display_Changed = %d, TsinceLastDraw = %g (frame_period = %g)\n", Otk_Display_Changed, otk_report_time() - otk_last_redraw_time, OTK_FRAME_PERIOD );
  // Otk_Display_Changed = 10;

 
  if (otk_report_time() - otk_last_redraw_time > 0.1)
   {
    if (Otk_Display_Changed > 0) 
     {
      // printf("--- Redraw ---\n");
      otk_last_redraw_time = otk_report_time();
      if (BLEND) glEnable(GL_BLEND);
      OtkDisplayFunct(); 
      if (BLEND) glDisable(GL_BLEND);
      Otk_Display_Changed = 0;

      if (OTK_FRAME_PERIOD==0.0)
       OTK_FRAME_PERIOD = otk_report_time() - otk_last_redraw_time;
      else
       OTK_FRAME_PERIOD = 0.9 * OTK_FRAME_PERIOD + 0.1 * (otk_report_time() - otk_last_redraw_time);
      // printf("OtkUpdate Frame_time = %g\n", OTK_FRAME_PERIOD );
     }
    else
    if ((allow_cpu_antialiasing) && (otk_last_render_quality <= otk_render_qual_level) && (otk_last_render_quality < 3) && (otk_report_time() - otk_last_redraw_time > 0.8)
	&& ((otk_got_accumbuf) || (otk_render_qual_level == 0)))
     { int prev_qual = otk_render_qual_level;   /* Idle for a while, so improve quality if possible. */
       // printf("\nCurrent Qual = %d, (Last drawn at %d), Re-rendering at %d\n", otk_render_qual_level, otk_last_render_quality, otk_last_render_quality + 1 );
       otk_last_redraw_time = otk_report_time();
       otk_render_qual_level = otk_last_render_quality + 1;
       otk_suspend_quality_adjustments = 1;
       Otk_Display_Changed = 1;
       OtkDisplayFunct(); 
       Otk_Display_Changed = 0;
       otk_render_qual_level = prev_qual;
       otk_suspend_quality_adjustments = 0;
     }
   }

 if (otk_make_movie) otk_snapmovieframe();
 otk_check_timer();

#endif

}



int Otk_UpdateMouse( int x, int y, int state )	/* For when using another application's event-loop to update Otk's mouse-state. */
{
 Otk_MousePixX = x;
 Otk_MousePixY = y;
 return Otk_handle_mouse_click( state, 1 );
}

void Otk_UpdateMousePosition( int x, int y )  /* For when using another application's event-loop to update Otk's mouse-state. */
{
 Otk_MousePixX = x;
 Otk_MousePixY = y;
}




#if (GlutEnabled==0)
 #if (WinGLEnabled==0)
  #define otk_event_type XEvent 
 #else
  #define otk_event_type MSG 
 #endif

struct otk_event_list	/* Keyboard/mouse events are pushed onto tail, removed from head in fifo order. */
 {
  int event_type, x, y, dx, dy, data;
  struct otk_event_list *nxt;
 } *otk_event_list_head=0, *otk_event_list_tail=0, *otk_event_free_list=0;

void otk_free_event_item( struct otk_event_list *oldevent )
{
 oldevent->nxt = otk_event_free_list;
 otk_event_free_list = oldevent;
}

void otk_push_event( otk_event_type event )	/* Push event onto tail. */
{
 struct otk_event_list *newevent;
 if (otk_event_free_list==0)
  newevent = (struct otk_event_list *)malloc(sizeof(struct otk_event_list));
 else { newevent = otk_event_free_list;  otk_event_free_list = otk_event_free_list->nxt; }
 if (otk_event_list_tail==0) otk_event_list_head = newevent;
 else otk_event_list_tail->nxt = newevent;
 otk_event_list_tail = newevent;
 newevent->nxt = 0;
 // newevent->event = event;
}

void otk_pop_event()		/* Pop event from head. */
{
 struct otk_event_list *newevent;
 // if (otk_event_list_head==0) return (otk_event_type)0;
 newevent = otk_event_list_head;
 otk_event_list_head = otk_event_list_head->nxt;
 if (otk_event_list_head==0) otk_event_list_tail = 0;
 newevent->nxt = otk_event_free_list;
 otk_event_free_list = newevent;
 // return newevent->event;
}

#endif	/* Not Glut. */




struct otk_timer_queue_record
 {
   int id;
   double timeout;
   void (*callback)(void *x);
   void *parameter;
   struct otk_timer_queue_record *nxt;
 } *otk_timer_queue=0;		/* Time-ordered list of callbacks. */
int otk_timer_id=0;


int Otk_Set_Timer( double t_secs, void (*callback)(void *x), void *parameter )	/* Returns unique timer-event ID. */
{
 struct otk_timer_queue_record *item, *last=0;

 t_secs = t_secs + otk_report_time();
 item = otk_timer_queue;
 while ((item != 0) && (item->timeout <= t_secs))
  { last = item;  item = item->nxt; }
 item = (struct otk_timer_queue_record *)malloc( sizeof(struct otk_timer_queue_record) );
 item->id = otk_timer_id;
 item->timeout = t_secs;
 item->callback = callback;
 item->parameter = parameter;
 if (last == 0)
  { otk_timer_queue = item;  item->nxt = 0; }
 else
  { item->nxt = last->nxt;  last->nxt = item; }
 return otk_timer_id++;
}


void otk_check_timer()
{
 struct otk_timer_queue_record *olditem;
 double Tnow;
 Tnow = otk_report_time();
 while ((otk_timer_queue != 0) && (otk_timer_queue->timeout <= Tnow))
  {
   olditem = otk_timer_queue;
   otk_timer_queue = otk_timer_queue->nxt;
   olditem->callback( olditem->parameter );	/* Call the callback. */
   free( olditem );
  }
}



 /* OtkMainLoop. */
void OtkMainLoop()
{
#if (GlutEnabled==0)
 #if (WinGLEnabled==0)

  XMapWindow( Otkdpy, Otkwin );	/* Map the Window. */
  Otk_windowmapped_state = 1;
  otk_last_redraw_time = otk_report_time() - 1.0;

 if (otk_test_playback) Otk_Apply_Test_Stimuli( "otk_test_capture.xml" );

 while (1)
  { /*outer*/

    otk_hover_box_check();
    if (Otk_Display_Changed > 0)
     { 
      otk_last_redraw_time = otk_report_time();
      OtkDisplayFunct(); 
      Otk_Display_Changed = 0;
      if (OTK_FRAME_PERIOD==0.0)
 	OTK_FRAME_PERIOD = otk_report_time() - otk_last_redraw_time;
      else
	OTK_FRAME_PERIOD = 0.9 * OTK_FRAME_PERIOD + 0.1 * (otk_report_time() - otk_last_redraw_time);
      // printf("Frame_time = %g\n", OTK_FRAME_PERIOD );
     }
    else
    if ((allow_cpu_antialiasing) && (otk_last_render_quality <= otk_render_qual_level) && (otk_last_render_quality < 3) && (otk_report_time() - otk_last_redraw_time > 0.8)
	&& ((otk_got_accumbuf) || (otk_render_qual_level == 0)))
     { int prev_qual = otk_render_qual_level;	/* Idle for a while, so improve quality if possible. */
      otk_last_redraw_time = otk_report_time();
      otk_render_qual_level = otk_last_render_quality + 1;
      otk_suspend_quality_adjustments = 1;
      OtkDisplayFunct(); 
      otk_render_qual_level = prev_qual;
      otk_suspend_quality_adjustments = 0;
     }
    else
     usleep(100000);		/* Conserve CPU by running no faster than ~20 Hz. */

    /* Intercept any X-Window user interactions. */
    otk_last_redraw_time = otk_report_time();
    while ((XPending( Otkdpy )) && (otk_report_time() - otk_last_redraw_time < 0.25))		// (Otk_Display_Changed < 10))
     { /*Xloop*/
      XEvent event;
      KeySym ks;
      static int MouseDx, MouseDy;

      XNextEvent( Otkdpy, &event );  /* XtDispatchEvent( event ); */
      switch( event.type )
       { /*case*/
        case ConfigureNotify:   if (event.xconfigure.width * event.xconfigure.height < OtkWindowSizeX * OtkWindowSizeY)
				 { otk_highest_qual_tried = 0; }
				OtkWindowSizeX = event.xconfigure.width;
				OtkWindowSizeY = event.xconfigure.height;
				if (OtkWindowSizeX < 2) OtkWindowSizeX = 2;
				if (OtkWindowSizeY < 2) OtkWindowSizeY = 2;
				if (Otk_verbose) printf("Resized window %d %d\n",  OtkWindowSizeX, OtkWindowSizeY );
				glViewport( 0, 0, event.xconfigure.width, event.xconfigure.height );
				Otk_Display_Changed = 1;
		break;
        case Expose:		
				Otk_Display_Changed = 1;
		break;
	case ClientMessage:   if (event.xclient.data.l[0] == wmDeleteWindow)	/* Window forced closed by window manager. */
				{   if (OtkOuterWindow->callback != 0)
				     OtkOuterWindow->callback( OtkOuterWindow->callback_param );
				    else exit(0);
				}
                break;
	case MotionNotify:	MouseDx = -(Otk_MousePixX - event.xmotion.x);
				MouseDy = -(Otk_MousePixY - event.xmotion.y);
				Otk_handle_mouse_move( MouseDx, MouseDy );
				Otk_MousePixX = event.xbutton.x;
				Otk_MousePixY = event.xbutton.y;
		break;
	case ButtonPress:	Otk_MousePixX = event.xbutton.x;
				Otk_MousePixY = event.xbutton.y;
				// printf("Button Press (%d,%d)\n", Otk_MousePixX, Otk_MousePixY);
				Otk_handle_mouse_click(0, event.xbutton.button);
				if (event.xbutton.button==2)  otk_paste_textbuf();
                break;
	case ButtonRelease:	Otk_MousePixX = event.xbutton.x;
				Otk_MousePixY = event.xbutton.y;
				// printf("Button Press (%d,%d)\n", Otk_MousePixX, Otk_MousePixY);
				Otk_handle_mouse_click(1, event.xbutton.button);
                break;
	case KeyPress:
			{
			 ks = XLookupKeysym( (XKeyEvent *) &event, 0 );
			 if (Otk_showkey) printf("Key=%d\n", (int)ks);
			 Otk_handle_key_input(ks);
			}
                break;
	case KeyRelease:
			ks = XLookupKeysym( (XKeyEvent *) &event, 0 );
			// printf("KeyRelease %d\n", ks);
			Otk_handle_key_release(ks);
       } /*case*/
     } /*Xloop*/
    if (otk_make_movie) otk_snapmovieframe();
    if (otk_timer_queue != 0) otk_check_timer();
  } /*outer*/

#else

  /*WinGLEnabled==1*/
  static int char_event = 0, MouseDx, MouseDy;
  MSG event;
  DWORD ks;

  otk_last_redraw_time = otk_report_time() - 1.0;

  if (otk_test_playback) Otk_Apply_Test_Stimuli( "otk_test_capture.xml" );

  while (1)
  { /*Winloop*/

    otk_hover_box_check();
    if (Otk_Display_Changed > 0) 
     { 
      otk_last_redraw_time = otk_report_time();
       OtkDisplayFunct();
      Otk_Display_Changed = 0;
      if (OTK_FRAME_PERIOD==0.0)
 	OTK_FRAME_PERIOD = otk_report_time() - otk_last_redraw_time;
      else
	OTK_FRAME_PERIOD = 0.9 * OTK_FRAME_PERIOD + 0.1 * (otk_report_time() - otk_last_redraw_time);
     }
    else
    if ((allow_cpu_antialiasing) && (otk_last_render_quality <= otk_render_qual_level) && (otk_last_render_quality < 3) && (otk_report_time() - otk_last_redraw_time > 0.8)
	&& ((otk_got_accumbuf) || (otk_render_qual_level == 0)))
     { int prev_qual = otk_render_qual_level;	/* Idle for a while, so improve quality if possible. */
      otk_last_redraw_time = otk_report_time();
      otk_render_qual_level = otk_last_render_quality + 1;
      otk_suspend_quality_adjustments = 1;
      OtkDisplayFunct(); 
      otk_render_qual_level = prev_qual;
      otk_suspend_quality_adjustments = 0;
     }
    else
     Sleep(100);	/* Conserve CPU by running no faster than ~20 Hz. */

    otk_last_redraw_time = otk_report_time();
    while ((PeekMessage( &event, Otkwin, 0, 0, PM_REMOVE )) && (otk_report_time() - otk_last_redraw_time < 0.25)) 
    { /*HandleEvent*/ 

     switch( event.message )
      { /*case*/
       case WM_SIZE:       if (GET_X_LPARAM(event.lParam) * GET_Y_LPARAM(event.lParam) < OtkWindowSizeX * OtkWindowSizeY)
			    { otk_highest_qual_tried = 0; }
		           OtkWindowSizeX = GET_X_LPARAM(event.lParam);
		           OtkWindowSizeY = GET_Y_LPARAM(event.lParam);
			   if (OtkWindowSizeX < 2) OtkWindowSizeX = 2;
			   if (OtkWindowSizeY < 2) OtkWindowSizeY = 2;
			   if (Otk_verbose) printf("resized window %d %d\n", OtkWindowSizeX, OtkWindowSizeY );
		           glViewport( 0, 0, OtkWindowSizeX, OtkWindowSizeY );
			   //glGetIntegerv( GL_VIEWPORT, Otk->viewport );
			   Otk_Display_Changed++;
	       break;
       case WM_CLOSE:      /* Window forced closed by window manager. */
			   if (OtkOuterWindow->callback != 0)
			    OtkOuterWindow->callback( OtkOuterWindow->callback_param );
			   else exit(0);
               break;
       case WM_MOUSEMOVE:
	                   // if( Otk_MouseButtonState[1] )
			    {
                             MouseDx = -(Otk_MouseLastX - GET_X_LPARAM(event.lParam));
			     MouseDy = -(Otk_MouseLastY - GET_Y_LPARAM(event.lParam));
			     Otk_MousePixX = GET_X_LPARAM(event.lParam);
			     Otk_MousePixY = GET_Y_LPARAM(event.lParam);
			     Otk_handle_mouse_move( MouseDx, MouseDy );
			     Otk_MouseLastX = GET_X_LPARAM(event.lParam);
			     Otk_MouseLastY = GET_Y_LPARAM(event.lParam);
			    }
	       break;
       case WM_LBUTTONDOWN:
	                    Otk_MouseButtonState[1] = 1;
			    Otk_MousePixX = GET_X_LPARAM(event.lParam);
			    Otk_MousePixY = GET_Y_LPARAM(event.lParam);
	                    Otk_handle_mouse_click(0, 1);
			    Otk_MouseLastX = Otk_MousePixX;
			    Otk_MouseLastY = Otk_MousePixY;
			    //printf( "got mouse down event %d %d\n", GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) );
	 fflush(stdout);
               break;
       case WM_LBUTTONUP:
	                    Otk_MouseButtonState[1] = 0;
	                    Otk_handle_mouse_click(1, 1);
			    //printf( "got mouse up event %d %d\n", GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) );
	 fflush(stdout);
               break;

       case WM_RBUTTONDOWN:
	                    Otk_MouseButtonState[3] = 1;
			    Otk_MousePixX = GET_X_LPARAM(event.lParam);
			    Otk_MousePixY = GET_Y_LPARAM(event.lParam);
	                    Otk_handle_mouse_click(0, 3);
			    Otk_MouseLastX = Otk_MousePixX;
			    Otk_MouseLastY = Otk_MousePixY;
			    //printf( "got mouse down event %d %d\n", GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) );
	 fflush(stdout);
               break;
       case WM_RBUTTONUP:
	                    Otk_MouseButtonState[3] = 0;
	                    Otk_handle_mouse_click(1, 3);
			    //printf( "got mouse up event %d %d\n", GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) );
	 fflush(stdout);
               break;

       case WM_MBUTTONDOWN:
	                    Otk_MouseButtonState[2] = 1;
			    Otk_MousePixX = GET_X_LPARAM(event.lParam);
			    Otk_MousePixY = GET_Y_LPARAM(event.lParam);
	                    Otk_handle_mouse_click(0, 2);
			    Otk_MouseLastX = Otk_MousePixX;
			    Otk_MouseLastY = Otk_MousePixY;
			    otk_paste_textbuf();
			    //printf( "got middle mouse down event %d %d\n", GET_X_LPARAM(event.lParam), GET_Y_LPARAM(event.lParam) );
	 fflush(stdout);
               break;
       case WM_MBUTTONUP:
			    Otk_MouseButtonState[2] = 0;
	                    Otk_handle_mouse_click(1, 2);
               break;

       case WM_KEYDOWN: if (Otk_verbose>100) printf( "got key down %c\n", (char)event.wParam );
	 fflush( stdout );
	                char_event = 1;
			switch (event.wParam)
			 {
			  case VK_UP:   case VK_LEFT:   case VK_DOWN:	case VK_RIGHT:  
					Otk_handle_key_input( (int)(event.wParam) + 2000 );
			     break;
			  case 16:  Otk_Keyboard_state.shiftkey = 1;	break;
			  case 17:  Otk_Keyboard_state.ctrlkey = 1;	break;
		 	  case 46:  Otk_handle_key_input( 65535 );	break;
			  default: 	TranslateMessage( &event ); DispatchMessage( &event );
			     break;
			 }
	       break;
       case WM_KEYUP:
	                char_event = 2;
		        switch (event.wParam)
			 {
			  case 16:  Otk_Keyboard_state.shiftkey = 0;	break;
			  case 17:  Otk_Keyboard_state.ctrlkey = 0;	break;
			 }
			TranslateMessage( &event ); DispatchMessage( &event );
	       break;
       case WM_SYSCHAR:
       case WM_CHAR:
	                ks = event.wParam;
			if( char_event == 1 )
			 {
			  if (Otk_verbose>100) printf( "got key down %c\n", (char)event.wParam );
			  fflush( stdout );
			  // printf("Otk_Keyboard_state.ctrlkey = %d	  ks = %d\n", Otk_Keyboard_state.ctrlkey, ks );
			  if ((Otk_Keyboard_state.ctrlkey) && (ks == 22))
			   otk_paste_textbuf();
			  else
			   Otk_handle_key_input(ks);
			 }
			else if( char_event == 2 )
			 {
			  if (Otk_verbose>100) printf( "got key up %c\n", (char)event.wParam );
			  fflush( stdout );
			  Otk_handle_key_release( ks );
			 }
			char_event = 0;
		break;
       case WM_MOUSEWHEEL:
		  { int j;
			j = (int)(event.wParam);
			if (j > 0)
			 { /*scroll-wheel-up*/
	                    Otk_MouseButtonState[4] = 0;
	                    Otk_handle_mouse_click(0, 4);
	                    Otk_handle_mouse_click(1, 4);
			 } /*scroll-wheel-up*/
			else
			 { /*scroll-wheel-down*/
	                    Otk_MouseButtonState[5] = 0;
	                    Otk_handle_mouse_click(0, 5);
	                    Otk_handle_mouse_click(1, 5);
			 } /*scroll-wheel-down*/
		  }
		break;
       default: 
		TranslateMessage( &event ); DispatchMessage( &event );	// Dispatch The Message
	       break;
     } /*case*/
    } /*HandleEvent*/ 
    if (otk_make_movie) otk_snapmovieframe();
    if (otk_timer_queue != 0) otk_check_timer();
   } /*Winloop*/

#endif
#else

 /* Register the key functions. */
 glutDisplayFunc( OtkDisplayFunct );
 glutReshapeFunc( Otk_reshapefunct );
 glutMotionFunc( Otk_mousemotion );
 glutMouseFunc( Otk_mousefunct );
 glutKeyboardFunc( Otk_keyeventfunct );
 glutIdleFunc( OtkDisplayFunct );
 glutSpecialFunc( Otk_speckeyeventfunct );

 glutMainLoop();
#endif
}
