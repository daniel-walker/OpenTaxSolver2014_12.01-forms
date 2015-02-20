/****************************************************************/
/* Letter2Vector.c - Portable vector font rendering function.	*/
/* The routines in this library accept a character and draw it	*/
/* in OpenGL as a vector-rendered character.  It accepts a 	*/
/* scaling, color, thickness, and slant (italics) values.       */
/* Scaling is specified independently in horizontal and height  */
/* to allow various aspect ratio display.  Position is 		*/
/* specified in x,y. 						*/
/****************************************************************/

#include <ctype.h>
#include <string.h>

float Otk_letter_x0, Otk_letter_h_scale, Otk_letter_v_scale, Otk_letter_slant, Otk_letter_xpos, Otk_letter_ypos, Otk_letter_zpos;
// float Otk_text_height, Otk_text_width;
int Otk_letter_orientation=0;	/* 0=horizontal, 1=vertical. */

#ifndef _GLUfuncptr
 #define _GLUfuncptr void *
#endif


void Otk_Draw_Vector( float x1, float y1, float x2, float y2 )
{
 float point[3];

//printf("Otk_letter_orientation=%d\n", Otk_letter_orientation);
 point[2] = Otk_letter_zpos;

 if (Otk_letter_orientation)
  {
   float ftmp;
   ftmp = x1;  x1 = -y1;  y1 = ftmp;
   ftmp = x2;  x2 = -y2;  y2 = ftmp;
  }

 point[0] = Otk_letter_xpos + Otk_letter_h_scale * (Otk_letter_x0 + Otk_letter_slant * x1 + y1);  
 point[1] = Otk_letter_ypos - Otk_letter_v_scale * x1;  
 glVertex3fv( point );
 point[0] = Otk_letter_xpos + Otk_letter_h_scale * (Otk_letter_x0 + Otk_letter_slant * x2 + y2);
 point[1] = Otk_letter_ypos - Otk_letter_v_scale * x2;
 glVertex3fv( point );
}



