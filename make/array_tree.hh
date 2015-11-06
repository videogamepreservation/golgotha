/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/


template <class T, int MAX_SIZE>
class i4_array_tree
{
  int used;
  T nodes[MAX_SIZE];

public:

  i4_array_tree()
  {
    used=0;
  }

  void add(T &x)
  {
    int i=0;

    if (used+1==MAX_SIZE)
      mk_error("array_tree full");
    else if (used)
    {
      int parent=0, found=0;

      while (i!=-1)
      {
        parent=i;
        if (x<nodes[i])
          i=nodes[i].left;
        else if (x>nodes[i])
          i=nodes[i].right;
        else
          return;      // already in tree               
      }

      i=used;

      if (x<nodes[parent])
        nodes[parent].left=i;
      else
        nodes[parent].right=i;
      

    }

    nodes[i]=x;
    nodes[i].left=-1;
    nodes[i].right=-1;    
    used++;
  }


  int find(T &x)
  {
    if (!used) return -1;
    for (int i=0; ;)
    {
      if (x<nodes[i])
        i=nodes[i].left;
      else if (x>nodes[i])
        i=nodes[i].right;
      else return i;
      if (i==-1)
        return -1;
    }    
  }

  T &get(int x) { return nodes[x]; }

};
