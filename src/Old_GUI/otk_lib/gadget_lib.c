/****************************************************************/
/* Gadget_lib.c - The Otk Gadget Library, a set of high-level   */
/* widgets, such as meters, bar-graphs, gauges, and other       */
/* re-usable displays.                                          */
/*                                                              */
/****************************************************************/

#include "gadget_lib.h"

void OtkTranslateColor( OtkColor Cin, OtkColorVector tmpc );	/* Prototype. */


OtkWidget Otk_MakeLEDmeter( OtkWidget container, float left, float top, float horiz_size, float vert_size, int nbars, char orientation, OtkColor tmpcolor )
{
 OtkWidget tmpobj;
 float x, y, delta;
 int k;

 tmpobj = OtkMakePanel( container, Otk_Recessed, OtkSetColor( 0.65, 0.65, 0.65 ), left, top, horiz_size, vert_size );
 tmpobj->superclass = Otk_SC_LEDmeter;
 tmpobj->nrows = nbars;
 Otk_Set_Object_Border_Thickness( tmpobj, 0.75 );
 delta = 90.0 / (float) nbars;

 if (orientation=='v')
  {
   x = 14.0;  y = 5.0;
   horiz_size = 70.0;
   vert_size = 60.0 / (float) nbars;
  }
 else
  {
   x = 100.0 - 0.5 * (100.0 - 90.0 - 30.0/(float)nbars) - delta;
   y = 14.0;
   horiz_size = 60.0 / (float) nbars;
   vert_size = 70.0;
  }

 for (k=0; k<nbars; k++)
  {
   OtkMakePanel( tmpobj, Otk_Flat, tmpcolor, x, y, horiz_size, vert_size );
   if (orientation=='v') y = y + delta; else x = x - delta;
  }
 return tmpobj;
}


void Otk_SetLEDmeter( OtkWidget container, float value, OtkColor offcolor, OtkColor oncolor )
{
 OtkWidget child;
 int k=0, m;

 if (container->superclass != Otk_SC_LEDmeter) 
  { printf("Error: Otk_SetLEDmeter called on non-guage object.\n"); return; }
 child = container->children;
 if (value>100.0) value = 100.0; else if (value<0.0) value = 0.0;
 m = (float)container->nrows * 0.01 * value + 0.5;
 while (child!=0) 
  {
   if (child->object_class==Otk_class_panel)
    {
     if (m>k) OtkTranslateColor( oncolor, child->color );
     else     OtkTranslateColor( offcolor, child->color );
     k++;
    }
   child = child->nxt;
  }
 Otk_ReDraw_Display();
}


void Otk_SetLEDmeter_Range( OtkWidget container, float minvalue, float maxvalue, OtkColor oncolor )
{
 OtkWidget child;
 int k=0, m1, m2;
 float x;

 if (container->superclass != Otk_SC_LEDmeter) 
  { printf("Error: Otk_SetLEDmeter_Range called on non-guage object.\n"); return; }
 child = container->children;
 if (maxvalue < minvalue) { x = maxvalue;  maxvalue = minvalue;  minvalue = x; }
 if (minvalue > 100.0) minvalue = 100.0;  else  if (minvalue < 0.0) minvalue = 0.0;
 if (maxvalue > 100.0) maxvalue = 100.0;  else  if (maxvalue < 0.0) maxvalue = 0.0;
 m1 = (float)container->nrows * 0.01 * minvalue + 0.5;
 m2 = (float)container->nrows * 0.01 * maxvalue + 0.5;
 while (child!=0) 
  {
   if (child->object_class == Otk_class_panel)
    {
     if ((k >= m1) && (k <= m2)) OtkTranslateColor( oncolor, child->color );
     k++;
    }
   child = child->nxt;
  }
 Otk_ReDraw_Display();
}





OtkWidget Otk_MakeBarMeter( OtkWidget container, float left, float top, float horiz_size, float vert_size, char orientation, OtkColor tmpcolor )
{
 OtkWidget tmpobj, tmpobj2;
 float x, y;

 tmpobj = OtkMakePanel( container, Otk_Recessed, OtkSetColor( 0.65, 0.65, 0.65 ), left, top, horiz_size, vert_size );
 tmpobj->superclass = Otk_SC_BarMeter;
 Otk_Set_Object_Border_Thickness( tmpobj, 0.75 );

 if (orientation=='v')
  {
   x = 14.0;  y = 94.5;
   horiz_size = 68.0;
   vert_size = 0.5;
  }
 else
  {
   x = 5.0;
   y = 14.0;
   horiz_size = 0.5;
   vert_size = 65.0;
  }

 tmpobj2 = OtkMakePanel( tmpobj, Otk_Flat, tmpcolor, x, y, horiz_size, vert_size );
 if (orientation!='v') tmpobj2->horiztextscroll = 1;
 return tmpobj;
}