void Otk_Letter2Draw_Vector( float h, char c, float hscale, float vscale, float slant, float xpos, float ypos, float zpos )
{
 int x;						/* Draw each vector-based character within 10x8 character-box.	*/
						/* Coordinates can be floats, but can consider box to be	*/
 ypos = -ypos;					/* 10-rows by 8-columns.  Top left corner is row=0, col=1.	*/
 Otk_letter_x0 = h;				/* Bottom right corner is row=9, col=8.				*/
 Otk_letter_h_scale = 0.15 * hscale;		/* Column 0 is left blank to provide space between characters.	*/
 Otk_letter_v_scale = 0.13 * vscale;		/* Command to draw a line segment from point-a to point-b is:	*/
 Otk_letter_slant = slant;			/*	Otk_Draw_Vector( row_a, col_a,  row_b, col_b );		*/
 Otk_letter_xpos = xpos;			/* Actual coordinates, when placed, are scaled and shifted to	*/
 Otk_letter_ypos = ypos;			/* arbitrary size and aspect ratio.				*/
 Otk_letter_zpos = zpos;
 x = (int)c;
 switch (x)
  {
   case '\f':	/*formfeed*/
   case 32:  ;	/*(space)*/
	break;
   case 33:  ;	/* ! */
	Otk_Draw_Vector( 0, 4, 7, 4 );
	Otk_Draw_Vector( 8, 4, 9, 4 );
	break;
   case 34:	/* " */
	Otk_Draw_Vector( 2, 2, 0, 4 );
	Otk_Draw_Vector( 2, 4, 0, 6 );
	break;
   case 35:	/* # */
	Otk_Draw_Vector( 3, 1, 3, 7 );
	Otk_Draw_Vector( 6, 1, 6, 7 );
	Otk_Draw_Vector( 1, 3, 8, 3 );
	Otk_Draw_Vector( 1, 5, 8, 5 );
	break;
   case 36:	/*$*/
	Otk_Draw_Vector( 1,7,0,6 );
	Otk_Draw_Vector( 0,6,0,3 );
	Otk_Draw_Vector( 0,3,2,1 );
	Otk_Draw_Vector( 2,1,3,1 );
	Otk_Draw_Vector( 3,1,4,2 );
	Otk_Draw_Vector( 4,2,4,6 );
	Otk_Draw_Vector( 4,6,5,7 );
	Otk_Draw_Vector( 5,7,8,7 );
	Otk_Draw_Vector( 8,7,9,6 );
	Otk_Draw_Vector( 9,6,9,3 );
	Otk_Draw_Vector( 9,3,7,1 );
	Otk_Draw_Vector( 0, 4, 9, 4 );
	break;
   case 37:	/*%*/
	Otk_Draw_Vector( 0, 7, 9, 1  );
	Otk_Draw_Vector( 0, 2, 1, 3 );
	Otk_Draw_Vector( 1, 1, 2, 2 );
	Otk_Draw_Vector( 7, 6, 8, 7  );
	Otk_Draw_Vector( 8, 5, 9, 6 );
	break;
   case 38:	/*&*/
	Otk_Draw_Vector( 0,4,0,5 );
	Otk_Draw_Vector( 0,5,2,6 );
	Otk_Draw_Vector( 2,6,3,6 );
	Otk_Draw_Vector( 3,6,4,5 );
	Otk_Draw_Vector( 4,5,4,4 );
	Otk_Draw_Vector( 4,4,7,2 );
	Otk_Draw_Vector( 7,2,8,2 );
	Otk_Draw_Vector( 8,2,9,3 );
	Otk_Draw_Vector( 9,3,9,4 );
	Otk_Draw_Vector( 9,4,5.5,7 );
	Otk_Draw_Vector( 9,7,7,5 );
	Otk_Draw_Vector( 7,5,4,4 );
	Otk_Draw_Vector( 4,4,3,3 );
	Otk_Draw_Vector( 3,3,2,3 );
	Otk_Draw_Vector( 2,3,0,4 );
	break;
	break;
   case 39:	/*'*/
	Otk_Draw_Vector( 2, 3, 0, 5 );
	break;
   case 41: 	/*)*/
	Otk_Draw_Vector( 0, 5, 3, 7 );
	Otk_Draw_Vector( 3, 7, 6, 7 );
	Otk_Draw_Vector( 6, 7, 9, 6 );
	break;
   case 40:	/*(*/
	Otk_Draw_Vector( 0, 3, 3, 1 );
	Otk_Draw_Vector( 3, 1, 6, 1 );
	Otk_Draw_Vector( 6, 1, 9, 3 );
	break;
   case 42:	/***/
	Otk_Draw_Vector( 2, 2, 8, 6 );
	Otk_Draw_Vector( 2, 4, 8, 4 );
	Otk_Draw_Vector( 2, 6, 8, 2 );
	Otk_Draw_Vector( 5, 1, 5, 7 );
	break;
   case 43:	/*+*/
	Otk_Draw_Vector( 2, 4, 8, 4 );
	Otk_Draw_Vector( 5, 1, 5, 7 );
	break;
   case 44:	/*,*/
	Otk_Draw_Vector( 8, 5, 10, 3 );
	break;
   case 45:	/*-*/
	Otk_Draw_Vector( 5, 1, 5, 7 );
	break;
   case 46:	/*.*/
	Otk_Draw_Vector( 8, 4, 9,5 );
	Otk_Draw_Vector( 8,5,9,4 );
	break;
   case 47:	/* / */
	Otk_Draw_Vector( 0, 7, 9, 1  );
	break;
   case 48:	/*0*/
	Otk_Draw_Vector( 0,4,0,5 );
	Otk_Draw_Vector( 0,5,2,7 );
	Otk_Draw_Vector( 2,7,7,7 );
	Otk_Draw_Vector( 7,7,9,5 );
	Otk_Draw_Vector( 9,5,9,4 );
	Otk_Draw_Vector( 9,4,7,2 );
	Otk_Draw_Vector( 7,2,2,2 );
	Otk_Draw_Vector( 2,2,0,4 );
	break;
   case 49:	/*1*/
	Otk_Draw_Vector( 1,4,0,5 );
	Otk_Draw_Vector( 0,5,9,5 );
	break;
   case 50:	/*2*/
	Otk_Draw_Vector( 2,2,0,3.5 );
	Otk_Draw_Vector( 0,3.5,0,5.5 );
	Otk_Draw_Vector( 0,5.5,2,7 );
	Otk_Draw_Vector( 2,7,4,7 );
	Otk_Draw_Vector( 4,7,5,5 );
	Otk_Draw_Vector( 5,5,9,1 );
	Otk_Draw_Vector( 9,1,9,7 );
	break;
   case 51:	/*3*/
	Otk_Draw_Vector( 2,2,0,3.5 );
	Otk_Draw_Vector( 0,3.5,0,5.5 );
	Otk_Draw_Vector( 0,5.5,2,7 );
	Otk_Draw_Vector( 2,7,4,7 );
	Otk_Draw_Vector( 4,7,5,6 );
	Otk_Draw_Vector( 5,6,5,4 );
	Otk_Draw_Vector( 5,6,6,7 );
	Otk_Draw_Vector( 6,7,8,7 );
	Otk_Draw_Vector( 8,7,9,6 );
	Otk_Draw_Vector( 9,6,9,3 );
	Otk_Draw_Vector( 9,3,8,2 );
	break;
   case 52:	/*4*/
	Otk_Draw_Vector( 6,1,0,6 );
	Otk_Draw_Vector( 0,6,9,6 );
	Otk_Draw_Vector( 6,1,6,6 );
	break;
   case 53:	/*5*/
	Otk_Draw_Vector( 0,2,0,7 );
	Otk_Draw_Vector( 0,2,4,2 );
	Otk_Draw_Vector( 4,2,3,4 );
	Otk_Draw_Vector( 3,4,3,5 );
	Otk_Draw_Vector( 3,5,5,7 );
	Otk_Draw_Vector( 5,7,7,7 );
	Otk_Draw_Vector( 7,7,9,5 );
	Otk_Draw_Vector( 9,5,9,3 );
	Otk_Draw_Vector( 9,3,8,2 );
	break;
   case 54:	/*6*/
	Otk_Draw_Vector( 1,7,0,6 );
	Otk_Draw_Vector( 0,6,0,4 );
	Otk_Draw_Vector( 0,4,2,2 );
	Otk_Draw_Vector( 2,2,8,2 );
	Otk_Draw_Vector( 8,2,9,3 );
	Otk_Draw_Vector( 9,3,9,6 );
	Otk_Draw_Vector( 9,6,8,7 );
	Otk_Draw_Vector( 8,7,6,7 );
	Otk_Draw_Vector( 6,7,5,6 );
	Otk_Draw_Vector( 5,6,5,2 );
	break;
   case 55:	/*7*/
	Otk_Draw_Vector( 0,2,0,7 );
	Otk_Draw_Vector( 0,7,9,3 );
	break;
   case 56:	/*8*/
	Otk_Draw_Vector( 0,4,0,5 );
	Otk_Draw_Vector( 0,5,2,7 );
	Otk_Draw_Vector( 2,7,3,7 );
	Otk_Draw_Vector( 3,7,5,5 );
	Otk_Draw_Vector( 5,5,5,4 );
	Otk_Draw_Vector( 5,4,7,2 );
	Otk_Draw_Vector( 7,2,8,2 );
	Otk_Draw_Vector( 8,2,9,3 );
	Otk_Draw_Vector( 9,3,9,6 );
	Otk_Draw_Vector( 9,6,8,7 );
	Otk_Draw_Vector( 8,7,7,7 );
	Otk_Draw_Vector( 7,7,5,5 );
	Otk_Draw_Vector( 5,5,5,4 );
	Otk_Draw_Vector( 5,4,3,2 );
	Otk_Draw_Vector( 3,2,2,2 );
	Otk_Draw_Vector( 2,2,0,4 );
	break;
   case 57:	/*9*/
	Otk_Draw_Vector( 0,4,0,6 );
	Otk_Draw_Vector( 0,6,1,7 );
	Otk_Draw_Vector( 1,7,5,7 );
	Otk_Draw_Vector( 5,7,9,5 );
	Otk_Draw_Vector( 4,7,4,4 );
	Otk_Draw_Vector( 4,4,2,2 );
	Otk_Draw_Vector( 2,2,0,4 );
	break;
   case 58:	/*:*/
	Otk_Draw_Vector( 3,4,4,5 );
	Otk_Draw_Vector( 4,4,3,5 );
	Otk_Draw_Vector( 8,4,9,5 );
	Otk_Draw_Vector( 9,4,8,5 );
	break;
   case 59:	/*;*/
	Otk_Draw_Vector( 3,4,4,5 );
	Otk_Draw_Vector( 4,4,3,5 );
	Otk_Draw_Vector( 8,4,8.5,4.5 );
	Otk_Draw_Vector( 10,3,8,5 );
	break;
   case 60:	/*<*/
	Otk_Draw_Vector( 1,7,5,1);
	Otk_Draw_Vector( 5,1,9,7 );
	break;
   case 61:	/*=*/
	Otk_Draw_Vector( 3,2,3,6 );
	Otk_Draw_Vector( 6,2,6,6 );
	break;
   case 62:	/*>*/
	Otk_Draw_Vector( 1,1,5,7);
	Otk_Draw_Vector( 5,7,9,1 );
	break;
   case 63:	/*?*/
	Otk_Draw_Vector( 2,2,0,4 );
	Otk_Draw_Vector( 0,4,0,6 );
	Otk_Draw_Vector( 0,6,1,7 );
	Otk_Draw_Vector( 1,7,3,7 );
	Otk_Draw_Vector( 3,7,4,6 );
	Otk_Draw_Vector( 4,6,4,5 );
	Otk_Draw_Vector( 4,5,5,4 );
	Otk_Draw_Vector( 5,4,6,4 );
	Otk_Draw_Vector( 8,4,9,4 );
	break;
   case 64:	/*@*/
	Otk_Draw_Vector( 0,4,0,5 );
	Otk_Draw_Vector( 0,5,2,7 );
	Otk_Draw_Vector( 2,7,6,7 );
	Otk_Draw_Vector( 6,7,7,5 );
	Otk_Draw_Vector( 7,5, 7,4 );
	Otk_Draw_Vector( 7,4, 4,4 );
	Otk_Draw_Vector( 4,4, 4,6 );
	Otk_Draw_Vector( 9,6,9,3.5 );
	Otk_Draw_Vector( 9,3.5, 8,2.5 );
	Otk_Draw_Vector( 8, 2.5, 7,2 );
	Otk_Draw_Vector( 7,2,2,2 );
	Otk_Draw_Vector( 2,2,0,4 );
	break;
   case 65:	/*A*/
	Otk_Draw_Vector( 9,1,0,4 );
	Otk_Draw_Vector( 0,4,9,7 );
	Otk_Draw_Vector( 7,2,7,6 );
	break;
   case 66:	/*B*/
	Otk_Draw_Vector( 0,1,0,5 );
	Otk_Draw_Vector( 0,5,2,7 );
	Otk_Draw_Vector( 2,7,3,7 );
	Otk_Draw_Vector( 3,7,4,6 );
	Otk_Draw_Vector( 4,6,4,1 );
	Otk_Draw_Vector( 4,5,6,7 );
	Otk_Draw_Vector( 6,7,8,7 );
	Otk_Draw_Vector( 8,7,9,6 );
	Otk_Draw_Vector( 9,6,9,1 );
	Otk_Draw_Vector( 9,1,0,1 );
	break;
   case 67:	/*C*/
	Otk_Draw_Vector( 1,7,0,6 );
	Otk_Draw_Vector( 0,6,0,3 );
	Otk_Draw_Vector( 0,3,2,1 );
	Otk_Draw_Vector( 2,1,7,1 );
	Otk_Draw_Vector( 7,1,9,3 );
	Otk_Draw_Vector( 9,3,9,6 );
	Otk_Draw_Vector( 9,6,8,7 );
	break;
   case 68:	/*D*/
	Otk_Draw_Vector( 0,1,0,5 );
	Otk_Draw_Vector( 0,5,2,7 );
	Otk_Draw_Vector( 2,7,7,7 );
	Otk_Draw_Vector( 7,7,9,5 );
	Otk_Draw_Vector( 9,5,9,1 );
	Otk_Draw_Vector( 9,1,0,1 );
	break;
   case 69:	/*E*/
	Otk_Draw_Vector( 0,7,0,1 );
	Otk_Draw_Vector( 0,1,9,1 );
	Otk_Draw_Vector( 9,1,9,7 );
	Otk_Draw_Vector( 5,1,5,6 );
	break;
   case 70:	/*F*/
	Otk_Draw_Vector( 0,7,0,1 );
	Otk_Draw_Vector( 0,1,9,1 );
	Otk_Draw_Vector( 5,1,5,5 );
	break;
   case 71:	/*G*/
	Otk_Draw_Vector( 1,7,0,6 );
	Otk_Draw_Vector( 0,6,0,3 );
	Otk_Draw_Vector( 0,3,2,1 );
	Otk_Draw_Vector( 2,1,7,1 );
	Otk_Draw_Vector( 7,1,9,3 );
	Otk_Draw_Vector( 9,3,9,6 );
	Otk_Draw_Vector( 9,6,7,7 );
	Otk_Draw_Vector( 7,7,5,7 );
	Otk_Draw_Vector( 5,7,5,5 );
	Otk_Draw_Vector( 5,5,6,5 );
	break;
   case 72:	/*H*/
	Otk_Draw_Vector( 0,1,9,1 );
	Otk_Draw_Vector( 5,1,5,7 );
	Otk_Draw_Vector( 0,7,9,7 );
	break;
   case 73:	/*I*/
	Otk_Draw_Vector( 0,3,0,5 );
	Otk_Draw_Vector( 0,4,9,4 );
	Otk_Draw_Vector( 9,3,9,5 );
	break;
   case 74:	/*J*/
	Otk_Draw_Vector( 0,5,0,7 );
	Otk_Draw_Vector( 0,6,7,6 );
	Otk_Draw_Vector( 7,6,9,4 );
	Otk_Draw_Vector( 9,4,9,3 );
	Otk_Draw_Vector( 9,3,7,1 );
	break;
   case 75:	/*K*/
	Otk_Draw_Vector( 0,1,9,1 );
	Otk_Draw_Vector( 0,7,4,1 );
	Otk_Draw_Vector( 4,1,9,7 );
	break;
   case 76:	/*L*/
	Otk_Draw_Vector( 0,1,9,1 );
	Otk_Draw_Vector( 9,1,9,7 );
	break;
   case 77:	/*M*/
	Otk_Draw_Vector( 9,1,0,1 );
	Otk_Draw_Vector( 0,1,4,4 );
	Otk_Draw_Vector( 4,4,0,7 );
	Otk_Draw_Vector( 0,7,9,7 );
	break;
   case 78:	/*N*/
	Otk_Draw_Vector( 9,1,0,1 );
	Otk_Draw_Vector( 0,1,9,7 );
	Otk_Draw_Vector( 9,7,0,7 );
	break;
   case 79:	/*O*/
	Otk_Draw_Vector( 2,7,0,5 );
	Otk_Draw_Vector( 0,5,0,3 );
	Otk_Draw_Vector( 0,3,2,1 );
	Otk_Draw_Vector( 2,1,7,1 );
	Otk_Draw_Vector( 7,1,9,3 );
	Otk_Draw_Vector( 9,3,9,5 );
	Otk_Draw_Vector( 9,5,7,7 );
	Otk_Draw_Vector( 7,7,2,7 );
	break;
   case 80:	/*P*/
	Otk_Draw_Vector( 9,1,0,1 );
	Otk_Draw_Vector( 0,1,0,6 );
	Otk_Draw_Vector( 0,6,1,7 );
	Otk_Draw_Vector( 1,7,3.5,7 );
	Otk_Draw_Vector( 3.5,7,5,5 );
	Otk_Draw_Vector( 5,5,5,1 );
	break;
   case 81:	/*Q*/
	Otk_Draw_Vector( 2,7,0,5 );
	Otk_Draw_Vector( 0,5,0,3 );
	Otk_Draw_Vector( 0,3,2,1 );
	Otk_Draw_Vector( 2,1,7,1 );
	Otk_Draw_Vector( 7,1,9,3 );
	Otk_Draw_Vector( 9,3,9,5 );
	Otk_Draw_Vector( 9,5,7,7 );
	Otk_Draw_Vector( 7,7,2,7 );
	Otk_Draw_Vector( 7,5,9,7 );
	break;
   case 82:	/*R*/
        Otk_Draw_Vector( 9,1,0,1 );
        Otk_Draw_Vector( 0,1,0,6 );
        Otk_Draw_Vector( 0,6,1,7 );
        Otk_Draw_Vector( 1,7,3.5,7 );
        Otk_Draw_Vector( 3.5,7,5,5 );
        Otk_Draw_Vector( 5,5,5,1 );
	Otk_Draw_Vector( 5,4.5,9,7 );
	break;
   case 83:	/*S*/
	Otk_Draw_Vector( 1,7,0,6 );
	Otk_Draw_Vector( 0,6,0,3 );
	Otk_Draw_Vector( 0,3,2,1 );
	Otk_Draw_Vector( 2,1,3,1 );
	Otk_Draw_Vector( 3,1,4,2 );
	Otk_Draw_Vector( 4,2,4,6 );
	Otk_Draw_Vector( 4,6,5,7 );
	Otk_Draw_Vector( 5,7,8,7 );
	Otk_Draw_Vector( 8,7,9,6 );
	Otk_Draw_Vector( 9,6,9,3 );
	Otk_Draw_Vector( 9,3,7,1 );
	break;
   case 84:	/*T*/
	Otk_Draw_Vector( 0,0,0,9 );
	Otk_Draw_Vector( 0,4,9,4 );
	break;
   case 85:	/*U*/
	Otk_Draw_Vector( 0,1,7,1 );
	Otk_Draw_Vector( 7,1,9,3 );
	Otk_Draw_Vector( 9,3,9,5 );
	Otk_Draw_Vector( 9,5,7,7 );
	Otk_Draw_Vector( 7,7,0,7 );
	break;
   case 86:	/*V*/
	Otk_Draw_Vector( 0,1,9,4 );
	Otk_Draw_Vector( 9,4,0,7 );
	break;
   case 87:	/*W*/
	Otk_Draw_Vector( 0,1,9,2 );
	Otk_Draw_Vector( 9,2,4,4 );
	Otk_Draw_Vector( 4,4,9,6 );
	Otk_Draw_Vector( 9,6,0,7 );
	break;
   case 88:	/*X*/
	Otk_Draw_Vector( 0,1,9,7 );
	Otk_Draw_Vector( 0,7,9,1 );
	break;
   case 89:	/*Y*/
	Otk_Draw_Vector( 0,1,4,4 );
	Otk_Draw_Vector( 4,4,9,4 );
	Otk_Draw_Vector( 4,4,0,7 );
	break;
   case 90:	/*Z*/
	Otk_Draw_Vector( 0,1,0,7 );
	Otk_Draw_Vector( 0,7,9,1 );
	Otk_Draw_Vector( 9,1,9,7 );
	break;
   case 91:	/*[*/
	Otk_Draw_Vector( 0,5,0,3 );
	Otk_Draw_Vector( 0,3,9,3 );
	Otk_Draw_Vector( 9,3,9,5 );
	break;
   case 92:	/* \ */
	Otk_Draw_Vector( 9, 7, 0, 1  );
	break;
   case 93:	/*]*/
	Otk_Draw_Vector( 0,3,0,5 );
	Otk_Draw_Vector( 0,5,9,5 );
	Otk_Draw_Vector( 9,5,9,3 );
	break;
   case 94:	/*^*/
	Otk_Draw_Vector( 5,1,0,4 );
	Otk_Draw_Vector( 0,4,5.5,7 );
	break;
   case 95:	/*_*/
	Otk_Draw_Vector( 9,1,9,7 );
	break;
   case 96:	/*`*/
	Otk_Draw_Vector( 2, 6, 0, 4 );
	break;
   case 97:	/*a*/
	Otk_Draw_Vector( 4,3,4,6 );
	Otk_Draw_Vector( 4,6,5,7 );
	Otk_Draw_Vector( 5,7,9,7 );
	Otk_Draw_Vector( 7,7,6,5 );
	Otk_Draw_Vector( 6,5,6,3 );
	Otk_Draw_Vector( 6,3,7,2 );
	Otk_Draw_Vector( 7,2,8,2 );
	Otk_Draw_Vector( 8,2,9,3 );
	Otk_Draw_Vector( 9,3,9,5 );
	Otk_Draw_Vector( 9,5,8,7 );
	break;
   case 98:	/*b*/
	Otk_Draw_Vector( 0,1,9,1 );
	Otk_Draw_Vector( 9,1,9,5 );
	Otk_Draw_Vector( 9,5,7,7 );
	Otk_Draw_Vector( 7,7,6,7 );
	Otk_Draw_Vector( 6,7,4,5 );
	Otk_Draw_Vector( 4,5,4,3 );
	Otk_Draw_Vector( 4,3,5.5,1 );
	break;
   case 99:	/*c*/
	Otk_Draw_Vector( 5,7,4,6 );
	Otk_Draw_Vector( 4,6,4,3 );
	Otk_Draw_Vector( 4,3,6,1 );
	Otk_Draw_Vector( 6,1,7,1 );
	Otk_Draw_Vector( 7,1,9,3 );
	Otk_Draw_Vector( 9,3,9,6 );
	Otk_Draw_Vector( 9,6,8,7 );
	break;
   case 100:	/*d*/
	Otk_Draw_Vector( 5,7,4,6 );
	Otk_Draw_Vector( 4,6,4,3 );
	Otk_Draw_Vector( 4,3,6,1 );
	Otk_Draw_Vector( 6,1,7,1 );
	Otk_Draw_Vector( 7,1,9,3 );
	Otk_Draw_Vector( 9,3,9,6 );
	Otk_Draw_Vector( 9,6,8,7 );
	Otk_Draw_Vector( 9,7,0,7 );
	break;
   case 101:	/*e*/
	Otk_Draw_Vector( 6,7,4,6 );
	Otk_Draw_Vector( 4,6,4,3 );
	Otk_Draw_Vector( 4,3,6,1 );
	Otk_Draw_Vector( 6,1,7,1 );
	Otk_Draw_Vector( 7,1,9,3 );
	Otk_Draw_Vector( 9,3,9,6 );
	Otk_Draw_Vector( 9,6,8,7 );
	Otk_Draw_Vector( 6,1,6,7 );
	break;
   case 102:	/*f*/
	Otk_Draw_Vector( 2,7,0,5 );
	Otk_Draw_Vector( 0,5,0,4 );
	Otk_Draw_Vector( 0,4,2,2 );
	Otk_Draw_Vector( 2,2,9,2 );
	Otk_Draw_Vector( 4,1,4,4 );
	break;
   case 103:	/*g*/
	Otk_Draw_Vector( 5,7,4,6 );
	Otk_Draw_Vector( 4,6,4,3 );
	Otk_Draw_Vector( 4,3,6,1 );
	Otk_Draw_Vector( 6,1,7,1 );
	Otk_Draw_Vector( 7,1,9,3 );
	Otk_Draw_Vector( 9,3,9,6 );
	Otk_Draw_Vector( 9,6,8,7 );
	Otk_Draw_Vector( 5,7,11,7 );
	Otk_Draw_Vector( 11,7,12,6 );
	Otk_Draw_Vector( 12,6,12,3 );
	break;
   case 104:	/*h*/
	Otk_Draw_Vector( 0,1,9,1 );
	Otk_Draw_Vector( 6,1,4,4 );
	Otk_Draw_Vector( 4,4,4,5 );
	Otk_Draw_Vector( 4,5,6,7 );
	Otk_Draw_Vector( 6,7,9,7 );
	break;
   case 105:	/*i*/
	Otk_Draw_Vector( 1.5,4,2.5,4 );
	Otk_Draw_Vector( 4,3,4,4 );
	Otk_Draw_Vector( 4,4,9,4 );
	Otk_Draw_Vector( 9,3,9,5 );
	break;
   case 106:	/*j*/
	Otk_Draw_Vector( 1.5,5,2.5,5 );
	Otk_Draw_Vector( 4,4,4,5 );
	Otk_Draw_Vector( 4,5,10,5 );
	Otk_Draw_Vector( 10,5,12,4 );
	Otk_Draw_Vector( 12,4,12,3 );
	Otk_Draw_Vector( 12,3,9,1.5 );
	break;
   case 107:	/*k*/
	Otk_Draw_Vector( 0,1,9,1 );
	Otk_Draw_Vector( 2,7,4.5,1 );
	Otk_Draw_Vector( 4.5,1,9,7 );
	break;
   case 108:	/*l*/
	Otk_Draw_Vector( 0,4,0,5 );
	Otk_Draw_Vector( 0,5,9,5 );
	Otk_Draw_Vector( 9,4,9,6 );
	break;
   case 109:	/*m*/
	Otk_Draw_Vector( 4,1,9,1 );
	Otk_Draw_Vector( 5.5,1,4,3 );
	Otk_Draw_Vector( 4,3,5,4 );
	Otk_Draw_Vector( 5,4,9,4 );
	Otk_Draw_Vector( 5,4,4,6 );
	Otk_Draw_Vector( 4,6,5,7 );
	Otk_Draw_Vector( 5,7,9,7 );
	break;
   case 110:	/*n*/
	Otk_Draw_Vector( 4,1, 9,1 );
	Otk_Draw_Vector( 5.5,1, 4,3 );
	Otk_Draw_Vector( 4,3, 4,5 );
	Otk_Draw_Vector( 4,5, 6,7 );
	Otk_Draw_Vector( 6,7, 9,7 );
	break;
   case 111:	/*o*/
	Otk_Draw_Vector( 4,3.8, 4,5.2 );
	Otk_Draw_Vector( 4,5.2, 5.8,7 );
	Otk_Draw_Vector( 5.8,7, 7,7.2 );
	Otk_Draw_Vector( 7.2,7, 9,5.2 );
	Otk_Draw_Vector( 9,5.2, 9,3.8 );
	Otk_Draw_Vector( 9,3.8, 7.2,2 );
	Otk_Draw_Vector( 7.2,2, 5.8,2 );
	Otk_Draw_Vector( 5.8,2, 4,3.8 );
	break;
   case 112:	/*p*/
	Otk_Draw_Vector( 4,1,12,1 );
	Otk_Draw_Vector( 5.5,1,4,3 );
	Otk_Draw_Vector( 4,3,4,5 );
	Otk_Draw_Vector( 4,5,6,7 );
	Otk_Draw_Vector( 6,7,7,7 );
	Otk_Draw_Vector( 7,7,9,5 );
	Otk_Draw_Vector( 9,5,9,3 );
	Otk_Draw_Vector( 9,3,7,1 );
	break;
   case 113:	/*q*/
	Otk_Draw_Vector( 4,2.5,4,4 );
	Otk_Draw_Vector( 4,4,6,6 );
	Otk_Draw_Vector( 6,6,12,6 );
	Otk_Draw_Vector( 12,6,12,7 );
	Otk_Draw_Vector( 7,6,9,4 );
	Otk_Draw_Vector( 9,4,9,3 );
	Otk_Draw_Vector( 9,2.5,7,1 );
	Otk_Draw_Vector( 7,1,6,1 );
	Otk_Draw_Vector( 6,1,4,2.5 );
	break;
   case 114:	/*r*/
	Otk_Draw_Vector( 4,1,9,1 );
	Otk_Draw_Vector( 6,1,4,4 );
	Otk_Draw_Vector( 4,4,4,5 );
	Otk_Draw_Vector( 4,5,6,7 );
	break;
   case 115:	/*s*/
	Otk_Draw_Vector( 5,7,4,6 );
	Otk_Draw_Vector( 4,6,4,3 );
	Otk_Draw_Vector( 4,3,5,2 );
	Otk_Draw_Vector( 5,2,5.8,2 );
	Otk_Draw_Vector( 5.8,2, 7,6 );
	Otk_Draw_Vector( 7,6, 7.5,7 );
	Otk_Draw_Vector( 7.5,7, 8,7 );
	Otk_Draw_Vector( 8,7, 9,6 );
	Otk_Draw_Vector( 9,6,9,3 );
	Otk_Draw_Vector( 9,3,8,2 );
	break;
   case 116:	/*t*/
	Otk_Draw_Vector( 0,3.5,8,3.5 );
	Otk_Draw_Vector( 2,1,2,6.5 );
	Otk_Draw_Vector( 8,3.5,9,5 );
	Otk_Draw_Vector( 9,5,9,6 );
	Otk_Draw_Vector( 9,6,8,7 );
	break;
   case 117:	/*u*/
	Otk_Draw_Vector( 4,1,7,1 );
	Otk_Draw_Vector( 7,1,9,3 );
	Otk_Draw_Vector( 9,3,9,5 );
	Otk_Draw_Vector( 9,5,8,7 );
	Otk_Draw_Vector( 4,7,9,7 );
	break;
   case 118:	/*v*/
	Otk_Draw_Vector( 4,1,9,4 );
	Otk_Draw_Vector( 9,4,4,7 );
	break;
   case 119:	/*w*/
	Otk_Draw_Vector( 4,1,9,2.5 );
	Otk_Draw_Vector( 9,2.5,5,4 );
	Otk_Draw_Vector( 5,4,9,5.5 );
	Otk_Draw_Vector( 9,5.5,4,7 );
	break;
   case 120:	/*x*/
	Otk_Draw_Vector( 4,1,9,7 );
	Otk_Draw_Vector( 4,7,9,1 );
	break;
   case 121:	/*y*/
	Otk_Draw_Vector( 4,1,7.5,4.5 );
	Otk_Draw_Vector( 4,7,12,1 );
	break;
   case 122:	/*z*/
	Otk_Draw_Vector( 4,1,4,7 );
	Otk_Draw_Vector( 4,7,9,1 );
	Otk_Draw_Vector( 9,1,9,7 );
	break;
   case 123:	/*{*/
	Otk_Draw_Vector( 0,6, 0,5 );
	Otk_Draw_Vector( 0,5, 1,4 );
	Otk_Draw_Vector( 1,4, 4,4.1 );
	Otk_Draw_Vector( 4,4.2, 4.5,3 );
	Otk_Draw_Vector( 4.5,3, 4.5,2 );
	Otk_Draw_Vector( 4.5,3, 5,4.2 );
	Otk_Draw_Vector( 5,4.1, 8,4 );
	Otk_Draw_Vector( 8,4, 9,5 );
	Otk_Draw_Vector( 9,5, 9,6 );
	break;
   case '|':  ;	/* | */
	Otk_Draw_Vector( 0, 4, 9, 4 );
	break;
   case 125:	/*}*/
	Otk_Draw_Vector( 0,2, 0,3 );
	Otk_Draw_Vector( 0,3, 1,4 );
	Otk_Draw_Vector( 1,4, 4,3.9 );
	Otk_Draw_Vector( 4,3.8, 4.5,5 );
	Otk_Draw_Vector( 4.5,5, 4.5,6 );
	Otk_Draw_Vector( 4.5,5, 5,3.8 );
	Otk_Draw_Vector( 5,3.9, 8,4 );
	Otk_Draw_Vector( 8,4, 9,3 );
	Otk_Draw_Vector( 9,3, 9,2 );
	break;
   case 126:	/*~*/
	Otk_Draw_Vector( 5, 1, 3, 3 );
	Otk_Draw_Vector( 3, 3, 7, 5 );
	Otk_Draw_Vector( 7, 5, 5, 7 );
	break;
  default:
	Otk_Draw_Vector( 2,2,0,4 );
	Otk_Draw_Vector( 0,4,0,6 );
	Otk_Draw_Vector( 0,6,1,7 );
	Otk_Draw_Vector( 1,7,3,7 );
	Otk_Draw_Vector( 3,7,4,6 );
	Otk_Draw_Vector( 4,6,4,5 );
	Otk_Draw_Vector( 4,5,5,4 );
	Otk_Draw_Vector( 5,4,6,4 );
	Otk_Draw_Vector( 8,4,9,4 );
  }
}





