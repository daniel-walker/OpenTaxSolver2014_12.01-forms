/****************************************************************/
/* Gadget_lib.h	- Header file for Otk Gadget Library, a set of  */
/* high-level widgets, such as meters, bar-graphs, gauges, and  */
/* other re-usable displays.                                    */
/*								*/
/****************************************************************/

#include "otk_lib.h"
#define Otk_SC_Gauge1   2001
#define Otk_SC_Gauge2   2002
#define Otk_SC_LEDmeter 2100
#define Otk_SC_BarMeter 2200
#define Otk_SC_XYgraph  2300
#define Otk_SC_XYaxis   2301
#define Otk_SC_XYdata   2302
#define OTK_SC_IndicatorLight 2400
extern float Otk_DZ;

OtkWidget Otk_MakeGauge2( OtkWidget container, float left, float top, float horiz_size, float vert_size, char *title );
void Otk_SetGauge2( OtkWidget container, float value, OtkColor tmpcolor );

void Otk_SetLEDmeter( OtkWidget container, float value, OtkColor offcolor, OtkColor oncolor );
void Otk_SetLEDmeter_Range( OtkWidget container, float minvalue, float maxvalue, OtkColor oncolor );
OtkWidget Otk_MakeLEDmeter( OtkWidget container, float left, float top, float horiz_size, float vert_size, 
			    int nbars, char orientation, OtkColor tmpcolor );

OtkWidget Otk_MakeBarMeter( OtkWidget container, float left, float top, float horiz_size, float vert_size, 
			    char orientation, OtkColor tmpcolor );
void Otk_SetBarMeter( OtkWidget container, float value );
void Otk_SetBarmeter_Range( OtkWidget container, int segment, float minval, float maxval, OtkColor color );

OtkWidget Otk_Plot_Init( OtkWidget container, char *xlabel, char *ylabel, float xpos, float ypos,
                         float width, float height, OtkColor fgcolor, OtkColor bgcolor );
void Otk_Plot_Set_Axes( OtkWidget plotbox, float xmin, float xmax, int xgrids,  
                                           float ymin, float ymax, int ygrids,
					   OtkColor tcolor, OtkColor gcolor );
void Otk_Plot_Data( OtkWidget plotbox, int npoints, float *xarray, float *yarray, OtkColor color );
void Otk_Plot_Data_AutoScaled( OtkWidget plotbox, 
                               int npoints, float *xarray, float *yarray, OtkColor dcolor, 
                               OtkColor tcolor, OtkColor gcolor, int xgrids, int ygrids );

void Otk_StripChart_Init( OtkWidget plotbox, int xrange, float ymin, float ymax, 
                          OtkColor text_color, OtkColor grid_color, OtkColor data_color );
void Otk_StripChart_AddPoint( OtkWidget plotbox, float value );

OtkWidget Otk_AddIndicatorLight( OtkWidget container, float x, float y, float radius, OtkColor color );
void Otk_SetIndicatorLight( OtkWidget indlght, OtkColor color );
void Otk_SetIndicatorLightBorder( OtkWidget indlght, OtkColor color );
