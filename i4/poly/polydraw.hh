/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "poly/poly.hh"
/*

  Ussage notes :
    
g1_poly_draw is a template function for drawing a polygon. You pass in
a class which has the needed components not defined here depending on
how these components are defined you can get the different poly drawing
behavior.  Here are a few :

solid color, gouraud shaded, colored shading, texture mapped, z buffering

g1_poly_draw assumes vertices are already clipped and projected

functions need int class_with_needed_functions (modified for asm)
*/

enum 
{ 
  I4_DO_W=1,                 // do you want w value for each pixel write? (1/z)
  I4_DO_ST=2,                // do you want texture coordinates?
  I4_DO_WHITE_LIGHT=4,
  I4_DO_COLOR_LIGHT=8,
  I4_DO_PERSPECTIVE=16,
  I4_FAVOR_LARGE_POLYS=32,    // unroll loops
  I4_DO_ALPHA=64
};
       

/*

a class_with_needed_functions should have a subclass called screen_pointer with the
following members

move_to(int x, int y);
+=
next_line()
write(int s, int t, int w, float r, float g, float b);

*/

//step modifiers are only needed for values interpolated
//over Z.  Typically vertex light values and alpha are not.
template <class class_with_needed_functions>
class i4_poly_draw_gradient_class
{
  w32 order[i4_polygon_class::V_BUF_SIZE];
  sw32 top, bot, cw, ccw, y_count;
  i4_float aOneOverZ[i4_polygon_class::V_BUF_SIZE],
    aUOverZ[i4_polygon_class::V_BUF_SIZE],
    aVOverZ[i4_polygon_class::V_BUF_SIZE];
  typedef sw32 fixed28_4;
  fixed28_4 fixedX[i4_polygon_class::V_BUF_SIZE];
  fixed28_4 fixedY[i4_polygon_class::V_BUF_SIZE];

  struct edge
  { //edge stepping info
    sw32 X, XStep, Numerator, Denominator;          //DDA info
    sw32 ErrorTerm, Y, Height;                      //current y and vertical count
    i4_float r,g,b,a, rs,bs,gs,as;                  //color and alpha
    i4_float OneOverZ, OneOverZStep, OneOverZStepExtra;  //edge 1/z
    i4_float UOverZ, UOverZStep, UOverZStepExtra;    //edge u/z
    i4_float VOverZ, VOverZStep, VOverZStepExtra;    //edge v/z
    void step(class_with_needed_functions &c)
    {
      X+=XStep;  Y++;  Height--;
      if(c.feature(I4_DO_ST))
      {
        UOverZ+=UOverZStep;
        VOverZ+=VOverZStep;
      }
      OneOverZ += OneOverZStep;
      if(c.feature(I4_DO_WHITE_LIGHT)) r+=rs;
      else if(c.feature(I4_DO_COLOR_LIGHT))
      {
        r+=rs;  g+=gs;  b+=bs;
      }
      if(c.feature(I4_DO_ALPHA)) a+=as;

      ErrorTerm+=Numerator;
      if(ErrorTerm >= Denominator)
      {
        X++;
        ErrorTerm-=Denominator;
        OneOverZ+=OneOverZStepExtra;
        if(c.feature(I4_DO_ST))
        {
          UOverZ+=UOverZStepExtra;
          VOverZ+=VOverZStepExtra;
        }
      }
    }
  } left, right;

  struct gradients
  { //polygon gradients
    i4_float dOneOverZdX, dOneOverZdX8;             //normal and scaled 1/z step in polygon x
    i4_float dOneOverZdY;                           //1/z step in polygon y
    i4_float dUOverZdX, dUOverZdX8, dUOverZdY;      //normal and scaled u/z step
    i4_float dVOverZdX, dVOverZdX8, dVOverZdY;      //normal and scaled v/z step
    sw32 dUdXModifier, dVdXModifier;                //fixed 16.16 fixup
  } grads;