#ifndef UTIL_H
#define UTIL_H

//#if (PLATFORM_KIND != Platform_Posix)
typedef unsigned int unsint;
//#endif

#ifndef c_new
#define c_new(t,n) ((t *)malloc( sizeof(t)*(n) ))
#define c_new0(t,n) ((t *)calloc( (n), sizeof(t) ))
#define c_dup(p,t,n) ((t *)memcpy( malloc( sizeof(t)*(n) ), (p), sizeof(t)*(n) ))
#define c_strdup(s) strcpy( malloc( strlen(s) ), (s) )
#endif

#define C_STRUCT_OFFSET(struct_type, member)	\
    ((long) ((unsigned char *) &((struct_type*) 0)->member))
#define C_STRUCT_MEMBER_P(struct_p, struct_offset)   \
    ((void *) ((unsigned char *) (struct_p) + (long) (struct_offset)))
#define C_STRUCT_MEMBER(member_type, struct_p, struct_offset)   \
    (*(member_type*) C_STRUCT_MEMBER_P ((struct_p), (struct_offset)))

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef CLAMP
#define CLAMP(a,b,c) MAX((a),MIN((b),(c)))
#endif
#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#endif // UTIL_H



void Otk_Read_SVG_start( char *name, char *attrs[], void *ud )
 {
  OtkFont *font = (OtkFont *)ud;
  char *colon;
  int i;

  if( (colon = strchr( name, ':' )) )
   {
    // xmlns = name;
    name = colon + 1;
   }

  if( !strcmp( name, "svg" ) );
  else if( !strcmp( name, "metadata" ) );
  else if( !strcmp( name, "defs" ) );
  else if( !strcmp( name, "font" ) );
  else if( !strcmp( name, "font-face" ) )
   {
    for( i = 0; attrs[i]; i++ )
     {
      if( !strcmp( "font-family=", attrs[i] ) && attrs[i + 1] )
	font->family = strdup( attrs[i + 1] );
      else if( !strcmp( "units-per-em=", attrs[i] ) && attrs[i + 1] )
	font->units_per_em = strtod( attrs[i + 1], NULL );
      else if( !strcmp( "font-weight=", attrs[i] ) && attrs[i + 1] )
	font->weight = strtod( attrs[i + 1], NULL );
      else if( !strcmp( "bbox=", attrs[i] ) && attrs[i + 1] )
	sscanf( attrs[i + 1], "%f %f %f %f", font->bbox, font->bbox + 1,
		font->bbox + 2, font->bbox + 3 );
      else if( !strcmp( "ascent=", attrs[i] ) && attrs[i + 1] )
	font->ascent = strtod( attrs[i + 1], NULL );
      else if( !strcmp( "descent=", attrs[i] ) && attrs[i + 1] )
	font->descent = strtod( attrs[i + 1], NULL );
      else if( !strcmp( "x-height=", attrs[i] ) && attrs[i + 1] )
	font->x_height = strtod( attrs[i + 1], NULL );
      else if( !strcmp( "underline-thickness=", attrs[i] ) && attrs[i + 1] )
	font->underline_thickness = strtod( attrs[i + 1], NULL );
      else if( !strcmp( "underline-position=", attrs[i] ) && attrs[i + 1] )
	font->underline_position = strtod( attrs[i + 1], NULL );
      else if( !strcmp( "unicode-range=", attrs[i] ) && attrs[i + 1] )
	sscanf( attrs[i + 1], "U+%x-U+%x", &font->start_glyph, &font->end_glyph );

      if( attrs[i + 1] )
	++i;
     }

    if( font->units_per_em > 0 )
      font->units_per_em = 2000.0;

    font->weight = font->weight/font->units_per_em;
    font->bbox[0] = font->bbox[0]/font->units_per_em;
    font->bbox[1] = font->bbox[1]/font->units_per_em;
    font->bbox[2] = font->bbox[2]/font->units_per_em;
    font->bbox[3] = font->bbox[3]/font->units_per_em;
    font->ascent = font->ascent/font->units_per_em;
    font->descent = font->descent/font->units_per_em;
    font->x_height = font->x_height/font->units_per_em;
    font->underline_thickness = font->underline_thickness/font->units_per_em;
    font->underline_position = font->underline_position/font->units_per_em;

    font->start_glyph = 0; /* fixme: must be zero at the moment because unsigned truncation */
    if( font->end_glyph > 255 )
      font->end_glyph = 255; /* I'm American */
    font->glyph_defs = (OtkGlyph **)calloc( font->end_glyph - font->start_glyph + 1,
					    sizeof(OtkGlyph *) );
   }
  else if( !strcmp( name, "missing-glyph" ) )
   {
     
   }
  else if( !strcmp( name, "glyph" ) )
   {
    int code = -1;

    for( i = 0; attrs[i]; i++ )
     {
      if( !strcmp( "unicode=", attrs[i] ) && attrs[i + 1] )
       {
	if ( !attrs[i + 1][0] || (strlen( attrs[i + 1] ) > 1  && attrs[i+1][0] != '\\'))
	 {
	  printf( "unicode \"%s\" is unamerican\n", attrs[i + 1] );
	  return; /* I'm American */
	 }
	code = attrs[i + 1][0];
	//printf( "unicode=\"%c\"\n", code );
       }

      if( attrs[i + 1] )
	++i;
     }

    if( code < 0 ) /* don't know what char to map this glyph to */
     {
      printf( "don't know what char to map unicode=\"%c\" to\n", code );
      return;
     }

    //printf( "mapping code \"%c\"\n", code );

    font->glyph_defs[code] = (OtkGlyph *)calloc( 1, sizeof(OtkGlyph) );
    for( i = 0; attrs[i]; i++ )
     {
      if( !strcmp( "glyph-name=", attrs[i] ) && attrs[i + 1] );
      else if( !strcmp( "horiz-adv-x=", attrs[i] ) && attrs[i + 1] )
	font->glyph_defs[code]->x_adv = strtod( attrs[i + 1], NULL )/font->units_per_em;
      else if( !strcmp( "orientation=", attrs[i] ) && attrs[i + 1] )
	font->glyph_defs[code]->orientation = attrs[i + 1][0];
      else if( !strcmp( "d=", attrs[i] ) && attrs[i + 1] )
	font->glyph_defs[code]->path = strdup( attrs[i + 1] );

      if( attrs[i + 1] )
	++i;
     }
  } else if( !strcmp( name, "hkern" ) ) {
  } else if( !strcmp( name, "vkern" ) ) {
  }
}


