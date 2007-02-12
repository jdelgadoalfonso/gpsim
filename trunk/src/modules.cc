/*
   Copyright (C) 1998,1999,2000 T. Scott Dattalo

This file is part of gpsim.

gpsim is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

gpsim is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with gpsim; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


#include "modules.h"

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <vector>

#include "../config.h"

#ifndef _WIN32
#include <dlfcn.h>
#if !defined(_MAX_PATH)
  #define _MAX_PATH 1024
#endif
#include <unistd.h>  // for getcwd 

#else
#include <direct.h>
#include <windows.h>
/*
 * interface is a Module class member variable in gpsim,
 * in WIN32 Platform SDK it is a macro, defined in BaseTypes.h
 * the WIN32 Platform SDK definition should be undefined
 */
#undef interface
#endif

#include "pic-processor.h"
#include "stimuli.h"
#include "symbol.h"
#include "xref.h"
#include "value.h"
#include "packages.h"
#include "cmd_manager.h"


int ModuleLibrary::m_iSequenceNumber = 0;
// ModuleLibrary *ModuleLibrary::m_pLibrary = new ModuleLibrary();
ModuleLibrary::FileList   ModuleLibrary::m_FileList;
ModuleLibrary::TypeList   ModuleLibrary::m_TypeList;
ModuleLibrary::ModuleList ModuleLibrary::m_ModuleList;


void          ModuleLibrary::LoadFile(const char *pFilename) 
{
  void *handle;
  const char *pszError;
  bool bReturn = false;

  string sPath(pFilename);
  FixupLibraryName(sPath);
  string sName;
  MakeCanonicalName(sPath, sName);
  if(!FileExists(sName)) {
    if ((handle = ::load_library(sPath.c_str(), &pszError)) == NULL) {
#ifdef THROW
      ostringstream stream;

      stream << "failed to open library module ";
      stream << sPath;
      stream << ": ";
      stream << pszError;
      stream << endl;

      char cw[_MAX_PATH];
      getcwd(cw, sizeof(cw));
      stream << "current working directory is ";
      stream << cw;
      stream << endl << ends;
      free_error_message(pszError);
      throw new Error(stream.str());
#endif //THROW
    }
    else {

      if(AddFile(sPath.c_str(),handle)) {
        bReturn = true;
      }
    }
  }

  if(verbose)
    DisplayFileList();
  return;
}

void          ModuleLibrary::FreeFile(const char *pFilename) {
  FileList::iterator  it;
  FileList::iterator  itEnd(m_FileList.end());
  for( it = m_FileList.begin(); it != itEnd; it++) {
    Module_Types *pLibModList = (*it)->get_mod_list();

    // Remove the library file's types
    for(int iIndex = 0; iIndex < Module_Types_Name_Count; iIndex++) {
      TypeList::iterator itTypeEnd = m_TypeList.end();
      TypeList::iterator itType = m_TypeList.FindIt(pLibModList->names[iIndex]);

      if(itType != itTypeEnd) {
        // Remove all instanciated objects of this type
        ModuleList::iterator  itObject;
        ModuleList::iterator  itObjectEnd(m_ModuleList.end());
        for( itObject = m_ModuleList.begin(); itObject != itObjectEnd; itObject++) {
          if( strcmp((*itObject)->m_pType->m_pName, (*itType)->m_pName) ) {
            m_ModuleList.erase(itObject);
            delete *itObject;
          }
        }
        // Remove the module type
        m_TypeList.erase(itType);
        delete *itType;
      }
    }
  }
}

Module *      ModuleLibrary::NewObject(const char *pTypeName, const char *pName) {
  Type *pType;
  ostringstream stream;
  if( pType = m_TypeList.Get(pTypeName)) {
    if(pName == NULL) {
      stream << pTypeName << m_iSequenceNumber << ends;
      pName = stream.str().c_str();
    }
    Module * pModule = pType->m_pConstructor(pName);
    if(pModule) {
      pModule->SetType(pType);
      m_ModuleList.push_back(pModule);
      globalSymbolTable().addModule(pModule);
      // Tell the gui or any modules that are interfaced to gpsim
      // that a new module has been added.
      gi.new_module(pModule);
      return pModule;
    }
  }
  return NULL;
}