void Otk_SetBarMeter( OtkWidget container, float value )
{
 OtkWidget child;

 if (container->superclass != Otk_SC_BarMeter) 
  { printf("Error: Otk_SetBarMeter called on non-guage object.\n"); return; }
 child = container->children;
 if (value>100.0) value = 100.0; else if (value<0.0) value = 0.0;
 if (child->object_class==Otk_class_panel)
  {
   if (child->horiztextscroll > 0)
    {	/* Horizontal bar. */
     child->x2 = child->x1 + 0.9 * value + 0.5;
     child->xright = container->xleft + child->x2 * (container->xright - container->xleft) * 0.01;   
    }
   else
    {   /* Vertical bar. */
     child->y1 = 94.5 - 0.9 * value;
     child->ytop = container->ytop + child->y1 * (container->ybottom - container->ytop) * 0.01;   
    }
  }
 Otk_ReDraw_Display();
}


void Otk_SetBarmeter_Range( OtkWidget container, int segment, float minval, float maxval, OtkColor color )
{
 OtkWidget child;
 float x;
 int seg=0, nchildren=0, orien;

 if (container->superclass != Otk_SC_BarMeter) 
  { printf("Error: Otk_SetBarMeter_range called on non-guage object.\n");  return; }
 child = container->children;	/* Children are pushed on in reverse order. */
 while (child != 0) { nchildren++;  child = child->nxt; }  /* Count the children. */
 seg = nchildren - 1;
 if (segment > seg)
  { /*Add new segment. */
    orien = container->children->horiztextscroll;
    if (container->children->horiztextscroll > 0)
     child = OtkMakePanel( container, Otk_Flat, color, 5.0, 14.0, 0.5, 65.0 );
    else
     child = OtkMakePanel( container, Otk_Flat, color, 14.0, 94.5, 68.0, 0.5 );
    child->horiztextscroll = orien;
    seg++;  segment = seg;
  }
 child = container->children;
 while ((seg > segment) && (child != 0))
  { 
   child = child->nxt;
   seg--;
  }
 if (child == 0) return;
 OtkTranslateColor( color, child->color );
 if (maxval < minval) { x = maxval;  maxval = minval;  minval = x; }
 if (minval > 100.0) minval = 100.0;  else  if (minval < 0.0) minval = 0.0;
 if (maxval > 100.0) maxval = 100.0;  else  if (maxval < 0.0) maxval = 0.0;
 if (child->object_class==Otk_class_panel)
  {
   if (child->horiztextscroll > 0)
    {	/* Horizontal bar. */
     x = (container->xright - container->xleft) * 0.01;
     child->x1 = 5.0 + 0.9 * minval + 0.5;
     child->xleft = container->xleft + child->x1 * x;
     child->x2 = 5.0 + 0.9 * maxval + 0.5;
     child->xright = container->xleft + child->x2 * x;
    }
   else
    {   /* Vertical bar. */
     x = (container->ybottom - container->ytop) * 0.01;
     child->y1 = 94.5 - 0.9 * maxval;
     child->ytop = container->ytop + child->y1 * x;
     child->y2 = 94.5 - 0.9 * minval;
     child->ybottom = container->ytop + child->y2 * x;
    }
  }
 Otk_ReDraw_Display();
}



void Otk_SetGauge2( OtkWidget container, float value, OtkColor tmpcolor )
{
 OtkWidget child;
 float r1=0.04, r2=0.37, phi, w=0.9;

 if (container->superclass != Otk_SC_Gauge2) 
  { printf("Error: Otk_SetGauge2 called on non-guage object.\n"); return; }
 child = container->children;
 while ((child!=0) && (child->object_class != Otk_class_triangle)) 
  child = child->nxt;
 if (child==0)
  {
   child = Otk_add_object( Otk_class_triangle, container );
   child->z = container->z + 0.6 * Otk_DZ;
  }
 OtkTranslateColor( tmpcolor, child->color );
 if (value<0.0) value = 0.0; else if (value>100.0) value = 100.0;
 value = 0.833 * value + 33.4;
 phi = 2.0 * 3.14159 * 0.01 * value;

 child->x1 = container->xleft + (0.5 + r2*cos(phi)) * (container->xright - container->xleft);
 child->y1 = container->ytop + (0.5 + r2*sin(phi)) * (container->ybottom - container->ytop);

 child->x2 = container->xleft + (0.5 + r1*cos(phi-w)) * (container->xright - container->xleft);
 child->y2 = container->ytop + (0.5 + r1*sin(phi-w)) * (container->ybottom - container->ytop);

 child->xleft = container->xleft + (0.5 + r1*cos(phi+w)) * (container->xright - container->xleft);
 child->ytop = container->ytop + (0.5 + r1*sin(phi+w)) * (container->ybottom - container->ytop);
 Otk_ReDraw_Display();
}


