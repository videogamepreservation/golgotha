/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "test.hh"

CStart startup("Test2");

class CTest2 : public CBase
{
protected:
  virtual char *Type() { return "Test2"; }
public:
  int accesses;
  CBase *sub;

  CTest2(char *name) : CBase(name), accesses(0), sub(0) {}

  ~CTest2()
  {
    if (sub)
      delete sub;
    sub = 0;
  }

  virtual void Action(char *command)
  {
    accesses++;
    switch (command[0]) {
    case 'c':
      if (sub)
	delete sub;
      sub = Make("Test1","subobject");
      break;
    case 's':
      sub->Action(command+2);
      break;
    case 'p':
      print();
      break;
    case 'a':
      printf("I've been accessed %d times.\n", accesses);
      break;
    }
  }
};

class CTest2Foundry : public CFoundry
{
public:
  char *Type() { return "Test2"; }
  CBase *Make(char *name) { return new CTest2(name); }
} CTest2Foundry;

extern "C" void test2(int a, int b)
{
  printf("Test 2 succesfully executed with %d and %d\n",a,b);
}