char *otk_memspool( char *spool, unsigned int *len, const void *data, const size_t n )
{
  int newlen, modlen;

  if (!spool) *len = 0;
  if (!*len) spool = NULL;
  newlen = *len + n;
  modlen = (newlen + (1 << 6)) >> 6;
  if( !*len || modlen > ((*len + (1 << 6)) >> 6) )
   {
    if (spool) spool = realloc( spool, modlen << 6 );
    else spool = malloc( modlen << 6 );
   }
  memcpy( spool + *len, data, n );
  *len = newlen;
  return( spool );
}



GLfloat *Otk_spool_3f( GLfloat *spool, unsint *len, GLfloat x, GLfloat y, GLfloat z )
{
  *len *= sizeof(GLfloat);
  spool = (GLfloat *)otk_memspool( (char *)spool, len, &x, sizeof(GLfloat) );
  spool = (GLfloat *)otk_memspool( (char *)spool, len, &y, sizeof(GLfloat) );
  spool = (GLfloat *)otk_memspool( (char *)spool, len, &z, sizeof(GLfloat) );
  *len /= sizeof(GLfloat);
  return( spool );
}


GLfloat *Otk_spool_3fv( GLfloat *spool, unsint *len, GLfloat *xyz )
{
  *len *= sizeof(GLfloat);
  spool = (GLfloat *)otk_memspool( (char *)spool, len, xyz, 3*sizeof(GLfloat) );
  *len /= sizeof(GLfloat);
  return( spool );
}


#ifndef CALLBACK
#define CALLBACK
#endif


void CALLBACK combine_callback( GLdouble coords[3], 
				GLdouble *vertex_data[4],
				GLfloat weight[4], GLdouble **dataOut )
{
   GLdouble *vertex;

   vertex = (GLdouble *)malloc( 3*sizeof(GLdouble) );
   vertex[0] = coords[0];
   vertex[1] = coords[1];
   vertex[2] = coords[2];
   *dataOut = vertex;
}


void CALLBACK end_callback()
{
  glEnd();
}


void CALLBACK begin_callback( GLenum which )
{
  glBegin( which );
}


void CALLBACK vertex_callback( GLdouble *vrt )
{
  glVertex3dv( vrt );
}


void CALLBACK edge_callback( GLboolean flag )
{
  glEdgeFlag( flag );
}


