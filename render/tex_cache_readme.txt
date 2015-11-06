Texture cache readme file
-------------------------

The texture management works as follows. I'll highlight the changes from Jonathans original implementation.

Each texturemanager has a heap manager (or a non-heap manager). I've implemented each in
tex_heap.hh and tex_no_heap.hh. tex_heap is used when you have actual access to the texture memory
chunk, as with the software or Glide implementation of R1, however with something like Direct3D or
OpenGL, you dont, so the non-heap manager will keep track of those textures, how much memory they're using,
how old they are, etc.

Aside from the heap manager, each texture manager has to implement an immediate_mip_load() and async_mip_load().
What each does should be obvious from the name. The way I have the async loads handled currently is that
the reads are started during the frame, but the upload isnt processed until next_frame() is called. Most
cards wont like it if you upload in mid-frame, so if you DO find yourself wanting to call immediate_mip_load(),
make sure its not in the middle of the frame.

Aside from that, the texture managers dont do much other than "select" a texture. The reason the "select"
function exists is so the heap manager can keep track of how old textures are, but not much else. It could
do other things if the texture manager wants to implement a 2d tile system of combining textures, or whatever,
but it currently does nothing of the sort.

As far as the texture file system goes, its a bit complicated.

There is a texture directory (a physical directory, c:\tmp\ctext)
and a file inside of it called the "cache" file (c:\tmp\ctext\tex_cache.dat).
The cache knows what pixel format the textures in the directory are in, the
maximum size of the textures, and it also contains their lowest mip levels.
The reason it contains the lowest levels is to speed up the game startup, but
I've noticed its rather useless because the game still has to chunk on the
higher miplevels that it needs when you actually start drawing scenes.
