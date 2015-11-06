/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef char *cell;
typedef cell matrix[4][4];

cell newcell()
{
  cell ret = (char*)malloc(80);
  ret[0] = 0;
  return ret;
}

char *add_term(char *s, char *t)
{
  cell ret;

  if (!t)
    ret = s;
  else if (!s)
    ret = strdup(t);
  else {
    ret = newcell();

    if (atoi(s) && atoi(t)) {
      sprintf(ret,"%+d",atoi(s)+atoi(t));
    }
    else {
      strcat(ret,s);
      strcat(ret,t);
    }
  }
  return ret;
}

char *mult_term(char *s, char *t)
{
  cell ret;

  if (!s || !t)
    ret = 0;
  else if (atoi(t)==1)
    ret = strdup(s);
  else if (atoi(t)==-1) {
    ret = strdup(s);
    ret[0] = ((ret[0]=='-')?'+':'-');
  }
  else if (atoi(s)==1) {
    ret = strdup(t);
  }
  else if (atoi(s)==-1) {
    ret = strdup(t);
    ret[0] = ((ret[0]=='-')?'+':'-');
  }
  else {
    ret = newcell();
    if (atoi(s) && atoi(t)) {
      sprintf(ret,"%+d",atoi(s)*atoi(t));
    }
    else {
      strcat(ret, s);
      strcat(ret, t+1);
      ret[0] = (((s[0]=='-') ^ (t[0]=='-'))?'-':'+');
    }
  }
  return ret;
}

void matmult(matrix a, matrix b, matrix result)
{
  for (int j=0; j<4; j++) 
    for (int i=0; i<4; i++) {
      cell sum = 0;
      cell prod;
      for (int k=0; k<4; k++)
	sum = add_term(sum, mult_term(a[j][k], b[k][i]));
      result[j][i] = sum;
    }
}

void matdup(matrix a,matrix b)
{
  for (int j=0; j<4; j++)
    for (int i=0; i<4; i++)
      a[j][i] = (b[j][i]?strdup(b[j][i]):0);
}

void mattranspose(matrix a,matrix dest)
{
  for (int j=0; j<4; j++)
    for (int i=0; i<4; i++)
      dest[j][i] = (a[i][j]?strdup(a[i][j]):0);
}

void matnegate(matrix a,matrix dest)
{
  for (int j=0; j<4; j++)
    for (int i=0; i<4; i++)
      dest[j][i] = mult_term(a[i][j],"-1");
}

void mat_gett(matrix a,matrix dest)
{
  for (int j=0; j<4; j++)
    for (int i=0; i<4; i++)
      if (i==j)
        dest[j][i] = strdup("+1");
      else if (i>=3)
        dest[j][i] = (a[j][i]?strdup(a[j][i]):0);
      else
        dest[j][i] = 0;
}

void mat_getrot(matrix a,matrix dest)
{
  for (int j=0; j<3; j++)
  {
    dest[3][j] = 0;
    dest[j][3] = 0;
    for (int i=0; i<3; i++)
      dest[j][i] = (a[j][i]?strdup(a[j][i]):0);
  }
  dest[3][3] = strdup("+1");
}

void print(matrix a)
{
  for (int j=0; j<4; j++)
    for (int i=0; i<4; i++)
      printf("(%d,%d) = %s\n", j+1,i+1, a[j][i]?a[j][i]:"0");
}

void printmat(matrix a)
{
  int len[4], i,j,l;
  char buf[4][20];

  for (i=0; i<4; i++)
    len[i]=1;

  for (j=0; j<4; j++)
    for (i=0; i<4; i++)
      if (a[j][i])
        if ((l=strlen(a[j][i]))>len[i])
          len[i]=l;

  for (i=0; i<4; i++)
    sprintf(buf[i]," %%-%ds ",len[i]);

  for (j=0; j<4; j++)
  {
    printf("[ ");
    for (i=0; i<4; i++)
      printf(buf[i], a[j][i]?a[j][i]:"0");
    printf(" ]\n");
  }
}

void printaff(matrix a)
{
  static char pr[]="xyzt";

  for (int i=0; i<4; i++)
    for (int j=0; j<3; j++)
      printf("aff.%c.%c = %s\n", pr[i],pr[j], a[j][i]?a[j][i]:"0");
}


matrix blank =
{ { "+", "+", "+", "+" },
  { "+", "+", "+", "+" },
  { "+", "+", "+", "+" },
  { "+", "+", "+", "+" } };

matrix v =
{ { "+v(1,1)", "+v(1,2)", "+v(1,3)", "+v(1,4)" },
  { "+v(2,1)", "+v(2,2)", "+v(2,3)", "+v(2,4)" },
  { "+v(3,1)", "+v(3,2)", "+v(3,3)", "+v(3,4)" },
  { "+v(4,1)", "+v(4,2)", "+v(4,3)", "+v(4,4)" } };

matrix aff =
{ { "+aff.x.x", "+aff.y.x", "+aff.z.x", "+aff.t.x" },
  { "+aff.x.y", "+aff.y.y", "+aff.z.y", "+aff.t.y" },
  { "+aff.x.z", "+aff.y.z", "+aff.z.z", "+aff.t.z" },
  { 0         , 0         , 0         , "+1"       } };

matrix m1 =
{ { "+1", 0,    0,    0    },
  { 0,    "+1", 0,    0    },
  { 0,    0,    "+1", 0    },
  { 0,    0,    0,    "+1" } };