void Otk_gl_from_SVG_path( OtkFont *font, char *path, char chr )
{
 char *endptr, mode='h';
 int i, j, k, Nj, Nk;
 GLdouble pts[4][3] = {{0.0,0.0,0.0},{0.0,0.0,0.0},{0.0,0.0,0.0},{0.0,0.0,0.0}};
 GLdouble curpt[3] = {0.0, 0.0, 0.0};
 GLdouble curctlpt[3] = {0.0, 0.0, 0.0};
 GLdouble vrts[512][3];
 int v0 = 0, vN, num_vrts = 0, outline = 1;
 static GLUtesselator *fnttess = NULL;

 if( !fnttess ) {
   fnttess = gluNewTess();
   gluTessCallback( fnttess, GLU_TESS_VERTEX, (_GLUfuncptr)vertex_callback );
   gluTessCallback( fnttess, GLU_TESS_BEGIN, (_GLUfuncptr)begin_callback );
   gluTessCallback( fnttess, GLU_TESS_END, (_GLUfuncptr)end_callback );
   gluTessCallback( fnttess, GLU_TESS_EDGE_FLAG, (_GLUfuncptr)edge_callback );
   gluTessCallback( fnttess, GLU_TESS_COMBINE, (_GLUfuncptr)combine_callback );
   gluTessProperty( fnttess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD );
   gluTessProperty( fnttess, GLU_TESS_TOLERANCE, 1e-6 );
   //gluTessProperty( fnttess, GLU_TESS_BOUNDARY_ONLY, GL_TRUE );
 }

 gluTessBeginPolygon( fnttess, NULL );
 gluTessNormal( fnttess, 0.0, 0.0, 1.0 );
 j = 0;
 k = 0;
 Nj = 0;
 Nk = 0;
 while( *path )
  {
   if( isdigit( *path ) || *path == '-' || *path == '+' )
    {
     pts[k][j] = strtod( path, &endptr )/font->units_per_em;
     if( num_vrts && islower( mode ) )
      {
       if( mode == 'v' )
	 pts[k][j] += curpt[1];
       else
	 pts[k][j] += curpt[j];
      }
     path = endptr;
     j++;
     if( j >= Nj )
      {
       k++;
       j = 0;
      }
    }
   else if( isspace( *path ) )
    {
     ++path;
    }
   else
    {
     //if( chr == ':' )
     //printf( "changing mode to '%c'\n", *path );
     mode = *path;
     switch( tolower( mode ) )
      {
        case 'm': 
	case 'l': case 't':	Nj = 2;  Nk = 1;  break;
      	case 'h': case 'v':	Nj = 1;  Nk = 1;  break;
      	case 's': case 'q':	Nj = 2;  Nk = 2;  break;
      	case 'c':		Nj = 2;  Nk = 3;  break;
      	case 'z':	
      	default:		Nj = 0;	 Nk = 0;  break;
      }
     j = 0;
     k = 0;
     path++;
    }
   if( k >= Nk )
    {
     int pti = 0; // new point index

     switch( tolower( mode ) )
      {
      case 'm':
       if( num_vrts > 1 ) // dump last line if there was one
	{
	 vN = v0 + num_vrts;
	 glBegin( GL_LINE_STRIP );
	 for( i = v0; i < vN; i++ )
	   glVertex3d( vrts[i][0], vrts[i][1], 0.0 );
	 glEnd();
	}
       v0 += num_vrts;
       num_vrts = 0;
       vrts[v0+num_vrts][0] = pts[0][0];
       vrts[v0+num_vrts++][1] = pts[0][1];
       //if( chr == ':' )
       //printf( "moveto (%g, %g)\n", pts[0][0], pts[0][1] );
       break;
      case 'z':
       if( num_vrts )
	{
	 pts[0][0] = vrts[v0][0];
	 pts[0][1] = vrts[v0][1];

	 //if( chr == ':' )
	 //printf( "closepath (%g, %g)\n", pts[0][0], pts[0][1] );

	 if( outline )
	   glBegin( GL_LINE_STRIP );
	 gluTessBeginContour( fnttess );
	 vN = v0 + num_vrts;
	 for( i = v0; i < vN; i++ )
	  {
	   vrts[i][2] = 0.0;
	   gluTessVertex( fnttess, vrts[i], vrts[i] );
	   if( outline )
	     glVertex3dv( vrts[i] );
	  }
	 if( outline )
	   glEnd();
	 gluTessEndContour( fnttess );
	 v0 += num_vrts;
	 num_vrts = 0;
	 vrts[v0+num_vrts][0] = pts[0][0];
	 vrts[v0+num_vrts++][1] = pts[0][1];
	}
       break;
      case 'l':
       if( num_vrts )
	{
	 vrts[v0+num_vrts][0] = pts[0][0];
	 vrts[v0+num_vrts++][1] = pts[0][1];
	 //printf( "lineto (%g, %g)\n", pts[0][0], pts[0][1] );
	}
       break;
      case 'h':
       if( num_vrts )
	{
	 pts[0][1] = curpt[1];
	 //if( chr == ':' )
	 //printf( "horiz lineto (%g, %g)\n", pts[0][0], pts[0][1] );
	 vrts[v0+num_vrts][0] = pts[0][0];
	 vrts[v0+num_vrts++][1] = pts[0][1];
	}
       break;
      case 'v':
       if( num_vrts )
	{
	 pts[0][1] = pts[0][0];
	 pts[0][0] = curpt[0];
	 //if( chr == ':' )
	 //printf( "vert lineto (%g, %g)\n", pts[0][0], pts[0][1] );
	 vrts[v0+num_vrts][0] = pts[0][0];
	 vrts[v0+num_vrts++][1] = pts[0][1];
	}
       break;
      case 'c':
       if( num_vrts )
	{
	 vrts[v0+num_vrts][0] =   (curpt[0]*(0.667*0.667) +
				pts[0][0]*(3.0*0.333*0.667*0.667) +
				pts[1][0]*(3.0*0.333*0.333*0.667) +
				pts[2][0]*(0.333*0.333));
	 vrts[v0+num_vrts++][1] = (curpt[1]*(0.667*0.667) +
				pts[0][1]*(3.0*0.333*0.667*0.667) +
				pts[1][1]*(3.0*0.333*0.333*0.667) +
				pts[2][1]*(0.333*0.333));
	 vrts[v0+num_vrts][0] =   (curpt[0]*(0.333*0.333) +
				pts[0][0]*(3.0*0.333*0.333*0.667) +
				pts[1][0]*(3.0*0.333*0.667*0.667) +
				pts[2][0]*(0.667*0.667));
	 vrts[v0+num_vrts++][1] = (curpt[1]*(0.333*0.333) +
				pts[0][1]*(3.0*0.333*0.333*0.667) +
				pts[1][1]*(3.0*0.333*0.667*0.667) +
				pts[2][1]*(0.667*0.667));
	 vrts[v0+num_vrts][0] = pts[2][0];
	 vrts[v0+num_vrts++][1] = pts[2][1];
	 pti = 2;
	 curctlpt[0] = 2.0*pts[2][0] - pts[1][0];
	 curctlpt[1] = 2.0*pts[2][1] - pts[1][1];
	 //if( chr == ':' )
	 //printf( "cubic bez (%g, %g) ctl (%g, %g) (%g, %g)\n",
	 //	 pts[2][0], pts[2][1], pts[0][0], pts[0][1], pts[1][0], pts[1][1] );
	}
       break;
      case 's':
       if( num_vrts )
	{
	 vrts[v0+num_vrts][0] =   (curpt[0]*(0.667*0.667) +
				curctlpt[0]*(3.0*0.333*0.667*0.667) +
				pts[0][0]*(3.0*0.333*0.333*0.667) +
				pts[1][0]*(0.333*0.333));
	 vrts[v0+num_vrts++][1] = (curpt[1]*(0.667*0.667) +
				curctlpt[1]*(3.0*0.333*0.667*0.667) +
				pts[0][1]*(3.0*0.333*0.333*0.667) +
				pts[1][1]*(0.333*0.333));
	 vrts[v0+num_vrts][0] =   (curpt[0]*(0.333*0.333) +
				curctlpt[0]*(3.0*0.333*0.333*0.667) +
				pts[0][0]*(3.0*0.333*0.667*0.667) +
				pts[1][0]*(0.667*0.667));
	 vrts[v0+num_vrts++][1] = (curpt[1]*(0.333*0.333) +
				curctlpt[1]*(3.0*0.333*0.333*0.667) +
				pts[0][1]*(3.0*0.333*0.667*0.667) +
				pts[1][1]*(0.667*0.667));
	 vrts[v0+num_vrts][0] = pts[1][0];
	 vrts[v0+num_vrts++][1] = pts[1][1];
	 pti = 1;
	 curctlpt[0] = 2.0*pts[1][0] - pts[0][0];
	 curctlpt[1] = 2.0*pts[1][1] - pts[0][1];
	 //printf( "cubic bez (%g, %g) ctl (%g, %g) (%g, %g)\n",
	 //	 pts[1][0], pts[1][1], curctlpt[0], curctlpt[1], pts[0][0], pts[0][1] );
	}
       break;
      case 'q':
       if( num_vrts )
	{
	 vrts[v0+num_vrts][0] =   (curpt[0]*(0.667*0.667) +
				pts[0][0]*(2.0*0.333*0.667) +
				pts[1][0]*(0.333*0.333));
	 vrts[v0+num_vrts++][1] = (curpt[1]*(0.667*0.667) +
				pts[0][1]*(2.0*0.333*0.667) +
				pts[1][1]*(0.333*0.333));
	 vrts[v0+num_vrts][0] =   (curpt[0]*(0.333*0.333) +
				pts[0][0]*(2.0*0.333*0.667) +
				pts[1][0]*(0.667*0.667));
	 vrts[v0+num_vrts++][1] = (curpt[1]*(0.333*0.333) +
				pts[0][1]*(2.0*0.333*0.667) +
				pts[1][1]*(0.667*0.667));
	 vrts[v0+num_vrts][0] = pts[1][0];
	 vrts[v0+num_vrts++][1] = pts[1][1];
	 pti = 1;
	 curctlpt[0] = 2.0*pts[1][0] - pts[0][0];
	 curctlpt[1] = 2.0*pts[1][1] - pts[0][1];
	 //if( chr == ':' )
	 //printf( "quad bez (%g, %g) ctl (%g, %g)\n", pts[1][0], pts[1][1],
	 //	 pts[0][0], pts[0][1] );
	}
       break;
      case 't':
       if( num_vrts )
	{
	 //if( chr == ':' )
	 //printf( "quad bez (%g, %g) ctl (%g, %g)\n", pts[0][0], pts[0][1],
	 //	 curctlpt[0], curctlpt[1] );

	 vrts[v0+num_vrts][0] =   (curpt[0]*(0.667*0.667) +
				curctlpt[0]*(2.0*0.333*0.667) +
				pts[0][0]*(0.333*0.333));
	 vrts[v0+num_vrts++][1] = (curpt[1]*(0.667*0.667) +
				curctlpt[1]*(2.0*0.333*0.667) +
				pts[0][1]*(0.333*0.333));
	 vrts[v0+num_vrts][0] =   (curpt[0]*(0.333*0.333) +
				curctlpt[0]*(2.0*0.333*0.667) +
				pts[0][0]*(0.667*0.667));
	 vrts[v0+num_vrts++][1] = (curpt[1]*(0.333*0.333) +
				curctlpt[1]*(2.0*0.333*0.667) +
				pts[0][1]*(0.667*0.667));
	 vrts[v0+num_vrts][0] = pts[0][0];
	 vrts[v0+num_vrts++][1] = pts[0][1];
	 curctlpt[0] = 2.0*pts[0][0] - curctlpt[0];
	 curctlpt[1] = 2.0*pts[0][1] - curctlpt[1];
	}
       break;
      default:
       break;
      }
     curpt[0] = pts[pti][0];
     curpt[1] = pts[pti][1];
     j = 0;
     k = 0;
    }
  }
 gluTessEndPolygon( fnttess );
}




OtkFont *Otk_Default_Font = NULL;	/* FONTS insert */
OtkFont *Otk_Vect_Font = NULL;		/* FONTS insert */
OtkFont *Otk_Primary_Font = NULL;	/* FONTS insert */






OtkFont *Otk_CreateBasicVectorFont()
{
 OtkFont *font = calloc( 1, sizeof(OtkFont) );
 int num_glyphs;
 int i;

 font->ascent = 1.0;
 font->descent = -0.5;
 font->start_glyph = 0; /* must be zero at the moment because unsigned truncation */
 font->end_glyph = '~';
 num_glyphs = font->end_glyph - font->start_glyph + 1; /* 126 - 32 + 1 = 95 */

 font->glyphs = glGenLists( num_glyphs+1 );
 for( i = 0; i <= num_glyphs; i++ )
  {
   font->metrics = NULL; 	/* no metrics available */
   glNewList( font->glyphs + i, GL_COMPILE );
   glBegin( GL_LINES );
   Otk_Letter2Draw_Vector( 0.0, font->start_glyph + i, 1.0/(0.15*9.5), 1.0/(0.13*13.0), 0.0, 0.0, 0.0, 0.0 );
   glEnd();
   glTranslatef( 1.0, 0.0, 0.0 );
   glEndList();
  }

 return( font );
}