OtkWidget Otk_MakeGauge2( OtkWidget container, float left, float top, float horiz_size, float vert_size, char *title )
{
 OtkWidget tmpobj, tmpobj1, tmpobj2, tmpobj3;
 float x1, y1, x2, y2, phi=0.0, dphi=3.14159/18.0, radius=43.0, cntr=50.0, dcntr=0.7, r, r1, r2, r3, a, b, scale;
 char text[10];
 int k;

 if (container->object_class != Otk_SC_Panel) {printf("Otk Error: Add Gadget parent not container panel.\n"); return 0;}
 tmpobj = OtkMakePanel( container, Otk_Raised, OtkSetColor( 0.65, 0.65, 0.65 ), left, top, horiz_size, 0.85 * vert_size );
 if ((title!=0) && (title[0]!='\0'))
  {
   tmpobj1 = OtkMakePanel( tmpobj, Otk_Raised, OtkSetColor( 0.65, 0.65, 0.65 ), 0.0, 100.0, 100.0, 14.0 );
   tmpobj1 = OtkMakeTextLabel( tmpobj1, title, Otk_Black, 1.5, 1.0, 2, 20 );
   Otk_FitTextInPanel(tmpobj1 );
  }

 tmpobj->superclass = Otk_SC_Gauge2;
 Otk_Set_Object_Border_Thickness( tmpobj, 0.5 );

 tmpobj2 = Otk_MakeDisk( tmpobj, 50.0, 50.0, radius, Otk_White );
 tmpobj2->z = tmpobj->z + 0.75;

 r = radius;
 while (dcntr < 2.0)
  { /*dcntr*/
   phi = 45.0 * 3.14159 / 180.0;
   cntr = 50.0 + dcntr;
   x2 = cntr + r * cos(phi); 
   y2 = cntr - r * sin(phi); 
   phi = phi + dphi;
   while (phi < 225.0 * 3.14159 / 180.0)
    {
     x1 = x2;  y1 = y2;
     x2 = cntr + r * cos(phi); 
     y2 = cntr - r * sin(phi);
     Otk_Add_Line( tmpobj, Otk_DarkGray, 4.0, x1, y1, x2, y2 );
     phi = phi + dphi;
    }

   phi = 225.0 * 3.14159 / 180.0;
   cntr = 50.0 - dcntr;
   x2 = cntr + r * cos(phi); 
   y2 = cntr - r * sin(phi); 
   phi = phi + dphi;
   while (phi < 395.0 * 3.14158/180.0 + 0.5 * dphi)
    {
     x1 = x2;  y1 = y2;
     x2 = cntr + r * cos(phi); 
     y2 = cntr - r * sin(phi);
     Otk_Add_Line( tmpobj, OtkSetColor(0.8,0.8,0.8), 4.0, x1, y1, x2, y2 );
     phi = phi + dphi;
    }
   dcntr = dcntr + 0.7;
  } /*dcntr*/

 phi = 0.0;
 x2 = 50.0 + radius * cos(phi); 
 y2 = 50.0 + radius * sin(phi); 
 phi = phi + dphi;
 while (phi < 2.0 * 3.14158 + 0.5 * dphi)
  {
   x1 = x2;  y1 = y2;
   x2 = 50.0 + radius * cos(phi); 
   y2 = 50.0 + radius * sin(phi);
   Otk_Add_Line( tmpobj, Otk_DarkGray, 4.0, x1, y1, x2, y2 );
   phi = phi + dphi;
  }

 r1 = 0.83 * radius;
 r2 = 0.93 * radius;
 r3 = 0.675 * radius;
 dphi = 2.0 * 3.14159 / 12.0;
 phi = -2.0 * dphi;
 k = 100;
 while (phi < 3.14158 + 2.5 * dphi)
  {
   a = cos(phi);
   b = sin(phi);
   x1 = 50.0 + r1 * a;
   y1 = 50.0 - r1 * b;
   x2 = 50.0 + r2 * a;
   y2 = 50.0 - r2 * b;
   Otk_Add_Line( tmpobj, Otk_Black, 2.0, x1, y1, x2, y2 );  		/* Tick marks. */
   sprintf(text,"%d", k);  k = k - 10;
   x1 = 44.5 + r3 * a;
   y1 = 46.0 - 1.0 * r3 * b;
   scale = (tmpobj->xright - tmpobj->xleft) / 25.0;
   tmpobj3 = OtkMakeTextLabel( tmpobj, text, Otk_Black, scale, 1.0, x1, y1 );	/* Numerals. */
   tmpobj3->z = tmpobj3->z + 1.0;
   phi = phi + dphi;
  }

 /* Rainbow. */
 r1 = 0.25 * radius;
 r2 = 0.5 * radius;
 r = 1.0;  b = 0.0;
 dphi = 3.14159 / 50.0;
 phi = -0.3 * 3.14159;
 while (phi < 1.33 * 3.14159)
  {
   a = cos(phi);
   b = sin(phi);
   x1 = 50.0 + r1 * a;
   y1 = 50.0 - r1 * b;
   x2 = 50.0 + r2 * a;
   y2 = 50.0 - r2 * b;
   b = 1.8 * ((phi + 0.3 * 3.14159) / (1.6 * 3.14159));
   if (b>1.0) b = 1.0;
   r = 1.8 * ((1.6 * 3.14159 - (phi + 0.3 * 3.14159)) / (1.6 * 3.14159));
   if (r>1.0) r = 1.0;
   // printf("%g: r=%g g=%g\n", 180.0 * phi/3.14159, r, b );
   Otk_Add_Line( tmpobj, OtkSetColor( r, b, 0.0 ), 5.0, x1, y1, x2, y2 );  /* Inner rainbow. */
   phi = phi + dphi;
  }

 radius = 3.0;
 b = 0.2;
 x1 = -0.5 * radius;  x2 = 0.5 * radius;
 y1 = radius;  y2 = radius;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0+x1, 50.0+y1, 50.0+x2, 50.0+y2 );	/* bottom. */
 b = 0.6;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0+x1, 50.0-y1, 50.0+x2, 50.0-y2 );	/* top. */
 x2 = -radius;  y2 = 0.5 * radius;	b = 1.0;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0+x1, 50.0-y1, 50.0+x2, 50.0-y2 );        /* upper left. */
 b = 0.45;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0-x1, 50.0-y1, 50.0-x2, 50.0-y2 );        /* upper right. */
 b = 0.3;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0+x1, 50.0+y1, 50.0+x2, 50.0+y2 );        /* lower left. */
 b = 0.0;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0-x1, 50.0+y1, 50.0-x2, 50.0+y2 );        /* lower right. */
 x1 = -radius;  y1 = 0.5 * radius;  y2 = -0.5 * radius;	 b = 0.45;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0-x1, 50.0+y1, 50.0-x2, 50.0+y2 );        /* left. */
 b = 0.38;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0+x1, 50.0+y1, 50.0+x2, 50.0+y2 );        /* right. */
 tmpobj2->z = tmpobj2->z + 0.25;

 radius = 1.5;
 b = 0.2;
 x1 = -0.5 * radius;  x2 = 0.5 * radius;
 y1 = radius;  y2 = radius;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0+x1, 50.0+y1, 50.0+x2, 50.0+y2 );	/* bottom. */
 b = 0.6;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0+x1, 50.0-y1, 50.0+x2, 50.0-y2 );	/* top. */
 x2 = -radius;  y2 = 0.5 * radius;	b = 1.0;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0+x1, 50.0-y1, 50.0+x2, 50.0-y2 );        /* upper left. */
 b = 0.45;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0-x1, 50.0-y1, 50.0-x2, 50.0-y2 );        /* upper right. */
 b = 0.3;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0+x1, 50.0+y1, 50.0+x2, 50.0+y2 );        /* lower left. */
 b = 0.0;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0-x1, 50.0+y1, 50.0-x2, 50.0+y2 );        /* lower right. */
 x1 = -radius;  y1 = 0.5 * radius;  y2 = -0.5 * radius;	 b = 0.45;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0-x1, 50.0+y1, 50.0-x2, 50.0+y2 );        /* left. */
 b = 0.38;  tmpobj2->z = tmpobj2->z + 0.25;
 tmpobj2 = Otk_Add_Line( tmpobj, OtkSetColor( b, b, b), 4.0, 50.0+x1, 50.0+y1, 50.0+x2, 50.0+y2 );        /* right. */
 tmpobj2->z = tmpobj2->z + 0.25;

 Otk_SetGauge2( tmpobj, 0.0, OtkSetColor( 0.8, 0.0, 0.2) );
 return tmpobj;
}