  //this stuff doesn't look too fast...
  fixed28_4 FloatToFixed28_4(i4_float Value) { return i4_f_to_i(Value * 16.f); }
  i4_float Fixed28_4ToFloat(fixed28_4 Value) { return Value / 16.0; }
  fixed28_4 Fixed28_4Mul(fixed28_4 A, fixed28_4 B) { return (A * B) / 16; }

  fixed28_4 Ceil28_4(fixed28_4 Value)
  {
    sw32 ReturnValue;
    sw32 Numerator = Value - 1 + 16;
    if(Numerator >= 0) ReturnValue = Numerator/16;
    else
    { // deal with negative numerators correctly
      ReturnValue = -((-Numerator)/16);
      ReturnValue -= ((-Numerator) % 16) ? 1 : 0;
    }
    return ReturnValue;
  }

  void FloorDivMod(sw32 Numerator, sw32 Denominator, sw32 &Floor, sw32 &Mod)
  {
    I4_ASSERT(Denominator > 0, "FloorDivMod d<=0");
    if(Numerator >= 0)
    { // positive case, C is okay
      Floor = Numerator / Denominator;
      Mod = Numerator % Denominator;
    }
    else
    { // Numerator is negative, do the right thing
      Floor = -((-Numerator) / Denominator);
      Mod = (-Numerator) % Denominator;
      if(Mod)
      { // there is a remainder
        Floor--;
        Mod = Denominator - Mod;
      }
    }
  }

  void sort(i4_polygon_class &poly)
  {
    int i, swap, mod;

    for (i=0 ; i<poly.t_verts; i++) order[i]=i;

    /* bubble sort the draworder corners */
    for (mod=1; mod;)
    {
      mod = 0;
      for (i=0 ; i<poly.t_verts-1 ; i++)
      {
        if (poly.vert[order[i]].py > poly.vert[order[i+1]].py)
        {
          swap = order[i];
          order[i] = order[i+1];
          order[i+1] = swap;
          mod = 1;
        }
      }
    }
  }
  
  sw32 FloatToFixed16_16(float Value) { return (sw32)(Value * 65536); }
  int next_cw(int i, int t)
  {
    i--;
    if (i<0)
      return t-1;
    else return i;     
  }

  int next_ccw(int i, int t)
  {
    i++;
    if (i==t)
      return 0;
    else return i;
  }

  void rasterize_parallelogram(i4_polygon_class &poly, class_with_needed_functions &c)
  {
    int p_starty = Ceil28_4(fixedY[top]);
    int ycount = y_count;

    enum { f_shift=16 };
    typename class_with_needed_functions::screen_pointer line_on, pixel_on;
    line_on.move_to(0, p_starty, c);
    
    while (ycount--)
    {
      pixel_on=line_on;
      pixel_on.add(left.X, c);
      if(right.X-left.X>0)
        draw_scanline_default(c, &pixel_on);
      left.step(c);
      right.step(c);
      line_on.next_line(c);
    }
  }

