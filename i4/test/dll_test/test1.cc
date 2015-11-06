/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#include "test.hh"

CStart startup("Test1");

class CTest1 : public CBase
{
protected:
  virtual char *Type() { return "Test1"; }
public:
  char Message[80];

  CTest1(char *name) : CBase(name) { strcpy(Message, "Init"); }

  virtual void Action(char *command)
  {
    switch (command[0]) {
    case 'p':
      print();
      break;
    case 'a':
      printf("You pressed my A!\n");
      break;
    case 'm':
      printf("My message to you '%s'!\n", Message);
      break;
    }
  }
};

class CTest1Foundry : public CFoundry
{
public:
  char *Type() { return "Test1"; }
  CBase *Make(char *name) { return new CTest1(name); }
} CTest1Foundry;

extern "C" void test1(int a, int b)
{
  printf("Test 1 succesfully executed with %d and %d\n",a,b);
}