char *Otk_HelvFont_Paths[] =
 {
	"",
	"M231 364l-55 777v325h223v-325l-52 -777h-116zM184 0v205h207v-205h-207z",
	"M144 947l-50 279v240h205v-240l-45 -279h-110zM475 947l-49 279v240h205v-240l-48 -279h-108z",
	"M103 -25l87 426h-169v149h199l74 363h-273v149h303l87 429h150l-87 -429h315l87 429h151l-87 -429h173v-149h-203l-75 -363h278v-149h-308l-87 -426h-150l86 426h-314l-87 -426h-150zM370 550h314l75 363h-315z",
	"M510 -211v180q-135 17 -219.5 60.5t-146 140.5t-71.5 237l181 34q21 -145 74 -213q76 -96 182 -107v573q-111 21 -227 86q-86 48 -132.5 133t-46.5 193q0 192 136 311q91 80 270 98v86h106v-86q157 -15 249 -92q118 -98 142 -269l-186 -28q-16 106 -66.5 162.5 t-138.5 74.5v-519q136 -34 180 -53q84 -37 137 -90t81.5 -126t28.5 -158q0 -187 -119 -312t-308 -134v-182h-106zM510 1365q-105 -16 -165.5 -84t-60.5 -161q0 -92 51.5 -154t174.5 -99v498zM616 121q105 13 173.5 91t68.5 193q0 98 -48.5 157.5t-193.5 106.5v-548z",
	"M119 1114q0 157 79 267t229 110q138 0 228.5 -98.5t90.5 -289.5q0 -186 -91.5 -286.5t-225.5 -100.5q-133 0 -221.5 99t-88.5 299zM432 1367q-67 0 -111.5 -58t-44.5 -213q0 -141 45 -198.5t111 -57.5q68 0 112.5 58t44.5 212q0 142 -45 199.5t-112 57.5zM433 -54 l802 1545h146l-799 -1545h-149zM1067 344q0 158 79 267.5t230 109.5q138 0 228.5 -98.5t90.5 -289.5q0 -186 -91.5 -286.5t-226.5 -100.5q-133 0 -221.5 99.5t-88.5 298.5zM1381 597q-68 0 -112.5 -58t-44.5 -213q0 -140 45 -198t111 -58q69 0 113.5 58t44.5 212 q0 142 -45 199.5t-112 57.5z",
	"M973 173q-89 -99 -194 -148.5t-227 -49.5q-225 0 -357 152q-107 124 -107 277q0 136 87.5 245.5t261.5 192.5q-99 114 -132 185t-33 137q0 132 103.5 229.5t260.5 97.5q150 0 245.5 -92t95.5 -221q0 -209 -277 -357l263 -335q45 88 70 204l187 -40q-48 -192 -130 -316 q101 -134 229 -225l-121 -143q-109 70 -225 207zM607 937q117 69 151.5 121t34.5 115q0 75 -47.5 122.5t-118.5 47.5q-73 0 -121.5 -47t-48.5 -115q0 -34 17.5 -71.5t52.5 -79.5zM860 315l-330 409q-146 -87 -197 -161.5t-51 -147.5q0 -89 71 -185t201 -96q81 0 167.5 50.5 t138.5 130.5z",
	"M136 947l-46 274v245h205v-245l-48 -274h-111z",
	"M479 -431q-149 188 -252 440t-103 522q0 238 77 456q90 253 278 504h129q-121 -208 -160 -297q-61 -138 -96 -288q-43 -187 -43 -376q0 -481 299 -961h-129z",
	"M253 -431h-129q299 480 299 961q0 188 -43 373q-34 150 -95 288q-39 90 -161 300h129q188 -251 278 -504q77 -218 77 -456q0 -270 -103.5 -522t-251.5 -440z",
	"M64 1197l46 142q159 -56 231 -97q-19 181 -20 249h145q-3 -99 -23 -248q103 52 236 96l46 -142q-127 -42 -249 -56q61 -53 172 -189l-120 -85q-58 79 -137 215q-74 -141 -130 -215l-118 85q116 143 166 189q-129 25 -245 56z",
	"M513 237v402h-399v168h399v399h170v-399h399v-168h-399v-402h-170z",
	"M182 0v205h205v-205q0 -113 -40 -182.5t-127 -107.5l-50 77q57 25 84 73.5t30 139.5h-102z",
	"M65 440v181h553v-181h-553z",
	"M186 0v205h205v-205h-205z",
	"M0 -25l425 1516h144l-424 -1516h-145z",
	"M85 723q0 260 53.5 418.5t159 244.5t265.5 86q118 0 207 -47.5t147 -137t91 -218t33 -346.5q0 -258 -53 -416.5t-158.5 -245t-266.5 -86.5q-212 0 -333 152q-145 183 -145 596zM270 723q0 -361 84.5 -480.5t208.5 -119.5t208.5 120t84.5 480q0 362 -84.5 481t-210.5 119 q-124 0 -198 -105q-93 -134 -93 -495z",
	"M763 0h-180v1147q-65 -62 -170.5 -124t-189.5 -93v174q151 71 264 172t160 196h116v-1472z",
	"M1031 173v-173h-969q-2 65 21 125q37 99 118.5 195t235.5 222q239 196 323 310.5t84 216.5q0 107 -76.5 180.5t-199.5 73.5q-130 0 -208 -78t-79 -216l-185 19q19 207 143 315.5t333 108.5q211 0 334 -117t123 -290q0 -88 -36 -173t-119.5 -179t-277.5 -258 q-162 -136 -208 -184.5t-76 -97.5h719z",
	"M86 387l180 24q31 -153 105.5 -220.5t181.5 -67.5q127 0 214.5 88t87.5 218q0 124 -81 204.5t-206 80.5q-51 0 -127 -20l20 158q18 -2 29 -2q115 0 207 60t92 185q0 99 -67 164t-173 65q-105 0 -175 -66t-90 -198l-180 32q33 181 150 280.5t291 99.5q120 0 221 -51.5 t154.5 -140.5t53.5 -189q0 -95 -51 -173t-151 -124q130 -30 202 -124.5t72 -236.5q0 -192 -140 -325.5t-354 -133.5q-193 0 -320.5 115t-145.5 298z",
	"M662 0v351h-636v165l669 950h147v-950h198v-165h-198v-351h-180zM662 516v661l-459 -661h459z",
	"M85 384l189 16q21 -138 97.5 -207.5t184.5 -69.5q130 0 220 98t90 260q0 154 -86.5 243t-226.5 89q-87 0 -157 -39.5t-110 -102.5l-169 22l142 753h729v-172h-585l-79 -394q132 92 277 92q192 0 324 -133t132 -342q0 -199 -116 -344q-141 -178 -385 -178 q-200 0 -326.5 112t-144.5 297z",
	"M1019 1107l-179 -14q-24 106 -68 154q-73 77 -180 77q-86 0 -151 -48q-85 -62 -134 -181t-51 -339q65 99 159 147t197 48q180 0 306.5 -132.5t126.5 -342.5q0 -138 -59.5 -256.5t-163.5 -181.5t-236 -63q-225 0 -367 165.5t-142 545.5q0 425 157 618q137 168 369 168 q173 0 283.5 -97t132.5 -268zM284 475q0 -93 39.5 -178t110.5 -129.5t149 -44.5q114 0 196 92t82 250q0 152 -81 239.5t-204 87.5q-122 0 -207 -87.5t-85 -229.5z",
	"M97 1274v173h949v-140q-140 -149 -277.5 -396t-212.5 -508q-54 -184 -69 -403h-185q3 173 68 418t186.5 472.5t258.5 383.5h-718z",
	"M362 795q-112 41 -166 117t-54 182q0 160 115 269t306 109q192 0 309 -111.5t117 -271.5q0 -102 -53.5 -177.5t-162.5 -116.5q135 -44 205.5 -142t70.5 -234q0 -188 -133 -316t-350 -128t-350 128.5t-133 320.5q0 143 72.5 239.5t206.5 131.5zM326 1100q0 -104 67 -170 t174 -66q104 0 170.5 65.5t66.5 160.5q0 99 -68.5 166.5t-170.5 67.5q-103 0 -171 -66t-68 -158zM268 423q0 -77 36.5 -149t108.5 -111.5t155 -39.5q129 0 213 83t84 211q0 130 -86.5 215t-216.5 85q-127 0 -210.5 -84t-83.5 -210z",
	"M112 339l173 16q22 -122 84 -177t159 -55q83 0 145.5 38t102.5 101.5t67 171.5t27 220q0 12 -1 36q-54 -86 -147.5 -139.5t-202.5 -53.5q-182 0 -308 132t-126 348q0 223 131.5 359t329.5 136q143 0 261.5 -77t180 -219.5t61.5 -412.5q0 -281 -61 -447.5t-181.5 -253.5 t-282.5 -87q-172 0 -281 95.5t-131 268.5zM849 986q0 155 -82.5 246t-198.5 91q-120 0 -209 -98t-89 -254q0 -140 84.5 -227.5t208.5 -87.5q125 0 205.5 87.5t80.5 242.5z",
	"M185 857v205h205v-205h-205zM185 0v205h205v-205h-205z",
	"M182 857v205h205v-205h-205zM182 0v205h205v-205q0 -113 -40 -182.5t-127 -107.5l-50 77q57 25 84 73.5t30 139.5h-102z",
	"M112 641v168l971 410v-179l-770 -316l770 -319v-179z",
	"M1082 862h-968v168h968v-168zM1082 417h-968v168h968v-168z",
	"M1083 641l-971 -415v179l769 319l-769 316v179l971 -410v-168z",
	"M472 361q-1 36 -1 54q0 106 30 183q22 58 71 117q36 43 129.5 125.5t121.5 131.5t28 107q0 105 -82 184.5t-201 79.5q-115 0 -192 -72t-101 -225l-185 22q25 205 148.5 314t326.5 109q215 0 343 -117t128 -283q0 -96 -45 -177t-176 -197q-88 -78 -115 -115t-40 -85 t-15 -156h-173zM461 0v205h205v-205h-205z",
	"M1161 163q-65 -75 -145.5 -120.5t-161.5 -45.5q-89 0 -173 52t-136.5 160t-52.5 237q0 159 81.5 318.5t202.5 239.5t235 80q87 0 166 -45.5t136 -138.5l34 155h179l-144 -671q-30 -140 -30 -155q0 -27 20.5 -46.5t49.5 -19.5q53 0 139 61q114 80 180.5 214.5t66.5 277.5 q0 167 -85.5 312t-255 232t-374.5 87q-234 0 -427.5 -109.5t-300 -314t-106.5 -438.5q0 -245 106.5 -422t308 -261.5t446.5 -84.5q262 0 439 88t265 214h181q-51 -105 -175 -214t-295 -172.5t-412 -63.5q-222 0 -409.5 57t-319.5 171.5t-199 263.5q-84 189 -84 408 q0 244 100 465q122 271 346.5 415t544.5 144q248 0 445.5 -101.5t311.5 -302.5q97 -173 97 -376q0 -290 -204 -515q-182 -202 -398 -202q-69 0 -111.5 21t-62.5 60q-13 25 -19 86zM677 434q0 -137 65 -213t149 -76q56 0 118 33.5t118.5 99.5t92.5 167.5t36 203.5 q0 136 -67.5 211t-164.5 75q-64 0 -120.5 -32.5t-109.5 -104.5t-85 -175t-32 -189z",
	"M-3 0l563 1466h209l600 -1466h-221l-171 444h-613l-161 -444h-206zM420 602h497l-153 406q-70 185 -104 304q-28 -141 -79 -280z",
	"M150 0v1466h550q168 0 269.5 -44.5t159 -137t57.5 -193.5q0 -94 -51 -177t-154 -134q133 -39 204.5 -133t71.5 -222q0 -103 -43.5 -191.5t-107.5 -136.5t-160.5 -72.5t-236.5 -24.5h-559zM344 850h317q129 0 185 17q74 22 111.5 73t37.5 128q0 73 -35 128.5t-100 76 t-223 20.5h-293v-443zM344 173h365q94 0 132 7q67 12 112 40t74 81.5t29 123.5q0 82 -42 142.5t-116.5 85t-214.5 24.5h-339v-504z",
	"M1204 514l194 -49q-61 -239 -219.5 -364.5t-387.5 -125.5q-237 0 -385.5 96.5t-226 279.5t-77.5 393q0 229 87.5 399.5t249 259t355.5 88.5q220 0 370 -112t209 -315l-191 -45q-51 160 -148 233t-244 73q-169 0 -282.5 -81t-159.5 -217.5t-46 -281.5q0 -187 54.5 -326.5 t169.5 -208.5t249 -69q163 0 276 94t153 279z",
	"M158 0v1466h505q171 0 261 -21q126 -29 215 -105q116 -98 173.5 -250.5t57.5 -348.5q0 -167 -39 -296t-100 -213.5t-133.5 -133t-175 -73.5t-235.5 -25h-529zM352 173h313q145 0 227.5 27t131.5 76q69 69 107.5 185.5t38.5 282.5q0 230 -75.5 353.5t-183.5 165.5 q-78 30 -251 30h-308v-1120z",
	"M162 0v1466h1060v-173h-866v-449h811v-172h-811v-499h900v-173h-1094z",
	"M168 0v1466h989v-173h-795v-454h688v-173h-688v-666h-194z",
	"M844 575v172l621 1v-544q-143 -114 -295 -171.5t-312 -57.5q-216 0 -392.5 92.5t-266.5 267.5t-90 391q0 214 89.5 399.5t257.5 275.5t387 90q159 0 287.5 -51.5t201.5 -143.5t111 -240l-175 -48q-33 112 -82 176t-140 102.5t-202 38.5q-133 0 -230 -40.5t-156.5 -106.5 t-92.5 -145q-56 -136 -56 -295q0 -196 67.5 -328t196.5 -196t274 -64q126 0 246 48.5t182 103.5v273h-431z",
	"M164 0v1466h194v-602h762v602h194v-1466h-194v691h-762v-691h-194z",
	"M191 0v1466h194v-1466h-194z",
	"M59 416l175 24q7 -168 63 -230t155 -62q73 0 126 33.5t73 91t20 183.5v1010h194v-999q0 -184 -44.5 -285t-141 -154t-226.5 -53q-193 0 -295.5 111t-98.5 330z",
	"M150 0v1466h194v-727l728 727h263l-615 -594l642 -872h-256l-522 742l-240 -234v-508h-194z",
	"M150 0v1466h194v-1293h722v-173h-916z",
	"M152 0v1466h292l347 -1038q48 -145 70 -217q25 80 78 235l351 1020h261v-1466h-187v1227l-426 -1227h-175l-424 1248v-1248h-187z",
	"M156 0v1466h199l770 -1151v1151h186v-1466h-199l-770 1152v-1152h-186z",
	"M99 714q0 365 196 571.5t506 206.5q203 0 366 -97t248.5 -270.5t85.5 -393.5q0 -223 -90 -399t-255 -266.5t-356 -90.5q-207 0 -370 100t-247 273t-84 366zM299 711q0 -265 142.5 -417.5t357.5 -152.5q219 0 360.5 154t141.5 437q0 179 -60.5 312.5t-177 207t-261.5 73.5 q-206 0 -354.5 -141.5t-148.5 -472.5z",
	"M158 0v1466h553q146 0 223 -14q108 -18 181 -68.5t117.5 -141.5t44.5 -200q0 -187 -119 -316.5t-430 -129.5h-376v-596h-194zM352 769h379q188 0 267 70t79 197q0 92 -46.5 157.5t-122.5 86.5q-49 13 -181 13h-375v-524z",
	"M1269 157q135 -93 249 -136l-57 -135q-158 57 -315 180q-163 -91 -360 -91q-199 0 -361 96t-249.5 270t-87.5 392q0 217 88 395t250.5 271t363.5 93q203 0 366 -96.5t248.5 -270t85.5 -391.5q0 -181 -55 -325.5t-166 -251.5zM842 405q168 -47 277 -140q171 156 171 469 q0 178 -60.5 311t-177 206.5t-261.5 73.5q-217 0 -360 -148.5t-143 -443.5q0 -286 141.5 -439t361.5 -153q104 0 196 39q-91 59 -192 84z",
	"M161 0v1466h650q196 0 298 -39.5t163 -139.5t61 -221q0 -156 -101 -263t-312 -136q77 -37 117 -73q85 -78 161 -195l255 -399h-244l-194 305q-85 132 -140 202t-98.5 98t-88.5 39q-33 7 -108 7h-225v-651h-194zM355 819h417q133 0 208 27.5t114 88t39 131.5 q0 104 -75.5 171t-238.5 67h-464v-485z",
	"M92 471l183 16q13 -110 60.5 -180.5t147.5 -114t225 -43.5q111 0 196 33t126.5 90.5t41.5 125.5q0 69 -40 120.5t-132 86.5q-59 23 -261 71.5t-283 91.5q-105 55 -156.5 136.5t-51.5 182.5q0 111 63 207.5t184 146.5t269 50q163 0 287.5 -52.5t191.5 -154.5t72 -231 l-186 -14q-15 139 -101.5 210t-255.5 71q-176 0 -256.5 -64.5t-80.5 -155.5q0 -79 57 -130q56 -51 292.5 -104.5t324.5 -93.5q128 -59 189 -149.5t61 -208.5q0 -117 -67 -220.5t-192.5 -161t-282.5 -57.5q-199 0 -333.5 58t-211 174.5t-80.5 263.5z",
	"M531 0v1293h-483v173h1162v-173h-485v-1293h-194z",
	"M1120 1466h194v-847q0 -221 -50 -351t-180.5 -211.5t-342.5 -81.5q-206 0 -337 71t-187 205.5t-56 367.5v847h194v-846q0 -191 35.5 -281.5t122 -139.5t211.5 -49q214 0 305 97t91 373v846z",
	"M577 0l-568 1466h210l381 -1065q46 -128 77 -240q34 120 79 240l396 1065h198l-574 -1466h-199z",
	"M414 0l-389 1466h199l223 -961q36 -151 62 -300q56 235 66 271l279 990h234l210 -742q79 -276 114 -519q28 139 73 319l230 942h195l-402 -1466h-187l-309 1117q-39 140 -46 172q-23 -101 -43 -172l-311 -1117h-198z",
	"M9 0l567 764l-500 702h231l266 -376q83 -117 118 -180q49 80 116 167l295 389h211l-515 -691l555 -775h-240l-369 523q-31 45 -64 98q-49 -80 -70 -110l-368 -511h-233z",
	"M571 0v621l-565 845h236l289 -442q80 -124 149 -248q66 115 160 259l284 431h226l-585 -845v-621h-194z",
	"M41 0v180l751 939q80 100 152 174h-818v173h1050v-173l-823 -1017l-89 -103h936v-173h-1159z",
	"M139 -407v1873h397v-149h-217v-1575h217v-149h-397z",
	"M425 -25l-425 1516h145l424 -1516h-144z",
	"M436 -407h-397v149h217v1575h-217v149h397v-1873z",
	"M239 690h-185l353 801h145l355 -801h-181l-247 597z",
	"M-31 -407v130h1193v-130h-1193z",
	"M465 1194h-145l-231 280h241z",
	"M828 131q-100 -85 -192.5 -120t-198.5 -35q-175 0 -269 85.5t-94 218.5q0 78 35.5 142.5t93 103.5t129.5 59q53 14 160 27q218 26 321 62q1 37 1 47q0 110 -51 155q-69 61 -205 61q-127 0 -187.5 -44.5t-89.5 -157.5l-176 24q24 113 79 182.5t159 107t241 37.5 q136 0 221 -32t125 -80.5t56 -122.5q9 -46 9 -166v-240q0 -251 11.5 -317.5t45.5 -127.5h-188q-28 56 -36 131zM813 533q-98 -40 -294 -68q-111 -16 -157 -36t-71 -58.5t-25 -85.5q0 -72 54.5 -120t159.5 -48q104 0 185 45.5t119 124.5q29 61 29 180v66z",
	"M301 0h-167v1466h180v-523q114 143 291 143q98 0 185.5 -39.5t144 -111t88.5 -172.5t32 -216q0 -273 -135 -422t-324 -149q-188 0 -295 157v-133zM299 539q0 -191 52 -276q85 -139 230 -139q118 0 204 102.5t86 305.5q0 208 -82.5 307t-199.5 99q-118 0 -204 -102.5 t-86 -296.5z",
	"M828 389l177 -23q-29 -183 -148.5 -286.5t-293.5 -103.5q-218 0 -350.5 142.5t-132.5 408.5q0 172 57 301t173.5 193.5t253.5 64.5q173 0 283 -87.5t141 -248.5l-175 -27q-25 107 -88.5 161t-153.5 54q-136 0 -221 -97.5t-85 -308.5q0 -214 82 -311t214 -97q106 0 177 65 t90 200z",
	"M824 0v134q-101 -158 -297 -158q-127 0 -233.5 70t-165 195.5t-58.5 288.5q0 159 53 288.5t159 198.5t237 69q96 0 171 -40.5t122 -105.5v526h179v-1466h-167zM255 530q0 -204 86 -305t203 -101q118 0 200.5 96.5t82.5 294.5q0 218 -84 320t-207 102q-120 0 -200.5 -98 t-80.5 -309z",
	"M862 342l186 -23q-44 -163 -163 -253t-304 -90q-233 0 -369.5 143.5t-136.5 402.5q0 268 138 416t358 148q213 0 348 -145t135 -408q0 -16 -1 -48h-792q10 -175 99 -268t222 -93q99 0 169 52t111 166zM271 633h593q-12 134 -68 201q-86 104 -223 104q-124 0 -208.5 -83 t-93.5 -222z",
	"M178 0v922h-159v140h159v113q0 107 19 159q26 70 91.5 113.5t183.5 43.5q76 0 168 -18l-27 -157q-56 10 -106 10q-82 0 -116 -35t-34 -131v-98h207v-140h-207v-922h-179z",
	"M102 -88l175 -26q11 -81 61 -118q67 -50 183 -50q125 0 193 50t92 140q14 55 13 231q-118 -139 -294 -139q-219 0 -339 158t-120 379q0 152 55 280.5t159.5 198.5t245.5 70q188 0 310 -152v128h166v-918q0 -248 -50.5 -351.5t-160 -163.5t-269.5 -60q-190 0 -307 85.5 t-113 257.5zM251 550q0 -209 83 -305t208 -96q124 0 208 95.5t84 299.5q0 195 -86.5 294t-208.5 99q-120 0 -204 -97.5t-84 -289.5z",
	"M135 0v1466h180v-526q126 146 318 146q118 0 205 -46.5t124.5 -128.5t37.5 -238v-673h-180v673q0 135 -58.5 196.5t-165.5 61.5q-80 0 -150.5 -41.5t-100.5 -112.5t-30 -196v-581h-180z",
	"M136 1259v207h180v-207h-180zM136 0v1062h180v-1062h-180z",
	"M134 1257v209h180v-209h-180zM-94 -412l34 153q54 -14 85 -14q55 0 82 36.5t27 182.5v1116h180v-1120q0 -196 -51 -273q-65 -100 -216 -100q-73 0 -141 19z",
	"M136 0v1466h180v-836l426 432h233l-406 -394l447 -668h-222l-351 543l-127 -122v-421h-180z",
	"M131 0v1466h180v-1466h-180z",
	"M135 0v1062h161v-149q50 78 133 125.5t189 47.5q118 0 193.5 -49t106.5 -137q126 186 328 186q158 0 243 -87.5t85 -269.5v-729h-179v669q0 108 -17.5 155.5t-63.5 76.5t-108 29q-112 0 -186 -74.5t-74 -238.5v-617h-180v690q0 120 -44 180t-144 60q-76 0 -140.5 -40 t-93.5 -117t-29 -222v-551h-180z",
	"M135 0v1062h162v-151q117 175 338 175q96 0 176.5 -34.5t120.5 -90.5t56 -133q10 -50 10 -175v-653h-180v646q0 110 -21 164.5t-74.5 87t-125.5 32.5q-115 0 -198.5 -73t-83.5 -277v-580h-180z",
	"M68 531q0 295 164 437q137 118 334 118q219 0 358 -143.5t139 -396.5q0 -205 -61.5 -322.5t-179 -182.5t-256.5 -65q-223 0 -360.5 143t-137.5 412zM253 531q0 -204 89 -305.5t224 -101.5q134 0 223 102t89 311q0 197 -89.5 298.5t-222.5 101.5q-135 0 -224 -101 t-89 -305z",
	"M135 -407v1469h164v-138q58 81 131 121.5t177 40.5q136 0 240 -70t157 -197.5t53 -279.5q0 -163 -58.5 -293.5t-170 -200t-234.5 -69.5q-90 0 -161.5 38t-117.5 96v-517h-180zM298 525q0 -205 83 -303t201 -98q120 0 205.5 101.5t85.5 314.5q0 203 -83.5 304t-199.5 101 q-115 0 -203.5 -107.5t-88.5 -312.5z",
	"M812 -407v520q-42 -59 -117.5 -98t-160.5 -39q-189 0 -325.5 151t-136.5 414q0 160 55.5 287t161 192.5t231.5 65.5q197 0 310 -166v142h162v-1469h-180zM257 534q0 -205 86 -307.5t206 -102.5q115 0 198 97.5t83 296.5q0 212 -87.5 319t-205.5 107q-117 0 -198.5 -99.5 t-81.5 -310.5z",
	"M133 0v1062h162v-161q62 113 114.5 149t115.5 36q91 0 185 -58l-62 -167q-66 39 -132 39q-59 0 -106 -35.5t-67 -98.5q-30 -96 -30 -210v-556h-180z",
	"M63 317l178 28q15 -107 83.5 -164t191.5 -57q124 0 184 50.5t60 118.5q0 61 -53 96q-37 24 -184 61q-198 50 -274.5 86.5t-116 101t-39.5 142.5q0 71 32.5 131.5t88.5 100.5q42 31 114.5 52.5t155.5 21.5q125 0 219.5 -36t139.5 -97.5t62 -164.5l-176 -24 q-12 82 -69.5 128t-162.5 46q-124 0 -177 -41t-53 -96q0 -35 22 -63q22 -29 69 -48q27 -10 159 -46q191 -51 266.5 -83.5t118.5 -94.5t43 -154q0 -90 -52.5 -169.5t-151.5 -123t-224 -43.5q-207 0 -315.5 86t-138.5 255z",
	"M528 161l26 -159q-76 -16 -136 -16q-98 0 -152 31t-76 81.5t-22 212.5v611h-132v140h132v263l179 108v-371h181v-140h-181v-621q0 -77 9.5 -99t31 -35t61.5 -13q30 0 79 7z",
	"M831 0v156q-124 -180 -337 -180q-94 0 -175.5 36t-121 90.5t-55.5 133.5q-11 53 -11 168v658h180v-589q0 -141 11 -190q17 -71 72 -111.5t136 -40.5t152 41.5t100.5 113t29.5 207.5v569h180v-1062h-161z",
	"M430 0l-404 1062h190l228 -636q37 -103 68 -214q24 84 67 202l236 648h185l-402 -1062h-168z",
	"M331 0l-325 1062h186l169 -613l63 -228q4 17 55 219l169 622h185l159 -616l53 -203l61 205l182 614h175l-332 -1062h-187l-169 636l-41 181l-215 -817h-188z",
	"M15 0l388 552l-359 510h225l163 -249q46 -71 74 -119l81 117l179 251h215l-367 -500l395 -562h-221l-218 330l-58 89l-279 -419h-218z",
	"M127 -409l-20 169q59 -16 103 -16q60 0 96 20t59 56q17 27 55 134q5 15 16 44l-403 1064h194l221 -615q43 -117 77 -246q31 124 74 242l227 619h180l-404 -1080q-65 -175 -101 -241q-48 -89 -110 -130.5t-148 -41.5q-52 0 -116 22z",
	"M40 0v146l676 776q-115 -6 -203 -6h-433v146h868v-119l-575 -674l-111 -123q121 9 227 9h491v-155h-940z",
	"M57 612q77 2 125.5 41.5t64.5 108.5t17 236t6 220q9 84 33.5 135t60.5 81.5t92 46.5q38 10 124 10h56v-157h-31q-104 0 -138 -37.5t-34 -167.5q0 -262 -11 -331q-18 -107 -61.5 -165t-136.5 -103q110 -46 159.5 -140.5t49.5 -309.5q0 -195 4 -232q8 -68 40.5 -95 t127.5 -27h31v-157h-56q-98 0 -142 16q-64 23 -106 74.5t-54.5 130.5t-13.5 259t-17 249t-64.5 109t-125.5 42v163z",
	"M188 -431v1922h157v-1922h-157z",
	"M626 612v-163q-77 -2 -125.5 -42t-64.5 -108.5t-17 -235.5t-6 -220q-9 -85 -33.5 -135.5t-60.5 -81t-92 -46.5q-38 -11 -124 -11h-56v157h31q104 0 138 37.5t34 168.5q0 250 9 317q16 111 64 177.5t136 103.5q-115 55 -162 145.5t-47 305.5q0 195 -5 233 q-7 67 -39.5 93.5t-127.5 26.5h-31v157h56q98 0 142 -16q64 -22 106 -74t54.5 -131t13.5 -259t17 -248.5t64.5 -108.5t125.5 -42z",
	"M87 557v205q106 120 278 120q60 0 126 -17.5t188 -69.5q69 -29 103.5 -38t69.5 -9q65 0 134.5 39t123.5 98v-212q-64 -60 -129.5 -87t-147.5 -27q-60 0 -114.5 14t-173 67t-197.5 53q-64 0 -120.5 -27.5t-140.5 -108.5z"
 };