void Otk_Rotate_Text( OtkWidget label, float angle )
{
 label->outlinestyle = 1;        /* Set rotated style. */
}


OtkWidget Otk_Plot_Init( OtkWidget container, char *xlabel, char *ylabel, float xpos, float ypos,
			 float width, float height, OtkColor fgcolor, OtkColor bgcolor )
{ /* Iinitialize a Plot-Gadget - Create plot-panel and place axis border and labels. */
  OtkWidget plotbox, tmpobj, label;
  float x, y, hsz, vsz, h, v;
 
  if (container->object_class != Otk_SC_Panel) {printf("Otk Error: Add Plot-Gadget parent not container panel.\n"); return 0;}
  plotbox = OtkMakePanel( container, Otk_Raised, bgcolor, xpos, ypos, width, height );
  plotbox->superclass = Otk_SC_XYgraph;
  Otk_Set_Object_Border_Thickness( plotbox, 0.2 );
  Otk_Add_BoundingBox( plotbox, fgcolor, 2.0, 21.0, 5.0, 96.0, 84.0 );
  plotbox->children->superclass = Otk_SC_XYgraph;
  tmpobj = plotbox->children;	/* Set bounding box segments as members of this graph. */
  while (tmpobj != 0) { tmpobj->superclass = Otk_SC_XYgraph;  tmpobj = tmpobj->nxt; }

  /* Place X-axis label. */
  label = OtkMakeTextLabel( plotbox, xlabel, fgcolor, 1.0, 1.0, 1.0, 100.0 );
  label->superclass = Otk_SC_XYgraph;
  Otk_Get_Text_Size( label, &h, &v );
  hsz = 0.9*(plotbox->xright - plotbox->xleft);
  vsz = 0.09*(plotbox->ybottom - plotbox->ytop);
  label->sqrtaspect = sqrt( (v*hsz)/(h*vsz) );
  label->scale = 0.8*hsz / (h*label->sqrtaspect);
  //  if( label->sqrtaspect > 2.0 )
  //    label->sqrtaspect = 2.0;
  Otk_Get_Text_Size( label, &h, &v );
  x = 0.5 - 0.5*h*(0.9/hsz);
  Otk_move_object( label, 1, 100.0*(x + 0.4*x*x), 98.0 - 8.5*v/vsz );
  
  /* Place Y-axis label. */
  label = OtkMakeTextLabel( plotbox, ylabel, fgcolor, 1.0, 1.0, 1.0, 100.0 );
  label->superclass = Otk_SC_XYgraph;
  Otk_Get_Text_Size( label, &v, &h );
  hsz = 0.04 * (plotbox->xright - plotbox->xleft);	/* Tallness of characters. */
  vsz = plotbox->ybottom - plotbox->ytop;
  if( v < 10.0 )
   {
    label->sqrtaspect = sqrt( (h * vsz*0.65) / (v * hsz) );
    label->scale = 0.9 * vsz*0.65 / (v * label->sqrtaspect);
   }
  else
   {
    label->sqrtaspect = sqrt( (h * vsz*0.85) / (v * hsz) );
    label->scale = 0.9 * vsz*0.85 / (v * label->sqrtaspect);
   }
  Otk_Get_Text_Size( label, &h, &v );
  Otk_Rotate_Text( label, 90.0 );	// Set rotated style.
  y = 0.5 - 0.5*h/vsz;
  Otk_move_object( label, 1, 2.0, 100.0*(1.0 - y - 0.3*y*y) );

  plotbox->callback_param = 0;
  return plotbox;
}