  void setup_edge(i4_polygon_class &poly, struct edge &edge, int end, class_with_needed_functions &c)
  {  
    i4_vertex_class *top_v=poly.vert+top;
    i4_vertex_class *end_v=poly.vert+end;

    edge.Y=Ceil28_4(fixedY[top]);
    sw32 YEnd=Ceil28_4(fixedY[end]);
    edge.Height=YEnd - edge.Y;
//    i4_float yd=(edge.Height>0)? 1.0/(i4_float)edge.Height : 1.0;

    if(edge.Height > 0)
    {
      i4_float yd=1.0/(i4_float)edge.Height;
      sw32 dN=fixedY[end] - fixedY[top];
      sw32 dM=fixedX[end] - fixedX[top];

      sw32 InitialNumerator = dM*16*edge.Y - dM*fixedY[top] +
        dN*fixedX[top] - 1 + dN*16;

      FloorDivMod(InitialNumerator,dN*16,edge.X,edge.ErrorTerm);
      FloorDivMod(dM*16,dN*16,edge.XStep,edge.Numerator);
      edge.Denominator = dN*16;

      i4_float YPrestep=Fixed28_4ToFloat(edge.Y*16 - fixedY[top]);
      i4_float XPrestep=Fixed28_4ToFloat(edge.X*16 - fixedX[top]);

      edge.OneOverZ=aOneOverZ[top] + YPrestep * grads.dOneOverZdY
        + XPrestep * grads.dOneOverZdX;
      edge.OneOverZStep=edge.XStep * grads.dOneOverZdX + grads.dOneOverZdY;
      edge.OneOverZStepExtra=grads.dOneOverZdX;

      if(c.feature(I4_DO_ST))
      {
        edge.UOverZ=aUOverZ[top] + YPrestep * grads.dUOverZdY
          + XPrestep * grads.dUOverZdX;
        edge.UOverZStep=edge.XStep * grads.dUOverZdX + grads.dUOverZdY;
        edge.UOverZStepExtra=grads.dUOverZdX;

        edge.VOverZ = aVOverZ[top] + YPrestep * grads.dVOverZdY
          + XPrestep * grads.dVOverZdX;
        edge.VOverZStep=edge.XStep * grads.dVOverZdX + grads.dVOverZdY;
        edge.VOverZStepExtra=grads.dVOverZdX;
      }
      if(c.feature(I4_DO_WHITE_LIGHT))
      {
        edge.r=top_v->r;
        edge.rs=(end_v->r - top_v->r) * yd;
      }
      else if (c.feature(I4_DO_COLOR_LIGHT))
      {
        edge.r=top_v->r;
        edge.rs=(end_v->r - top_v->r) * yd;

        edge.g=top_v->g;
        edge.gs=(end_v->g - top_v->g) * yd;

        edge.b=top_v->b;
        edge.bs=(end_v->b - top_v->b) * yd;
      }
      if (c.feature(I4_DO_ALPHA))
      {
        edge.a=top_v->a;
        edge.as=(end_v->a - top_v->a) * yd;
      }
    }
  }

public:

