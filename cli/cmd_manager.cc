#include "cmd_manager.h"
#include <strstream>
#include <algorithm>

//
//  CGpsimConsole
//  Connector between the gpsim console and the
//  console handler for the loaded modules.
//////////////////////////////////////////////////

CGpsimConsole::CGpsimConsole(FILE*) {
}

void CGpsimConsole::Printf(const char *fmt, ...) {
  va_list ap;

  va_start(ap,fmt);
  vfprintf(m_pfOut, fmt, ap);
  va_end(ap);
}

void CGpsimConsole::VPrintf(const char *fmt, va_list argptr) {
  vfprintf(m_pfOut, fmt, argptr);
}

void CGpsimConsole::Puts(const char*s) {
  fputs(s, m_pfOut);
}

void CGpsimConsole::Putc(const char c) {
  fputc(c, m_pfOut);
}

char* CGpsimConsole::Gets(char *s, int size) {
  return fgets(s, size, m_pfIn);
}

void CGpsimConsole::SetOut(FILE *pOut) {
  m_pfOut = pOut;
}

void CGpsimConsole::SetIn(FILE *pIn) {
  m_pfIn = pIn;
}

//
//  CCommandManager
//////////////////////////////////////////////////

CCommandManager::CCommandManager(FILE *out, FILE *in) {
  m_Console.SetOut(out);
  m_Console.SetIn(in);
}

void CCommandManager::SetFileStream(FILE *out) {
  m_Console.SetOut(out);
}

int CCommandManager::Execute(string &sName, const char *cmdline) {
  ICommandHandler *handler = find(sName.c_str());
  if (handler != NULL) {
    SetFileStream(stdout);
    return handler->Execute(cmdline, &m_Console);
  }
  return CMD_ERR_PROCESSORNOTDEFINED;
}

int CCommandManager::Register(ICommandHandler * ch) {
  List::iterator it = lower_bound(m_HandlerList.begin( ), m_HandlerList.end( ),
    ch, lessThan());
  if (it != m_HandlerList.end() &&
    strcmp((*it)->GetName(), ch->GetName()) == 0) {
    return CMD_ERR_PROCESSORDEFINED;
  }
  m_HandlerList.insert(it, ch);
  return CMD_ERR_OK;
}

ICommandHandler * CCommandManager::find(const char *name) {
  CommandHandlerKey key(name);
  List::iterator it = lower_bound(m_HandlerList.begin( ), m_HandlerList.end( ),
    (ICommandHandler*)&key, lessThan());
  if (it != m_HandlerList.end() &&
    strcmp((*it)->GetName(), name) == 0) {
    return *it;
  }
  return NULL;
}

CCommandManager CCommandManager::m_CommandManger;

CCommandManager &CCommandManager::GetManager() {
  return m_CommandManger;
}