void          ModuleLibrary::Delete(Module *pObject) {
  ModuleList::iterator it;
  if( (it = ::find(m_ModuleList.begin(), m_ModuleList.end(), pObject))
      != m_ModuleList.end()) {
    m_ModuleList.erase(it);
    // JRH - there should be a module_type->delete() to call
    delete *it;
  }
}

ICommandHandler * ModuleLibrary::GetCommandHandler(const char *pName) {
  File *pFile = m_FileList.Get(pName);
  if(pFile != NULL) {
    return pFile->GetCli();
  }
  return NULL;
}

void * ModuleLibrary::GetLibraryFileHandle(const char *pName) {
  File *pFile = m_FileList.Get(pName);
  if(pFile != NULL) {
    return pFile->m_pHandle;
  }
  return NULL;
}

void * ModuleLibrary::GetLibraryFunction(const char *pLibraryName,
                                         const char *pFunctionName) {
  File *pFile = m_FileList.Get(pLibraryName);
  if(pFile != NULL) {
    const char * error;
    return get_library_export(pFunctionName, pFile->m_pHandle, &error);
  }
  return NULL;
}

string        ModuleLibrary::DisplayFileList() {
  ostringstream stream;
  FileList::iterator  it;
  FileList::iterator  itEnd(m_FileList.end());
  stream << "Module Library Files\n";
  
  for( it = m_FileList.begin(); it != itEnd; it++) {
    stream << (*it)->m_pName << endl;
    Module_Types *pLibModList = (*it)->get_mod_list();
    if(pLibModList) {
      // Loop through the list and display all of the module types.
      for(Module_Types *pModTypes = pLibModList;
            pModTypes->names[0] != NULL;
            pModTypes++) {
        stream << "   " << pModTypes->names[0] << endl;
      }
    }
  }
  stream << ends;
  return string(stream.str());
}

string        ModuleLibrary::DisplayModuleTypeList() {
  ostringstream stream;
  string sDisplay;
  TypeList::iterator  it;
  TypeList::iterator  itEnd(m_TypeList.end());
  stream << "Module Types\n";
  for( it = m_TypeList.begin(); it != itEnd; it++) {
    stream << (*it)->m_pName << endl;
  }
  stream << ends;
  return string(stream.str());
}

string  ModuleLibrary::DisplayModuleList()
{
  //  return globalSymbolTable().listModules();
  return string ("FIXME -- modules.cc - DisplayModuleList");
}

string ModuleLibrary::DisplayModulePins(char *pName)
{
  ostringstream stream;
  Module * pMod = globalSymbolTable().findModule(pName);
  if(pMod == NULL) {
    stream << "module `" << pName << "' wasn't found" << endl;
  }
  else {
    for(int i=1; i<=pMod->get_pin_count(); i++) {
      stream << " Pin number " << i
            << " named " << pMod->get_pin_name(i) 
	          << " is " << ( (pMod->get_pin_state(i)>0) ? "high" : "low");
      stream << endl;
    }
  }
  stream << ends;
  return string(stream.str());
}

string        ModuleLibrary::DisplayProcessorTypeList() {
  // I'm thinking of moving the processor constructor list
  // into the ModuleLibrary since ProcessorConstructors 
  // come from module library files.
  return ProcessorConstructorList::GetList()->DisplayString();
}

ModuleLibrary::FileList & ModuleLibrary::GetFileList() {
  return m_FileList;
}

ModuleLibrary::TypeList & ModuleLibrary::GetTypeList() {
  return m_TypeList;
}

#if 0
Processor *   ModuleLibrary::NewProcessorFromFile(const char *pName) {
  iReturn = CSimulationContext::GetContext()->LoadProgram(
                pName);
  return NULL;
}

Processor *   ModuleLibrary::NewProcessorFromType(const char *pType,
                                                  const char *pName) {
  return NULL;
}

void          ModuleLibrary::DeleteProcessor(Processor *) {
}
#endif