  i4_poly_draw_gradient_class(class_with_needed_functions &c,
                     i4_polygon_class &poly)
  {
    i4_float X1Y0=(poly.vert[1].px - poly.vert[2].px)*(poly.vert[0].py - poly.vert[2].py);
    i4_float X0Y1=(poly.vert[0].px - poly.vert[2].px)*(poly.vert[1].py - poly.vert[2].py);

    i4_float OneOverdX=1.0/(X1Y0 - X0Y1);
    i4_float OneOverdY=-OneOverdX;

    int i;

    for(i=0;i<poly.t_verts;i++)
    {
      i4_float const OneOverZ = poly.vert[i].w;
      aOneOverZ[i]=OneOverZ;
      if(c.feature(I4_DO_ST))
      {
        aUOverZ[i]=poly.vert[i].s * OneOverZ;
        aVOverZ[i]=poly.vert[i].t * OneOverZ;
      }
      fixedX[i]=FloatToFixed28_4(poly.vert[i].px);
      fixedY[i]=FloatToFixed28_4(poly.vert[i].py);
    }

    grads.dOneOverZdX=OneOverdX*(((aOneOverZ[1] - aOneOverZ[2]) *
      (poly.vert[0].py - poly.vert[2].py)) -
      ((aOneOverZ[0] - aOneOverZ[2]) * (poly.vert[1].py - poly.vert[2].py)));

    grads.dOneOverZdY=OneOverdY*(((aOneOverZ[1] - aOneOverZ[2]) *
      (poly.vert[0].px - poly.vert[2].px)) -
      ((aOneOverZ[0] - aOneOverZ[2]) * (poly.vert[1].px - poly.vert[2].px)));
    grads.dOneOverZdX8=grads.dOneOverZdX*8.0;

    if(c.feature(I4_DO_ST))
    {
      grads.dUOverZdX=OneOverdX*(((aUOverZ[1] - aUOverZ[2])*
        (poly.vert[0].py - poly.vert[2].py))-((aUOverZ[0] - aUOverZ[2])*
        (poly.vert[1].py - poly.vert[2].py)));

      grads.dUOverZdY=OneOverdY*(((aUOverZ[1] - aUOverZ[2]) *
        (poly.vert[0].px - poly.vert[2].px))-((aUOverZ[0] - aUOverZ[2])*
        (poly.vert[1].px - poly.vert[2].px)));

      grads.dVOverZdX=OneOverdX*(((aVOverZ[1] - aVOverZ[2]) *
        (poly.vert[0].py - poly.vert[2].py))-((aVOverZ[0] - aVOverZ[2])*
        (poly.vert[1].py - poly.vert[2].py)));

      grads.dVOverZdY=OneOverdY*(((aVOverZ[1] - aVOverZ[2]) *
        (poly.vert[0].px - poly.vert[2].px))-((aVOverZ[0] - aVOverZ[2])*
        (poly.vert[1].px - poly.vert[2].px)));

      grads.dUOverZdX8=grads.dUOverZdX*8.0;
      grads.dVOverZdX8=grads.dVOverZdX*8.0;
    }

    w32 const Half = 0x8000;
    w32 const PosModifier = Half;
    w32 const NegModifier = Half - 1;

    i4_float dUdXIndicator=grads.dUOverZdX*aOneOverZ[0]-aUOverZ[0]*grads.dOneOverZdX;
    if(dUdXIndicator > 0)
      grads.dUdXModifier = PosModifier;
    else if(dUdXIndicator < 0)
      grads.dUdXModifier = NegModifier;
    else
    {
      // dUdX == 0
      i4_float dUdYIndicator=grads.dUOverZdY*aOneOverZ[0]-aUOverZ[0]*grads.dOneOverZdY;
      if(dUdYIndicator >= 0)
        grads.dUdXModifier = PosModifier;
      else
        grads.dUdXModifier = NegModifier;
    }
    i4_float dVdXIndicator=grads.dVOverZdX*aOneOverZ[0]-aVOverZ[0]*grads.dOneOverZdX;
    if(dVdXIndicator > 0)
      grads.dVdXModifier = PosModifier;
    else if(dVdXIndicator < 0)
      grads.dVdXModifier = NegModifier;
    else
    {
      // dVdX == 0
      float dVdYIndicator=grads.dVOverZdY*aOneOverZ[0]-aVOverZ[0]*grads.dOneOverZdY;
      if(dVdYIndicator >= 0)
        grads.dVdXModifier = PosModifier;
      else
        grads.dVdXModifier = NegModifier;
    }
    // sort the vertices by y    
    sort(poly);

    top=order[0];
    bot=order[1];

    cw = next_cw(top, poly.t_verts);
    ccw = next_ccw(top, poly.t_verts);

    setup_edge(poly, left, ccw, c);
    setup_edge(poly, right, cw, c);

    y_count =Ceil28_4(fixedY[bot]) - Ceil28_4(fixedY[top]);
    rasterize_parallelogram(poly, c);

    for (i=1; i<poly.t_verts-1; i++)
    {
      top=order[i];
      bot=order[i+1];

      y_count =Ceil28_4(fixedY[bot]) - Ceil28_4(fixedY[top]);

      if (top==ccw)
      {
        ccw=next_ccw(top, poly.t_verts);
        setup_edge(poly, left, ccw, c);
      }
      else if (top == cw)
      {
        cw=next_cw(top, poly.t_verts);
        setup_edge(poly, right, cw, c);
      }
      rasterize_parallelogram(poly, c);
    }
  }  