void Otk_Plot_Set_Axes( OtkWidget plotbox, float xmin, float xmax, int xgrids,  
					   float ymin, float ymax, int ygrids,  
					   OtkColor tcolor, OtkColor gcolor )
{ /* Erases any previous data and places new axes. */
  OtkWidget tmpobj, tmpobj2, label;
  float range, step, ftmp, scale=1.0, x, y, yy, xx;
  float hsz, hsz1, vsz, v, h;
  int k;
  char word[50];

 if (plotbox->superclass != Otk_SC_XYgraph) 
  { printf("Error: Otk_Plot_Set_Axes called on non-XYgraph object.\n"); return; }

  /* Remove any previous axis or data items. */
  tmpobj = plotbox->children;
  while (tmpobj != 0)
   {
    if (tmpobj->superclass != Otk_SC_XYgraph)
     {
      tmpobj2 = tmpobj;
      tmpobj = tmpobj->nxt;
      Otk_object_detach( tmpobj2 );
      free( tmpobj2 );
     }
    else 
     tmpobj = tmpobj->nxt;
   }

  /* Record the axis ranges and place on hidden list. */
  if (plotbox->hidden_children) Otk_object_detach_hidden( plotbox->hidden_children );
  tmpobj = Otk_Add_Line( plotbox, gcolor, 1.0, xmin, ymin, xmax, ymax );
  Otk_object_detach( tmpobj );
  Otk_object_attach_hidden( plotbox, tmpobj );

 /* Add X-axis numbers and grids. */
  if (xmax<xmin) { ftmp = xmax;  xmax = xmin;  xmin = ftmp; }
  range = xmax - xmin;
  if (range<1e-9) {printf("Otk_Plot: x-range too small. %g\n", range); range = 1e-9;}
 /* Find a nice step size which is a round number multiple of 1, 2, or 5. */
  step = range / (int)xgrids;	k = 0;
  if (step<1.0) { while (step < 0.75) {step = 10.0 * step;  k++;} }
  else          { while (step >= 7.5) {step = 0.1 * step;  k--;} }
  if (step < 1.5) step = 1.0; else 
  if (step < 3.5) step = 2.0; else step = 5.0;
  while (k>0) { step = 0.1 * step; k--; }
  while (k<0) { step = 10.0 * step; k++; }
 /* Find first value. (Integer multiple of step that comes closest, but larger than min_start). */
  k = xmin/step;
  x = (float)k*step;
  if (x < xmin) { if (xmin < 0.0) {k--;  x = (float)k*step;} else {k++;  x = (float)k*step;} }
 /* Step until end. */
  y = 85.8;
  while (x <= xmax)
   {
    xx = 21.0+(96.0-21.0)*(x-xmin)/range;
    if ((xmax-x)/range > 0.05)
     {
      if (step<0.000001) sprintf(word,"%e", x); else
      if (step<0.00001) sprintf(word,"%1.6f", x); else
      if (step<0.0001) sprintf(word,"%1.5f", x); else
      if (step<0.001) sprintf(word,"%1.4f", x); else
      if (step<0.01) sprintf(word,"%1.3f", x); else
      if (step<0.1) sprintf(word,"%1.2f", x); else
      if (step<1.0) sprintf(word,"%1.1f", x); else sprintf(word,"%g", x);
      label = OtkMakeTextLabel( plotbox, word, tcolor, scale, 1.0, xx, y );
      label->sqrtaspect = 1.0;  label->scale = 1.0;
      Otk_Get_Text_Size( label, &h, &v );
      if (strlen(word) > 5) hsz1 = 0.11;  else  hsz1 = 0.13 * (float)strlen(word)/4.0;
      hsz = hsz1 * (plotbox->xright - plotbox->xleft);
      vsz = 0.085 * (plotbox->ybottom - plotbox->ytop);
      label->sqrtaspect = sqrt( (v * hsz) / (h * 0.8 * vsz) );
      label->scale = 0.925 * hsz / (h * label->sqrtaspect);
      label->x1 = xx + 1.0 - 50.0 * hsz1;
      label->xleft = plotbox->xleft + label->x1 * (plotbox->xright - plotbox->xleft) * 0.01;
     }
    if (((x-xmin)/range > 0.02) && ((xmax-x)/range > 0.02))
     Otk_Add_Line( plotbox, gcolor, 1.0, xx, 5.5, xx, 83.5 );
    x = x + step;
   }

 /* Add Y-axis numbers and grids. */
  if (ymax<ymin) { ftmp = ymax;  ymax = ymin;  ymin = ftmp; }
  range = ymax - ymin;
  if (range<1e-9) {printf("Otk_Plot: y-range too small. %g\n", range); range = 1e-9;}
 /* Find a nice step size which is a round number multiple of 1, 2, or 5. */
  step = range / (int)ygrids;	k = 0;
  if (step<1.0) { while (step < 0.75) {step = 10.0 * step;  k++;} }
  else          { while (step >= 7.5) {step = 0.1 * step;  k--;} }
  if (step < 1.5) step = 1.0; else 
  if (step < 3.5) step = 2.0; else step = 5.0;
  while (k>0) { step = 0.1 * step; k--; }
  while (k<0) { step = 10.0 * step; k++; }
 /* Find first value. (Integer multiple of step that comes closest, but larger than min_start). */
  k = ymin/step;
  y = (float)k*step;
  if (y < ymin) { if (ymin < 0.0) {k--;  y = (float)k*step;} else {k++;  y = (float)k*step;} }
 /* Step until end. */
  x = 19.0;
  while (y<=ymax)
   {
    if (step<0.000001) sprintf(word,"%e", y); else
    if (step<0.00001) sprintf(word,"%1.6f", y); else
    if (step<0.0001) sprintf(word,"%1.5f", y); else
    if (step<0.001) sprintf(word,"%1.4f", y); else
    if (step<0.01) sprintf(word,"%1.3f", y); else
    if (step<0.1) sprintf(word,"%1.2f", y); else
    if (step<1.0) sprintf(word,"%1.1f", y); else
    sprintf(word,"%g", y);
    yy = 5.0+(81.5-5.0)*(ymax-y)/range;
    label = OtkMakeTextLabel( plotbox, word, tcolor, scale, 1.0, x, yy - 1.0 );
    label->sqrtaspect = 1.0;  label->scale = 1.0;
    Otk_Get_Text_Size( label, &h, &v );
    if (strlen(word) > 5) hsz1 = 0.12;  else  hsz1 = 0.13 * (float)strlen(word)/4.0;
    hsz = hsz1 * (plotbox->xright - plotbox->xleft);
    vsz = 0.085 * (plotbox->ybottom - plotbox->ytop);
    label->sqrtaspect = sqrt( (v * hsz) / (h * 0.8 * vsz) );
    label->scale = 0.925 * hsz / (h * label->sqrtaspect);
    label->x1 = x - 78.0 * hsz1;
    label->xleft = plotbox->xleft + label->x1 * (plotbox->xright - plotbox->xleft) * 0.01;
    yy = 5.0+(84.0-5.0)*(ymax-y)/range;
    if ((fabs(y-ymin)/range > 0.02) && (fabs(y-ymax)/range > 0.02))
     Otk_Add_Line( plotbox, gcolor, 1.0, 21.5, yy, 95.5, yy );
    y = y + step;
   }

}


