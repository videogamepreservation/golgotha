/********************************************************************** <BR>
  This file is part of Crack dot Com's free source code release of
  Golgotha. <a href="http://www.crack.com/golgotha_release"> <BR> for
  information about compiling & licensing issues visit this URL</a> 
  <PRE> If that doesn't help, contact Jonathan Clark at 
  golgotha_source@usa.net (Subject should have "GOLG" in it) 
***********************************************************************/

#ifndef TEST_HH
#define TEST_HH

#include <stdio.h>

class CStart
// Startup code test
{
public:
  CStart(char *name)
  {
    printf("%s loaded successfully\n",name);
  }
};

class CBase
// Base class for virtual functions & object members testing
{
protected:
  virtual char *Type() = 0;
public:
  char *name;

  CBase(char *_name) : name(_name) {}

  void print() 
  {
    printf("I'm [%s]:[%s].\n", Type(), name);
  }

  virtual void Action(char *command) = 0;
};

class CFoundry;
extern CFoundry *Foundry;
class CFoundry
{
private:
  friend class CBaseFoundry;
  CFoundry *next, *prev;

  CFoundry(int dummy) 
  {
    next = this;
    prev = this;
  }

public:
  CFoundry()
  {
    prev = Foundry;
    next = Foundry->next;
    Foundry->next->prev = this;
    Foundry->next = this;
    printf("Foundry registered.\n");
  }

  ~CFoundry()
  {
    next->prev = prev;
    prev->next = next;
    printf("Foundry unlinked.\n");
  }

  virtual char *Type() = 0;
  virtual CBase *Make(char *name) = 0;
};

typedef void (*test_func)(int a, int b);

extern CBase *Make(char *type, char *name);

#endif