  void draw_scanline_default(class_with_needed_functions &c, typename class_with_needed_functions::screen_pointer * pon)
  {
    int XStart=left.X;
    int Width=right.X-XStart;

    float wd=1.0/(float)Width;
    float r,g,b,a,w, rstep,gstep,bstep,astep,wstep;

    if(c.feature(I4_DO_WHITE_LIGHT))
    {
      r=left.r;
      rstep=(right.r-left.r)*wd;
    }
    else if(c.feature(I4_DO_COLOR_LIGHT))
    {
      rstep=(right.r-left.r)*wd;
      gstep=(right.g-left.g)*wd;
      bstep=(right.b-left.b)*wd;
    }
    if(c.feature(I4_DO_ALPHA)) astep=(right.a-left.a)*wd;
    if(c.feature(I4_DO_W))
    {
      w=left.OneOverZ;
      wstep=grads.dOneOverZdX;
    }

    float OneOverZLeft = left.OneOverZ;
    float UOverZLeft = left.UOverZ;
    float VOverZLeft = left.VOverZ;

    float dOneOverZdXAff = grads.dOneOverZdX8;
    float dUOverZdXAff = grads.dUOverZdX8;
    float dVOverZdXAff = grads.dVOverZdX8;

    float OneOverZRight = OneOverZLeft + dOneOverZdXAff;
    float UOverZRight, VOverZRight;
    if(c.feature(I4_DO_ST))
    {
      UOverZRight = UOverZLeft + dUOverZdXAff;
      VOverZRight = VOverZLeft + dVOverZdXAff;
    }

    float ZLeft = 1/OneOverZLeft, ULeft, VLeft, URight, VRight, ZRight;
    sw32 U, V, DeltaU, DeltaV;
    if(c.feature(I4_DO_ST))
    {
      ULeft = ZLeft * UOverZLeft;
      VLeft = ZLeft * VOverZLeft;
    }

    if(Width > 0)
    {
      int Subdivisions = Width>>3;
      int WidthModLength = Width % 8;

      if(!WidthModLength)
      {
        Subdivisions--;
        WidthModLength = 8;
      }

      while(Subdivisions-- > 0)
      {
        ZRight = 1/OneOverZRight;
        if(c.feature(I4_DO_ST))
        {
          URight = ZRight * UOverZRight;
          VRight = ZRight * VOverZRight;

          U = FloatToFixed16_16(ULeft) + grads.dUdXModifier;
          V = FloatToFixed16_16(VLeft) + grads.dVdXModifier;
          DeltaU = FloatToFixed16_16(URight - ULeft) / 8;
          DeltaV = FloatToFixed16_16(VRight - VLeft) / 8;
        }
        for(int Counter = 0;Counter < 8;Counter++)
        {
          sw32 UInt, VInt;
          if(c.feature(I4_DO_ST))
          {
            UInt = U>>16;
            VInt = V>>16;
          }
          pon->write(UInt, VInt, w, r, g, b, a, c);
          pon->add(1,c);

          if(c.feature(I4_DO_ST))
          {
            U += DeltaU;
            V += DeltaV;
          }
          if(c.feature(I4_DO_WHITE_LIGHT)) r += rstep;
          else if(c.feature(I4_DO_COLOR_LIGHT))
          {
            r+=rstep;  g+=gstep;  b+=bstep;
          }
          if(c.feature(I4_DO_ALPHA)) a+=astep;
        }
        ZLeft = ZRight;
        ULeft = URight;
        VLeft = VRight;

        OneOverZRight += dOneOverZdXAff;
        if(c.feature(I4_DO_ST))
        {
          UOverZRight += dUOverZdXAff;
          VOverZRight += dVOverZdXAff;
        }
      }

      if(WidthModLength)
      {
        ZRight = 1.0/(right.OneOverZ - grads.dOneOverZdX);
        if(c.feature(I4_DO_ST))
        {
          URight = ZRight * (right.UOverZ - grads.dUOverZdX);
          VRight = ZRight * (right.VOverZ - grads.dVOverZdX);

          U = FloatToFixed16_16(ULeft) + grads.dUdXModifier;
          V = FloatToFixed16_16(VLeft) + grads.dVdXModifier;
        }
        if(--WidthModLength)
        {
          // guard against div-by-0 for 1 pixel lines
          if(c.feature(I4_DO_ST))
          {
            DeltaU = FloatToFixed16_16(URight - ULeft) / WidthModLength;
            DeltaV = FloatToFixed16_16(VRight - VLeft) / WidthModLength;
          }
        }

        for(int Counter = 0;Counter <= WidthModLength;Counter++)
        {
          sw32 UInt, VInt;
          if(c.feature(I4_DO_ST))
          {
            UInt = U>>16;
            VInt = V>>16;
          }
          pon->write(UInt, VInt, w, r, g, b, a, c);
          pon->add(1,c);

          if(c.feature(I4_DO_ST))
          {
            U += DeltaU;
            V += DeltaV;
          }
          if(c.feature(I4_DO_WHITE_LIGHT)) r += rstep;
          else if(c.feature(I4_DO_COLOR_LIGHT))
          {
            r+=rstep;  g+=gstep;  b+=bstep;
          }
          if(c.feature(I4_DO_ALPHA)) a+=astep;
        }
      }
    }
  }
};

