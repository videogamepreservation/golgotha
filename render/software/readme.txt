Software r1 implementation. uses span buffers, or draws polygons in the order
received.

if perspective mapping is enabled (via the R1_PERSPECTIVE_CORRECT flag, r1_api->modify_features)
it will automatically draw smaller polygons as affine

if any texture mapping is enabled, it will draw REALLY small polygons (<5 pixels I think) as just
a solid color. this is primarily to counteract black lines in the terrain where polygons meet, and
triangles end up being really tiny

lighting uses 2 lookup tables. theoretically, you only need to lookup 3 bytes total per pixel,
however due to a lack of registers and the slowness of mixing 16 and 32bit code, the lookups
are all 32bit values. further description of the tables is in tint_manage.cc and the simplest
lit mapper is in affine_map_lit_c.cc. All of them "dither" the lighting value, meaning that
if the lighting value on some line of pixels is 10.5, it will actually alternate between 10 and 11
when lighting those pixels.

3 types of alpha are supported.
1) solid color, half alpha. Smoke trails use this. See solid_blend_half_*cc
2) true texture alpha. the texture must be 4-4-4-4. a lookup is performed on the lower 12 bits
   to get a true 565 (or whatever the screen format is) for that pixel. the screen pixel is read
   from memory. the 2 pixels are then lit using the lighting table lookups with a properly scaled
   alpha value, added together, and stored. look at affine_map_unlit_true_alpha_c.cc
3) dithered alpha. uses the same texture format, but skips pixels based on the accumulated alpha
   value of the scanline

aside from alpha and lit texturemapping (perspective and affine), there is one other texturemapping
mode supported: "holy" texturemapping - both affine and perspective. The texture format is the
same as is used in regular texturemapping, but all texels that are black are skipped

lighting, alpha, and holiness are mutually exclusive. if you try to set a mode that isnt supported
(for instance, lit alphatextureing) the resulting output is undefined. look at update_tri_function()
in r1_software.cc. Thats what evaluates the current "state" of the renderer and chooses the
appropriate "type" of polygon that will be drawn. All types are enumerated in mappers.hh.

I *THINK* only win32 spefic code is in win32_specific.hh and win32_specific.cc. Of course,
the inline assembly in the texturemappers is not portable, and the fpu state management code
in inline_fpu.cc is not portable, but otherwise the rest of the code should be.