float Otk_HelvFont_x_adv[] =
{
	 0.2845, 0.2845, 0.3635, 0.5695, 0.5695, 0.9105, 0.683, 0.1955, 0.341, 0.341, 0.3985, 0.598, 0.2845, 0.341, 0.2845, 
	 0.2845, 0.5695, 0.5695, 0.5695, 0.5695, 0.5695, 0.5695, 0.5695, 0.5695, 0.5695, 0.5695, 0.2845, 0.2845, 0.598, 
	 0.598, 0.598, 0.5695, 1.0395, 0.683, 0.683, 0.7395, 0.7395, 0.683, 0.6255, 0.7965, 0.7395, 0.2845, 0.512, 0.683,
	 0.5695, 0.853, 0.7395, 0.7965, 0.683, 0.7965, 0.7395, 0.683, 0.6255, 0.7395, 0.683, 0.9665, 0.683, 0.683, 0.6255,
	 0.2845, 0.2845, 0.2845, 0.4805, 0.5695, 0.341, 0.5695, 0.5695, 0.512, 0.5695, 0.5695, 0.2845, 0.5695, 0.5695,
	 0.2275, 0.2275, 0.512, 0.2275, 0.853, 0.5695, 0.5695, 0.5695, 0.5695, 0.341, 0.512, 0.2845, 0.5695, 0.512, 0.7395,
	 0.512, 0.512, 0.512, 0.342, 0.266, 0.342, 0.598,
};