//for polygons that need no gradient calculated
template <class class_with_needed_functions>
class i4_poly_draw_class
{
  w32 order[i4_polygon_class::V_BUF_SIZE];
  sw32 top, bot, cw, ccw, y_count;
  i4_float aOneOverZ[i4_polygon_class::V_BUF_SIZE];

  typedef sw32 fixed28_4;
  fixed28_4 fixedX[i4_polygon_class::V_BUF_SIZE];
  fixed28_4 fixedY[i4_polygon_class::V_BUF_SIZE];

  struct edge
  { //edge stepping info
    sw32 X, XStep, Numerator, Denominator;          //DDA info
    sw32 ErrorTerm, Y, Height;                      //current y and vertical count
    i4_float r,g,b,a, rs,bs,gs,as;                  //color and alpha
    i4_float w,s,t, ws,ss,ts;                       //texture and w 
    void step(class_with_needed_functions &c)
    {
      X+=XStep;  Y++;  Height--;
      if(c.feature(I4_DO_ST))
      {
        s+=ss;  t+=ts;
      }
      if(c.feature(I4_DO_W)) w+=ws;
      if(c.feature(I4_DO_WHITE_LIGHT)) r+=rs;
      else if(c.feature(I4_DO_COLOR_LIGHT))
      {
        r+=rs;  g+=gs;  b+=bs;
      }
      if(c.feature(I4_DO_ALPHA)) a += as;

      ErrorTerm+=Numerator;
      if(ErrorTerm >= Denominator)
      {
        X++;
        ErrorTerm-=Denominator;
      }
    }
  } left, right;

  //this stuff doesn't look too fast...
  fixed28_4 FloatToFixed28_4(i4_float Value) { return (fixed28_4)(Value * 16); }
  i4_float Fixed28_4ToFloat(fixed28_4 Value) { return Value / 16.0; }
  fixed28_4 Fixed28_4Mul(fixed28_4 A, fixed28_4 B) { return (A * B) / 16; }

  fixed28_4 Ceil28_4(fixed28_4 Value)
  {
    sw32 ReturnValue;
    sw32 Numerator = Value - 1 + 16;
    if(Numerator >= 0) ReturnValue = Numerator/16;
    else
    { // deal with negative numerators correctly
      ReturnValue = -((-Numerator)/16);
      ReturnValue -= ((-Numerator) % 16) ? 1 : 0;
    }
    return ReturnValue;
  }

