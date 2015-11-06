/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

typedef sw32 f16_16; //fixed point 16:16
typedef sw32 f28_4;  //fixed point 28:4


//if you change the struct below, adjust this to reflect
//the proper offset
#define TRI_AREA_STRUCT_AREA 16 

//stores the area of a triangle
struct tri_area_struct
{
  float dx1x0; //tri.x1 - tri.x0
  float dy1y0; //tri.y1 - tri.y0
  float dx2x0; //tri.x2 - tri.x0
  float dy2y0; //tri.y2 - tri.y0
  float area;  //(float)((dx2x0 * dy1y0) - (dx1x0) * dy2y0));
  
  //calculates the area. uses asm

  void  calc_area(sw32 x0, sw32 x1, sw32 x2,
                  sw32 y0, sw32 y1, sw32 y2); 
  
  void  calc_area(float x0, float x1, float x2,
                  float y0, float y1, float y2); 
};

//the rendering vertex structure used by most/all software routines
struct s_vert //needs to be 4 byte aligned
{
  float px,py;   //screen x, screen y (floats)
  sw32  ix,iy; //screen x, screen y (truncated ints)
  w32   color; //the lower 16-bits are the color, the upper 16-bits might also be a duplicate
  f28_4 l;     //the lighting value (fixed 24.8 interpolated)
  float ooz;   //1/z value
  float s,soz; //s value, s/z value
  float t,toz; //t value, t/z value
  i4_bool st_projected; //TRUE if soz and toz have been calculated (based on s, t, and z)
  w8 pad1;
  w8 pad2;
  w8 pad3;
};

//used for simple rasterization of triangles
struct tri_edge
{
  f16_16 px;         //fixed point representation of screen x
  
  float  dxdy;       //change in x per screen y
  f16_16 dxdy_fixed; //i4_f_to_i(dxdy * 65536.f)

  sw32   dy;         //total change in y
};

//simple primitive used for drawing lines
typedef struct
{
  w16 start_color,end_color;
  sw16 x0,x1,y0,y1;
} software_line;


//gradients used for texturemapping a triangle either affine or perspective correct
//(hence the use of unions)
struct tri_gradients //needs to be 8 byte aligned
{
  //DO NOT REORDER THESE

  // 1/z gradient -- convert back to floats?
  float doozdx;     //change in 1/z per screen x
  float doozdy;     //change in 1/z per screen y

  //t gradient
  union
  {
    float dtozdx; //change in t/z per screen x
    float dtdx;   //change in t   per screen x
  };

  union
  {
    float dtozdy; //change in t/z per screen y
    float dtdy;   //change in t   per screen y
  };

  //s gradient
  union
  {
    float dsozdx;     //change in s/z per x
    float dsdx;       //change in s   per x
  };

  union
  {
    float dsozdy;      //change in s/z per screen y
    float dsdy;        //change in s   per screen y
  };

  union
  {
    float dsozdxspan; //change in s/z per 8 pixel x span
    float dsdxspan;   //change in s   per 8 pixel x span
  };

  union
  {
    float dtozdxspan; //change in t/z per 8 pixel span 
    float dtdxspan;   //change in t   per 8 pixel span
  };

  //lighting gradient  
  float dldx;  //change in lighting value per screen x
  float dldy;  //change in lighting value per screen y

  union
  {
    float sozat00;  //this tri's soz value at screen point 0,0
    float sat00;    //this tri's s   value at screen point 0,0
  };
  
  union
  {
    float tozat00; //this tri's toz value at screen position (0,0)
    float tat00;   //this tri's t   value at screen position (0,0)
  };

  //adjustment data (needed to ensure we dont step outside the texture boundary)
  f16_16 s_adjust;
  f16_16 t_adjust;

  float doozdxspan; //change in 1/z per 8 pixel span
  float oozat00;    //this tri's ooz value at screen position (0,0)

  float lat00; //this tri's lighting value at screen position (0,0)
  w32   pad; //ensures that the structure itself is 8-byte aligned, total of 72 bytes
};

//data passed to each scanline texturemapper
//(the starting values for the indicated components)
//other pertinant information is also passed (the width, the x offset)
//in the function call, and textureing gradient information
//is copied into a global variable

//if you change these structures you must change the #defines below to correspond
//with the correct offsets
typedef struct //needs to be 8 byte aligned
{
  f16_16 s,t,l;
  float ooz;
} affine_span;

#define AFFINE_SPAN_S 0
#define AFFINE_SPAN_T 4
#define AFFINE_SPAN_L 8

typedef struct //needs to be 8 byte aligned
{
  float soz,toz,ooz;
  f16_16 l;  
} perspective_span;

typedef struct
{
  float ooz;
  w32   color;
  f28_4 alpha;
} solid_blend_span;

//lighting table defines
#define NUM_LIGHT_SHADES  (31)
#define NUM_LIGHT_SHADES_LOG2 (5)
#define ctable_size       (256*(NUM_LIGHT_SHADES+1))
#define ctable_size_bytes (ctable_size*4)

//the lighting lookup table structure
struct software_color_table
{
  w32 table[ctable_size*2];
  w32 *low_lookups()  { return table; }
  w32 *high_lookups() { return &table[ctable_size]; }
};

extern software_color_table software_color_tables[];
