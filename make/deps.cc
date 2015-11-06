/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


enum { MAX_SRC=10000 };

name_table src_files;
name_table src_deps[MAX_SRC];

int find_src(char *filename)
{
  for (int i=0; i<src_files.size(); i++)
  {
    if (strcmp(src_files[i],filename)==0)
      return i;
  }
  return -1;
}

name_table *get_deps(char *filename, name_table *includes)
{
  int f=find_src(filename);
  if (f!=-1)
    return &src_deps[f];
  else
  {
    FILE *fp=fopen(filename,"rb");
    if (!fp)
      return 0;
    fseek(fp, 0, SEEK_END);
    int size=ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *mem=(char *)malloc(size+1);
    fread(mem, 1, size, fp);
    fclose(fp);
    mem[size]=0;


    int x = src_files.add(strdup(filename));
    src_deps[x].add(strdup(filename));
    
    char *p=mem, name2[100], *s;
    
    while (*p)
    {
      if (p[0]=='#' && 
          p[1]=='i' && 
          p[2]=='n' && 
          p[3]=='c' && 
          p[4]=='l' && 
          p[5]=='u' && 
          p[6]=='d' && 
          p[7]=='e' &&
          p[8]==' ')
      {
        p+=9;

        while (*p==' ') p++;
        if (*p=='"')
        {
          p++; s=p;
          while (*p && *p!='"') 
            p++;

          if (*p)
          {
            *p=0;         
            p++;
          }

          name_table *ret=get_deps(s, includes);

          for (int j=0; !ret && j<includes->size(); j++)
          {
            sprintf(name2,"%s/%s",(*includes)[j],s);
            ret=get_deps(name2, includes);
          }

          if (ret)
          {
            for (int i=0;i<ret->size(); i++)
            {
              int found=0;
              for (int j=0; j<src_deps[x].size(); j++)
                if (strcmp(src_deps[x][j],(*ret)[i])==0)
                  found=1;
              if (!found)
                src_deps[x].add((*ret)[i]);
            }
          } else
          {
            // ignore files with a '~' because they are for name mangled 8.3 systems
            int t=0;
            for (char *q=s; *q; q++)
              if (*q=='~') t=1;              
            if (!t)
              fprintf(stderr,"%s : couldn't find include %s\n",filename,s);
          }
        }
      }
      else p++;
    }

    free(mem);

    return src_deps+x;
  }
}