OtkFont *Otk_CreateHelvFont( int spacing )
{
 OtkFont *newfont;
 int j;

 newfont = (OtkFont *)calloc( 1, sizeof(OtkFont) );
 newfont->family = strdup("OtkArial");
 newfont->weight = 0.2;
 newfont->units_per_em = 2000.0;
 newfont->bbox[0] = -0.68001;
 newfont->bbox[1] = -0.3325;
 newfont->bbox[2] = 2.077;
 newfont->bbox[3] = 1.03;
 newfont->ascent = 0.819;
 newfont->descent = -0.205;
 newfont->x_height = 0.531;
 newfont->underline_thickness = 0.075;
 newfont->underline_position = -0.1085;
 newfont->start_glyph = 0;
 newfont->end_glyph = 255;
 newfont->missing_glyph = 0;
 newfont->glyphs = 1;
 newfont->metrics = 0;
 newfont->glyph_defs = (OtkGlyph **)calloc( 256, sizeof(OtkGlyph *) );

 for (j=32; j<=126; j++)
  {
   newfont->glyph_defs[j] = (OtkGlyph *)calloc( 1, sizeof(OtkGlyph) );
   newfont->glyph_defs[j]->path = Otk_HelvFont_Paths[j-32];
   newfont->glyph_defs[j]->x_adv = Otk_HelvFont_x_adv[j-32];
  }

 { /*Realize_font*/
 int num_glyphs;
 int i, chr;
 float scale=1.0;

 num_glyphs = newfont->end_glyph - newfont->start_glyph + 1; /* 126 - 32 + 1 = 95 */

 newfont->glyphs = glGenLists( num_glyphs );
 for( i = 0; i < num_glyphs; i++ )
  {
    // newfont->metrics = NULL; /* no metrics available */
   glNewList( newfont->glyphs + i, GL_COMPILE );
   glTranslatef( 0.1, -0.8, 0.0 );
   
   chr = i;
   if( !newfont->glyph_defs[chr] && islower( chr ) )
    {
     chr = toupper( chr );
     scale = 0.8;
     newfont->units_per_em *= 1.0/scale;
    }

   if( newfont->glyph_defs[chr] )
    {
     //printf( "SVG font rendering '%c' x_adv=%g\n", i, newfont->glyph_defs[i]->x_adv );
     if( newfont->glyph_defs[chr]->path )
       Otk_gl_from_SVG_path( newfont, newfont->glyph_defs[chr]->path, chr );
     glTranslatef( newfont->glyph_defs[chr]->x_adv*scale, 0.8, 0.0 );
     if (spacing==Otk_FontSpacing_Mono) newfont->glyph_defs[i]->x_adv = 0.5;  // 0.7395;
    }
   else
    {
     glBegin( GL_LINE_STRIP );
     glVertex3f( 0.0, 0.0, 0.0 );
     glVertex3f( 0.0, 1.0, 0.0 );
     glVertex3f( 0.7, 1.0, 0.0 );
     glVertex3f( 0.7, 0.0, 0.0 );
     glVertex3f( 0.0, 0.0, 0.0 );
     glEnd();
     glTranslatef( 0.9, 0.8, 0.0 );
    }

   if( chr != i )
    {
     newfont->units_per_em *= scale;
     scale = 1.0;
    }

   glEndList();
  }
 } /*Realize_font*/

 return newfont;
}




OtkFont *Otk_Build_Internal_Font( int font_kind, int spacing )
{
  switch (font_kind)
   {
    case Otk_Font_Vect:  return Otk_CreateBasicVectorFont();  break;
    case Otk_Font_Helv:  return Otk_CreateHelvFont(spacing);  break;
    default: printf("Otk Error:  Unknown font kind %d\n", font_kind ); return 0;
   }
}