void Otk_Plot_Data( OtkWidget plotbox, int npoints, float *xarray, float *yarray, OtkColor color )
{
 int k, leave;
 float x1, y1, x2, y2, xmin, xmax, ymin, ymax, dx, dy, x, y, ox, oy, xx, yy, oxx, oyy;

 if (plotbox->superclass != Otk_SC_XYgraph) 
  { printf("Error: Otk_Plot_Data called on non-XYgraph object.\n"); return; }
 if (plotbox->hidden_children==0)  { printf("Error: Otk_Plot_Data called on plot with unset axes ranges.\n"); return;}
 xmin = plotbox->hidden_children->x1;
 ymin = plotbox->hidden_children->y1;
 xmax = plotbox->hidden_children->x2;
 ymax = plotbox->hidden_children->y2;
 // printf("RANGES: %g, %g - %g, %g\n", xmin, ymin, xmax, ymax );
 if (npoints<=1) return;
 x2 = xarray[0];
 y2 = yarray[0];
 for (k=1; k<npoints; k++)
  {
   leave = 0;
   x1 = x2;         y1 = y2;
   x2 = xarray[k];  y2 = yarray[k];

   ox = x1; 	   oy = y1;
   x = x2;	   y = y2;

   /* Enforce graph borders. */
   if (((x<xmin) && (ox<xmin)) || ((x>xmax) && (ox>xmax)) || ((y<ymin) && (oy<ymin)) || ((y>ymax) && (oy>ymax))) leave = 1;
   else
   { /* in boundary */
    dx = ox - x;  dy = oy - y;
    if ((x<xmin) || (x>xmax)) 
     {
      if (dx==0.0) leave = 1;
      else 
         if (x<xmin) { y = dy/dx * (xmin-x) + y; x = xmin; }
         else { y = dy/dx * (xmax-x) + y; x = xmax; }
     }
    if ((y<ymin) || (y>ymax)) 
     {
      if (dy==0.0) leave = 1;
      else if (y<ymin) { x = dx/dy * (ymin-y) + x; y = ymin; }
           else { x = dx/dy * (ymax-y) + x; y = ymax; }
     }
    if ((ox<xmin) || (ox>xmax)) 
     {
      if (dx==0.0) leave = 1;
      else if (ox<xmin) { oy = dy/dx * (xmin-ox) + oy; ox = xmin; }
           else { oy = dy/dx * (xmax-ox) + oy; ox = xmax; }
     }
    if ((oy<ymin) || (oy>ymax)) 
     {
      if (dy==0.0) leave = 1;
      else if (oy<ymin) { ox = dx/dy * (ymin-oy) + ox; oy = ymin; }
           else { ox = dx/dy * (ymax-oy) + ox; oy = ymax; }
     }
    if (!leave)
     {
      oxx = 21.0+(96.0-21.0)*(ox-xmin)/(xmax-xmin);
      oyy = 5.0+(84.0-5.0)*(ymax-oy)/(ymax-ymin);
      xx = 21.0+(96.0-21.0)*(x-xmin)/(xmax-xmin);
      yy = 5.0+(84.0-5.0)*(ymax-y)/(ymax-ymin);
      Otk_Add_Line( plotbox, color, 1.0, oxx, oyy, xx, yy );
     }
   } /* in boundary */
 }
 Otk_ReDraw_Display();
}



