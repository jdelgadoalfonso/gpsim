#ifndef __CMD_MANAGER_H__
#define __CMD_MANAGER_H__
#include <stdio.h>
#include "cmd_gpsim.h"
#include "../src/gpsim_interface.h"
#include <vector>
#include <string>
#include <functional>
using namespace std;

class CommandHandlerKey : public ICommandHandler {
public:
  CommandHandlerKey(const char *name) {
    m_name = name;
  }
  virtual char *GetName(void) {return (char*)m_name; }
  virtual int Execute(const char * commandline, ISimConsole *out) {
    return CMD_ERR_COMMANDNOTDEFINED;}
  const char * m_name;
};



class CCommandManager {
public:
  CCommandManager();
  int   Register(ICommandHandler * ch);
  int   Execute(string &sName, const char *cmdline);

  static CCommandManager m_CommandManger;
  static CCommandManager &GetManager();
  ICommandHandler * find(const char *name);
  ISimConsole &GetConsole() {
    return GetUserInterface().GetConsole();
  }

private:
  struct lessThan : binary_function<ICommandHandler*, ICommandHandler*, bool> {
    bool operator()(const ICommandHandler* left, const ICommandHandler* right) const {
      return strcmp(((ICommandHandler*)left)->GetName(),
        ((ICommandHandler*)right)->GetName()) < 0;
    }
  };

  typedef vector<ICommandHandler*> List;

  List                  m_HandlerList;
};

#endif