matrix m0 =
{ { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 } };

matrix rotz =
{ { "+cos(z)", "-sin(z)", 0        , 0         },
  { "+sin(z)", "+cos(z)", 0        , 0         },
  { 0        , 0        , "+1"     , 0         },
  { 0        , 0        , 0        , "+1"      } };

matrix irotz =
{ { "+cos(z)", "+sin(z)", 0        , 0         },
  { "-sin(z)", "+cos(z)", 0        , 0         },
  { 0        , 0        , "+1"     , 0         },
  { 0        , 0        , 0        , "+1"      } };

matrix roty =
{ { "+cos(y)", 0        , "+sin(y)", 0         },
  { 0        , "+1"     , 0        , 0         },
  { "-sin(y)", 0        , "+cos(y)", 0         },
  { 0        , 0        , 0        , "+1"      } };

matrix iroty =
{ { "+cos(y)", 0        , "-sin(y)", 0         },
  { 0        , "+1"     , 0        , 0         },
  { "+sin(y)", 0        , "+cos(y)", 0         },
  { 0        , 0        , 0        , "+1"      } };

matrix rotx =
{ { "+1"     , 0        , 0        , 0         },
  { 0        , "+cos(x)", "-sin(x)", 0         },
  { 0        , "+sin(x)", "+cos(x)", 0         },
  { 0        , 0        , 0        , "+1"      } };

matrix irotx =
{ { "+1"     , 0        , 0        , 0         },
  { 0        , "+cos(x)", "+sin(x)", 0         },
  { 0        , "-sin(x)", "+cos(x)", 0         },
  { 0        , 0        , 0        , "+1"      } };

matrix xlat =
{ { "+1"     , 0        , 0        , "+tx"     },
  { 0        , "+1"     , 0        , "+ty"     },
  { 0        , 0        , "+1"     , "+tz"     },
  { 0        , 0        , 0        , "+1"      } };

matrix ixlat =
{ { "+1"     , 0        , 0        , "-tx"     },
  { 0        , "+1"     , 0        , "-ty"     },
  { 0        , 0        , "+1"     , "-tz"     },
  { 0        , 0        , 0        , "+1"      } };

matrix scale =
{ { "+scalex", 0        , 0        , 0         },
  { 0        , "+scaley", 0        , 0         },
  { 0        , 0        , "+scalez", 0         },
  { 0        , 0        , 0        , "+1"      } };

matrix iscale =
{ { "+(1/scalex)", 0            , 0            , 0         },
  { 0            , "+(1/scaley)", 0            , 0         },
  { 0            , 0            , "+(1/scalez)", 0         },
  { 0            , 0            , 0            , "+1"      } };

matrix cam =
{ { "+cam.x.x", "+cam.y.x", "+cam.z.x", "+cam.t.x" },
  { "+cam.x.y", "+cam.y.y", "+cam.z.y", "+cam.t.y" },
  { "+cam.x.z", "+cam.y.z", "+cam.z.z", "+cam.t.z" },
  { 0         , 0         , 0         , "+1"       } };

matrix icamt =
{ { "+1"     , 0        , 0        , "-cam.t.x"     },
  { 0        , "+1"     , 0        , "-cam.t.y"     },
  { 0        , 0        , "+1"     , "-cam.t.z"     },
  { 0        , 0        , 0        , "+1"      } };

matrix point =
{ { 0        , 0        , 0        , "+px"      },
  { 0        , 0        , 0        , "+py"      },
  { 0        , 0        , 0        , "+pz"      },
  { 0        , 0        , 0        , "+1"       } };

matrix front =
{ { 0        , 0        , 0        , "+1"       },
  { 0        , 0        , 0        , 0          },
  { 0        , 0        , 0        , 0          },
  { 0        , 0        , 0        , "+1"       } };

void main(int argc, char **argv)
{
  matrix tmp,tmp2;
  int test;

  test = (argc>1);

  if (test) {
    printf("\n1*0 = \n");
    matmult(m1,m0,tmp);
    print(tmp);
    
    printf("\n1*1 = \n");
    matmult(m1,m1,tmp);
    print(tmp);
    
    printf("\nv*0 = \n");
    matmult(v,m0,tmp);
    print(tmp);
    
    printf("\nv*1 = \n");
    matmult(v,m1,tmp);
    print(tmp);
    
    printf("\n1*v = \n");
    matmult(m1,v,tmp);
    print(tmp);

    printf("\nrotz*roty*rotx = \n");
    matmult(rotz,roty,tmp);
    matmult(tmp,rotx,tmp2);
    printaff(tmp2);
  }

  matrix test1, test2;

  matmult(aff, xlat, test1);
  printaff(test1);
  printf("\n");
  matmult(aff, rotz, test1);
  printaff(test1);
  printf("\n");
  matmult(aff, roty, test1);
  printaff(test1);
  printf("\n");
  matmult(aff, rotx, test1);
  printaff(test1);
  printf("\n");
  matmult(aff, scale, test1);
  printaff(test1);
  printf("\n");

  matrix camrot,icamrot,ixform;

  mat_getrot(cam, camrot);
  mattranspose(camrot, icamrot);

  printf("\nicamt = \n");
  printmat(icamt);
  printf("\nicamrot = \n");
  printmat(icamrot);

  matmult(icamrot,icamt,ixform);
  printf("\nixform = \n");
  printmat(ixform);

  matmult(ixform,point,tmp);
  printmat(tmp);
}