void Otk_Plot_Data_AutoScaled( OtkWidget plotbox, 
			       int npoints, float *xarray, float *yarray, OtkColor dcolor, 
			       OtkColor tcolor, OtkColor gcolor, int xgrids, int ygrids )
{
 float xmin=1e29, ymin=1e29, xmax=-1e29, ymax=-1e29;
 int k;

 for (k=0; k<npoints; k++)	/* Determine data ranges. */
  {
   if (xarray[k] < xmin) xmin = xarray[k];
   if (xarray[k] > xmax) xmax = xarray[k];
   if (yarray[k] < ymin) ymin = yarray[k];
   if (yarray[k] > ymax) ymax = yarray[k];
  }

 /* Expand the ranges slighty (5%) to frame the data nicely. */
 xmin = xmin - 0.05 * (xmax - xmin);
 xmax = xmax + 0.05 * (xmax - xmin);
 ymin = ymin - 0.05 * (ymax - ymin);
 ymax = ymax + 0.05 * (ymax - ymin);

 /* Set the axes accordingly and plot the data. */
 Otk_Plot_Set_Axes( plotbox, xmin, xmax, xgrids,  ymin, ymax, ygrids,  tcolor, gcolor );
 Otk_Plot_Data( plotbox, npoints, xarray, yarray, dcolor );
 Otk_ReDraw_Display();
}