  void FloorDivMod(sw32 Numerator, sw32 Denominator, sw32 &Floor, sw32 &Mod)
  {
    I4_ASSERT(Denominator > 0, "FloorDivMod d<=0");
    if(Numerator >= 0)
    { // positive case, C is okay
      Floor = Numerator / Denominator;
      Mod = Numerator % Denominator;
    }
    else
    { // Numerator is negative, do the right thing
      Floor = -((-Numerator) / Denominator);
      Mod = (-Numerator) % Denominator;
      if(Mod)
      { // there is a remainder
        Floor--;
        Mod = Denominator - Mod;
      }
    }
  }

  void sort(i4_polygon_class &poly)
  {
    int i, swap, mod;

    for (i=0 ; i<poly.t_verts; i++) order[i]=i;

    /* bubble sort the draworder corners */
    for (mod=1; mod;)
    {
      mod = 0;
      for (i=0 ; i<poly.t_verts-1 ; i++)
      {
        if (poly.vert[order[i]].py > poly.vert[order[i+1]].py)
        {
          swap = order[i];
          order[i] = order[i+1];
          order[i+1] = swap;
          mod = 1;
        }
      }
    }
  }

  int next_cw(int i, int t)
  {
    i--;
    if (i<0)
      return t-1;
    else return i;     
  }

  int next_ccw(int i, int t)
  {
    i++;
    if (i==t)
      return 0;
    else return i;
  }

  void rasterize_parallelogram(i4_polygon_class &poly, class_with_needed_functions &c)
  {
//    int p_starty = (int)poly.vert[top].py;
    int p_starty = left.Y;
    int ycount = y_count;

    enum { f_shift=16 };
    typename class_with_needed_functions::screen_pointer line_on, pixel_on;
    line_on.move_to(0, p_starty, c);
    
    while (ycount--)
    {
      pixel_on=line_on;
      pixel_on.add(left.X, c);
      if(right.X-left.X>0)
        draw_scanline_default(c, &pixel_on);
      left.step(c);
      right.step(c);
      line_on.next_line(c);
    }
  }

  void setup_edge(i4_polygon_class &poly, struct edge &edge, int end, class_with_needed_functions &c)
  {  
    i4_vertex_class *top_v=poly.vert+top;
    i4_vertex_class *end_v=poly.vert+end;

    edge.Y=Ceil28_4(fixedY[top]);
    sw32 YEnd=Ceil28_4(fixedY[end]);
    edge.Height=YEnd - edge.Y;

    if(edge.Height > 0)
    {
      i4_float yd=1.0/(i4_float)edge.Height;

      sw32 dN=fixedY[end] - fixedY[top];
      sw32 dM=fixedX[end] - fixedX[top];

      sw32 InitialNumerator = dM*16*edge.Y - dM*fixedY[top] +
        dN*fixedX[top] - 1 + dN*16;

      FloorDivMod(InitialNumerator,dN*16,edge.X,edge.ErrorTerm);
      FloorDivMod(dM*16,dN*16,edge.XStep,edge.Numerator);
      edge.Denominator = dN*16;

      i4_float YPrestep=Fixed28_4ToFloat(edge.Y*16 - fixedY[top]);
      i4_float XPrestep=Fixed28_4ToFloat(edge.X*16 - fixedX[top]);

      if(c.feature(I4_DO_W))
      {
        edge.w=aOneOverZ[top];
        edge.ws=(aOneOverZ[end]-edge.w)*yd;
      }
      if(c.feature(I4_DO_ST))
      {
        edge.s=top_v->s;
        edge.ss=(end_v->s - top_v->s)*yd;

        edge.t=top_v->t;
        edge.ts=(end_v->t - top_v->t)*yd;
      }
      if(c.feature(I4_DO_WHITE_LIGHT))
      {
        edge.r=top_v->r;
        edge.rs=(end_v->r - top_v->r) * yd;
      }
      else if (c.feature(I4_DO_COLOR_LIGHT))
      {
        edge.r=top_v->r;
        edge.rs=(end_v->r - top_v->r) * yd;

        edge.g=top_v->g;
        edge.gs=(end_v->g - top_v->g) * yd;

        edge.b=top_v->b;
        edge.bs=(end_v->b - top_v->b) * yd;
      }
      if (c.feature(I4_DO_ALPHA))
      {
        edge.a=top_v->a;
        edge.as=(end_v->a - top_v->a) * yd;
      }
    }
  }

public:

  i4_poly_draw_class(class_with_needed_functions &c,
                     i4_polygon_class &poly)
  {
    int i;

    for(i=0;i<poly.t_verts;i++)
    {
      if(c.feature(I4_DO_W)) aOneOverZ[i]=poly.vert[i].w;
      fixedX[i]=i4_f_to_i(poly.vert[i].px * 16.f);//FloatToFixed28_4(poly.vert[i].px);
      fixedY[i]=i4_f_to_i(poly.vert[i].py * 16.f);//FloatToFixed28_4(poly.vert[i].py);
    }

    // sort the vertices by y    
    sort(poly);

    top=order[0];
    bot=order[1];

    cw = next_cw(top, poly.t_verts);
    ccw = next_ccw(top, poly.t_verts);

    setup_edge(poly, left, ccw, c);
    setup_edge(poly, right, cw, c);

    y_count =Ceil28_4(fixedY[bot]) - Ceil28_4(fixedY[top]);
    rasterize_parallelogram(poly, c);

    for (i=1; i<poly.t_verts-1; i++)
    {
      top=order[i];
      bot=order[i+1];

      y_count =Ceil28_4(fixedY[bot]) - Ceil28_4(fixedY[top]);

      if (top==ccw)
      {
        ccw=next_ccw(top, poly.t_verts);
        setup_edge(poly, left, ccw, c);
      }
      else if (top == cw)
      {
        cw=next_cw(top, poly.t_verts);
        setup_edge(poly, right, cw, c);
      }
      rasterize_parallelogram(poly, c);
    }
  }

  void draw_scanline_default(class_with_needed_functions &c, typename class_with_needed_functions::screen_pointer * pon)
  {
    int XStart=left.X;
    int Width=right.X-XStart;

    float wd=1.0/(float)Width;
    float r,g,b,a,w,s,t, rstep,gstep,bstep,astep,wstep,sstep,tstep;

    if(c.feature(I4_DO_WHITE_LIGHT))
    {
      r=left.r;
      rstep=(right.r-left.r)*wd;
    }
    else if(c.feature(I4_DO_COLOR_LIGHT))
    {
      r=left.r;  b=left.b;  g=left.g;
      rstep=(right.r-left.r)*wd;
      gstep=(right.g-left.g)*wd;
      bstep=(right.b-left.b)*wd;
    }
    if(c.feature(I4_DO_ALPHA))
    {
      a=left.a;
      astep=(right.a-left.a)*wd;
    }
    if(c.feature(I4_DO_W))
    {
      w=left.w;
      wstep=(right.w-left.w)*wd;
    }
    if(c.feature(I4_DO_ST))
    {
      s=left.s;
      sstep=(right.s-s)*wd;
      t=left.t;
      tstep=(right.t-t)*wd;
    }
    if(Width > 0)
    {
      while(Width--)
      {
        pon->write((int)s, (int)t, w, r, g, b, a, c);
        pon->add(1,c);

        if(c.feature(I4_DO_ST))
        {
          s+=sstep;
          t+=tstep;
        }

        if(c.feature(I4_DO_W)) w+=wstep;
        if(c.feature(I4_DO_WHITE_LIGHT)) r+=rstep;
        else if(c.feature(I4_DO_COLOR_LIGHT))
        {
          r+=rstep;  g+=gstep;  b+=bstep;
        }
        if(c.feature(I4_DO_ALPHA)) a+=astep;
      }
    }
  }
};