bool ModuleLibrary::AddFile(const char *library_name, void *library_handle)
{
  const char * error;
  if(library_name) {
    string sName(library_name);
    MakeCanonicalName(sName, sName);
    File *ml = new File(sName.c_str(), library_handle);
    m_FileList.push_back(ml);
    ml->get_mod_list = (Module_Types_FPTR)get_library_export(
      "get_mod_list", ml->m_pHandle, &error);

    if (NULL == ml->get_mod_list) {
      cout << "WARNING: non-conforming module library\n"
           << "  gpsim libraries should have the get_mod_list() function defined\n";
      fputs(error, stderr);
      fputs ("\n", stderr);
      free_error_message(error);
    } else {

      // Get a pointer to the list of modules that this library file supports.
      Module_Types *pLibModList = ml->get_mod_list();

      if(pLibModList) {
        // Loop through the list and display all of the module types.
        for(Module_Types *pModTypes = pLibModList;
              pModTypes->names[0] != NULL;
              pModTypes++) {
          for(int iIndex = 0; iIndex < Module_Types_Name_Count; iIndex++) {
            char *pModName = pModTypes->names[iIndex];
            if(pModName != NULL && ! m_TypeList.Exists(pModName)) {
              m_TypeList.Add(
                new Type(pModName, pModTypes->module_constructor));
            }          
          }
        }
      }
        // If the module has an "initialize" function, then call it now.
      typedef  void * (*void_FPTR)(void);
      void * (*initialize)(void) = (void_FPTR)get_library_export(
        "initialize", ml->m_pHandle, NULL);
      if(initialize)
        initialize();

      ICommandHandler * pCliHandler = ml->GetCli();
      if (pCliHandler != NULL)
        CCommandManager::GetManager().Register(pCliHandler);
    }
    return true;
  } else {
    string sMsg("AddLibrary() called with null pointer");
    throw new Error(sMsg);
  }
  return false;
}

void ModuleLibrary::MakeCanonicalName(string &sPath, string &sName) {
#ifdef _WIN32
  sName = sPath;
#else
  GetFileName(sPath, sName);
#endif
}

bool ModuleLibrary::FileExists(const string &sName) {
  return m_FileList.Exists(sName.c_str());
#if 0
  FileList::iterator it;
  FileList::iterator itEnd = m_FileList.end();
  for(it = m_FileList.begin(); it != itEnd; it++) {
    if(sName.compare((*it)->pName) == 0) {
      return true;
    }
  }
  return false;
#endif
}


ICommandHandler *ModuleLibrary::File::GetCli() {

  PFNGETCOMMANDHANDLER pGetCli = (PFNGETCOMMANDHANDLER)get_library_export(
    GPSIM_GETCOMMANDHANDLER, m_pHandle, NULL);
  if (pGetCli != NULL)
    return pGetCli();
  return NULL;
}

Module *ModuleLibrary::TypeList::NewObject(const char *pName) {
  Type *pType = Get(pName);
  if(pType != NULL) {
    return pType->m_pConstructor(pName);
  }
  return NULL;
}





/*****************************************************************************
 *
 * Module.cc
 *
 * Here's where much of the infrastructure of gpsim is defined.
 *
 * A Module is define to be something that gpsim knows how to simulate.
 * When gpsim was originally designed, a module was simple a pic processor.
 * This concept was expanded to accomodate devices like LEDs, switches,
 * LCDs and so on. 
 */

Module::Module(const char *_name, const char *desc)
  : gpsimObject(_name, desc)
{

  package = 0;
  interface = 0;

  // Derived modules should assign more reasonable values for this.
  version = 0;

  xref = new XrefObject;

  simulation_mode = eSM_STOPPED;
  widget = 0;

  if (_name) {
    // If there is a module symbol already with this
    // name, then print a warning before deleting.

    gpsimObject *pOldModule = globalSymbolTable().find(name());
    if (pOldModule) {
      cout << "Warning: There already is a symbol in the symbol table named " << _name << endl;
      return;
    }
  }
  globalSymbolTable().addModule(this);

  // Create position attribute place holders if we're not using the gui
  if(!get_interface().bUsingGUI()) {
    addSymbol(new Float("xpos",80.0));
    addSymbol(new Float("ypos",80.0));
  }

}

