#include <stdio.h>

main()
{
  int c,x=0;
  fprintf(stdout,"unsigned char resource_buffer[]={\n");
  while (!feof(stdin))
  {
    c=fgetc(stdin);
    if (!feof(stdin))
    {
      if (x==0)
        fprintf(stdout,"  0x%x",c);
      else if ((x%15==0))
        fprintf(stdout,",\n  0x%x",c);
      else
        fprintf(stdout,",0x%x",c);
      x++;
    }
  }
  fprintf(stdout,"};\n");

}
