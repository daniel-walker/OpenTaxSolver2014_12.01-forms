/***
   Run_taxsolve_GUI.c - This program merely invokes "bin/ots_gui2.exe", if
	running under Linux, or "msbin\ots_gui.exe", if running under Microsoft.
        It is a convenience, because it is easily found at the top directory
        of the OTS installation.  So new users can make a shortcut to it.
	And it enables starting the application by double-clicking on the
	start-icon (or file), unlike the previously used command-script.
	(Some window managers whould open the command-script for editing
	 instead of executing it, since being a script, it was a text file.
	 Other window managers would ask whether to run or display, which
	 this is an uneeded extra step, and it could also confuse some users.
	)

  Compile:
	cc -O Run_taxsolve_GUI.c -o ../Run_taxsolve_GUI
***/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void caps_cpy( char *wrd )
{
 int j=0;
 while (wrd[j] != '\0')
  {
   wrd[j] = toupper( wrd[j] );
   j++;
  }
}


void my_strcpy( char *dst, char *src, int maxlen )
{
 int j=0;
 while ((j < maxlen-1) && (src[j] != '\0'))
  { 
   dst[j] = src[j];
   j++;
  }
 dst[j] = '\0';
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


int main( int argc, char *argv[] )
{
 char path[4096], *substr, cmd[4096], bin_path[4096];

 strcpy( path, argv[0] );
//printf("Path: '%s'\n", path );

 #if (PLATFORM_KIND != Posix_Platform)

  { /*mswin*/
   char drive_letter='c';
   caps_cpy( path );
   if (path[1] == ':')
    { /*Absolute_reference*/
     drive_letter = path[0];
     my_strcpy( bin_path, &(path[2]), 4096 );
     substr = strstr(bin_path,"\\RUN_TAXSOLVE_GUI");
     if (substr != 0) 
      {
       substr[0] = '\0';
       strcat( bin_path, "\\msbin" );
       sprintf(cmd,"set path=%c:%s;%%PATH%% & start /B %c:\"%s\\ots_gui2.exe\"", drive_letter, bin_path, drive_letter, bin_path );
      }
     else { printf("Unexpected Error\n");  exit(1); }
    } /*Absolute_reference*/
   else 
    { /*Local_reference*/
//     printf("Start path must begin with drive-letter:\n");
//     if ((path[0] != '/') && (path[0] != '\\')) exit(1);
      my_strcpy( bin_path, path, 4096 );
      substr = strstr(bin_path,"RUN_TAXSOLVE_GUI");
      if (substr != 0)
       {
        substr[0] = '\0';
        if (strlen( bin_path ) > 0) strcat( bin_path, "msbin" );
        else strcpy( bin_path, "msbin" );
	sprintf(cmd,"start /B %s\\ots_gui2.exe", bin_path );
       }
      else { printf("Unexpected Error2\n");  exit(1); }
    } /*Local_reference*/
  } /*mswin*/

 #else

  { /*posix*/
   int j;
   substr = strstr( path, "Run_taxsolve_GUI" );
   if (substr == 0) { printf("Could not find path.\nSaw: '%s'\n", bin_path);  exit(1); }
   substr[0] = '\0';
   strcpy( bin_path, path );
   j = strlen( bin_path );
   if ((j > 0) && (bin_path[j-1] != '/')) strcat( bin_path, "/" );
   if (j == 0) strcpy( bin_path, "./" );
   strcat( bin_path, "bin/ots_gui2" );
   strcpy( cmd, bin_path );
   strcat( cmd, " &" );
  } /*posix*/

 #endif

 printf("Issuing: '%s'\n", cmd );
 system( cmd );

 return 0;
}