Module::~Module(void)
{
  map<string ,ModuleScript *>::iterator si;

  for (si = m_scripts.begin();
       si != m_scripts.end();
       ++si) {
    ModuleScript *pMS = (*si).second;
    delete pMS;
  }
  m_scripts.clear();

  globalSymbolTable().removeModule(this);

//  instantiated_modules_list.remove(this);

  delete package;
  delete xref;
}

void Module::reset(RESET_TYPE r)
{
  cout << " resetting module " << name() << endl;
}

int Module::addSymbol(gpsimObject *pSym, string *ps_AliasedName)
{
  return mSymbolTable.addSymbol(pSym, ps_AliasedName);
}
int Module::removeSymbol(gpsimObject *pSym, bool bDeleteObject)
{
  return mSymbolTable.removeSymbol(pSym, bDeleteObject);
}
gpsimObject *Module::findSymbol(const string &searchString)
{
  return mSymbolTable.findSymbol(searchString);
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void Module::create_pkg(unsigned int number_of_pins)
{
  if(package)
    delete package;

  package = new Package(number_of_pins);

}

void Module::SetType(ModuleLibrary::Type *pType) {
  m_pType = pType;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void Module::assign_pin(unsigned int pin_number, IOPIN *pin)
{
  if(package)
    package->assign_pin(pin_number, pin);

}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
int Module::get_pin_count(void)
{
  if(package)
    return package->get_pin_count();

  return 0;

}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
string &Module::get_pin_name(unsigned int pin_number)
{
  static string invalid("");
  if(package)
    return package->get_pin_name(pin_number);
  return invalid;  //FIXME

}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
int Module::get_pin_state(unsigned int pin_number)
{
  if(package)
    return package->get_pin_state(pin_number);

  return 0;
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
IOPIN *Module::get_pin(unsigned int pin_number)
{
  if(package)
    return package->get_pin(pin_number);

  return 0;
}

//-------------------------------------------------------------------
// Module Scripts
//
// Module command line scripts are named scripts created by symbol 
// files. For example, with PIC cod files, it's possible to
// create assertions and simulation commands using the '.assert'
// and '.sim' directives. These commands are ASCII strings that
// are collected together. 
//

//-------------------------------------------------------------------
// Module::add_command
//
// Add a command line command to a Module Script.
//-------------------------------------------------------------------
void Module::add_command(string &script_name, string &command)
{
  ModuleScript *script = m_scripts[script_name];
  if (!script) {
    script = new ModuleScript(script_name);
    m_scripts[script_name] = script;
  }

  script->add_command(command);
}

//-------------------------------------------------------------------
// Module::run_script - execute a gpsim command line script
// 
//-------------------------------------------------------------------
void Module::run_script(string &script_name)
{
  ModuleScript *script = m_scripts[script_name];
  if (script) {
    ICommandHandler *pCli = CCommandManager::GetManager().find("gpsimCLI");
    if(pCli) {
      script->run(pCli);
    }
  }
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
Module::ModuleScript::ModuleScript(string &name_)
  : name(name_)
{
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
Module::ModuleScript::~ModuleScript()
{
  list <string *> :: iterator command_iterator;

  for (command_iterator = m_commands.begin();
       command_iterator != m_commands.end(); 
       ++command_iterator)
    delete *command_iterator;

  m_commands.clear();
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void Module::ModuleScript::add_command(string &command)
{
  string *new_command = new string(command);
  m_commands.push_back(new_command);
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void Module::ModuleScript::run(ICommandHandler *pCommandHandler)
{
  if (!pCommandHandler)
    return;

  pCommandHandler->ExecuteScript(m_commands,0);
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void Module::ModuleScript::concatenate(ModuleScript *pOtherScript)
{
  if (!pOtherScript)
    return;

  list <string *> :: iterator command_iterator;

  for (command_iterator = pOtherScript->m_commands.begin();
       command_iterator != pOtherScript->m_commands.end(); 
       ++command_iterator)
    m_commands.push_back(*command_iterator);
}