struct OtkStripChartData
 {
  int npoints, xrange;
  float ymin, ymax, *ydata;
  OtkColor text_color, grid_color, data_color;
 };


void Otk_StripChart_Init( OtkWidget plotbox, int xrange, float ymin, float ymax, 
                              OtkColor text_color, OtkColor grid_color, OtkColor data_color )
{
 struct OtkStripChartData *scdata;
 float xmin, xmax;

 if (plotbox->superclass != Otk_SC_XYgraph) 
  { printf("Error: Otk_StripChart_Init called on non-XYgraph object.\n"); return; }
 scdata = (struct OtkStripChartData *)malloc(sizeof(struct OtkStripChartData));
 scdata->npoints = 0;
 scdata->xrange = xrange;
 scdata->ymin = ymin;
 scdata->ymax = ymax;
 scdata->ydata = 0;
 scdata->text_color = text_color;
 scdata->grid_color = grid_color;
 scdata->data_color = data_color;
 plotbox->callback_param = (void *)scdata;
 xmin = 0.0;
 if (xrange<=0) xmax = 10.0; else xmax = xrange;
 Otk_Plot_Set_Axes( plotbox, xmin, xmax, 4,  ymin, ymax, 4,  text_color, grid_color );
}


void Otk_StripChart_AddPoint( OtkWidget plotbox, float value )
{
 struct OtkStripChartData *scdata;
 float xmin, xmax, *xdata, *ydata;
 int k, npoints;

 if (plotbox->superclass != Otk_SC_XYgraph) 
  { printf("Error: Otk_StripChart_AddPoint called on non-XYgraph object.\n"); return; }
 if (plotbox->callback_param==0)  { printf("Error: Otk_StripChart_AddPoint called on plot with uninitialed stripchart ranges.\n"); return;}

 scdata = (struct OtkStripChartData *)(plotbox->callback_param);
 xmax = (float)(scdata->npoints + 1);
 if (scdata->xrange <= 0) xmin = 0.0;  
 else 
  {
   xmin = xmax - (float)(scdata->xrange) + 0.9;
   if (xmin < 0.0) { xmin = 0.0;  xmax = scdata->xrange + 1.0; }
  }
 xmax = xmax + 0.5;

 if (xmax<1000.0) k = 4; else k = 3;
 Otk_Plot_Set_Axes( plotbox, xmin, xmax, k,  scdata->ymin, scdata->ymax, 4,  scdata->text_color, scdata->grid_color );

 /* Now append the data record. */
 if ((scdata->xrange <= 0) || (scdata->npoints < scdata->xrange))
  {
   npoints = scdata->npoints + 1;
   xdata = (float *)malloc( npoints * sizeof(float) );
   ydata = (float *)malloc( npoints * sizeof(float) );
   if (scdata->ydata != 0)
    {
     for (k=0; k<npoints-1; k++) ydata[k] = scdata->ydata[k];
     free(scdata->ydata);
    }
   for (k=0; k<npoints; k++) xdata[k] = k + 1;
  }
 else
  {
   npoints = scdata->xrange;
   xdata = (float *)malloc( npoints * sizeof(float) );
   ydata = (float *)malloc( npoints * sizeof(float) );
   if (scdata->ydata != 0)
    {
     for (k=0; k<npoints-1; k++) ydata[k] = scdata->ydata[k+1];
     free(scdata->ydata);
    }
   for (k=0; k<npoints; k++) xdata[k] = scdata->npoints - npoints + k + 2;
  }
 ydata[npoints-1] = value;
 scdata->ydata = ydata;
 scdata->npoints = scdata->npoints + 1;

 Otk_Plot_Data( plotbox, npoints, xdata, scdata->ydata, scdata->data_color );
 free(xdata);
 Otk_ReDraw_Display();
}







OtkWidget Otk_AddIndicatorLight( OtkWidget container, float x, float y, float radius, OtkColor color )
{
 OtkWidget indlght, child;

 if (container->object_class != Otk_SC_Panel) {printf("Otk Error: Add AddIndicatorLight parent not container panel.\n"); return 0;}
 indlght = Otk_MakeDisk( container, x, y, radius, Otk_Black );
 child = Otk_MakeDisk( indlght, 0.0, 0.0, 80.0, color );
 child->object_subtype = Otk_subtype_raised;
// child->z = child->z + 0.25;
 return indlght;
}

void Otk_SetIndicatorLight( OtkWidget indlght, OtkColor newcolor )
{
 if (indlght->superclass != OTK_SC_IndicatorLight) 
 OtkTranslateColor( newcolor, indlght->children->color );
 Otk_ReDraw_Display();
}

void Otk_SetIndicatorLightBorder( OtkWidget indlght, OtkColor newcolor )
{
 if (indlght->superclass != OTK_SC_IndicatorLight) 
 OtkTranslateColor( newcolor, indlght->color );
 Otk_ReDraw_Display();
}

