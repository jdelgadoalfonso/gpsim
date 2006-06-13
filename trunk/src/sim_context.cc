/*
   Copyright (C) 1998 T. Scott Dattalo

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
along with gpasm; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <stdio.h>
#ifdef _WIN32
#include "uxtime.h"
#endif
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <time.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>

#ifndef _WIN32
#if !defined(_MAX_PATH)
  #define _MAX_PATH 1024
#endif
#include <unistd.h>
#else
#include <direct.h>
#endif


#include "../config.h"

#include "errors.h"
#include "fopen-path.h"
#include "program_files.h"
#include "sim_context.h"
#include "breakpoints.h"
#include "trace.h"

//================================================================================
// Global Declarations
//  FIXME -  move these global references somewhere else

// don't print out a bunch of garbage while initializing


//================================================================================
//
// pic_processor
//
// This file contains all (most?) of the code that simulates those features 
// common to all pic microcontrollers.
//
//

CSimulationContext::CSimulationContext() :
  m_bEnableLoadSource(*new Boolean("EnableSourceLoad", true,
    "Enables and disables loading of source code")) {
  active_cpu_id = 0;
  cpu_ids = 0;
  m_bEnableLoadSource.setClearableSymbol(false);
  m_pbUserCanceled = NULL;
}

void CSimulationContext::Initialize() {
  get_symbol_table().add(&m_bEnableLoadSource);
}

CSimulationContext *CSimulationContext::s_SimulationContext = new CSimulationContext();

CSimulationContext *CSimulationContext::GetContext() {
  return s_SimulationContext;
}

bool CSimulationContext::SetDefaultProcessor(const char * processor_type,
                                             const char * processor_new_name) 
{
  if (processor_type) {
    ProcessorConstructor *pc = ProcessorConstructorList::GetList()->findByType(processor_type);

    if (pc) {
      m_DefProcessorName    = processor_type;
      if(processor_new_name == NULL)
	m_DefProcessorNameNew.clear();
      else
	m_DefProcessorNameNew = processor_new_name;
      return true;
    }
  } else {

    m_DefProcessorNameNew = processor_new_name;

  }

  return false;
}

//-------------------------------------------------------------------
Processor * CSimulationContext::SetProcessorByType(const char * processor_type,
                                                   const char * processor_new_name)
{
  Processor *p;
  CProcessorList::iterator it = processor_list.findByType(string(processor_type));
  GetBreakpoints().clear_all(GetActiveCPU());
  GetSymbolTable().Reinitialize();
  if(processor_list.end() == it) {
    p = add_processor(processor_type,processor_new_name);
  }
  else {
    p = it->second;
    delete p;
    p = add_processor(processor_type,processor_new_name);
//    p->init
  }
  return p;
}

//-------------------------------------------------------------------
Processor * CSimulationContext::add_processor(const char * processor_type,
                                              const char * processor_new_name)
{
  if(verbose)
    cout << "Trying to add new processor '" << processor_type << "' named '" 
	 << processor_new_name << "'\n";

  ProcessorConstructor *pc = ProcessorConstructorList::GetList()->findByType(processor_type);
  if(pc) {
    return add_processor(pc,processor_new_name ? processor_new_name : m_DefProcessorNameNew.c_str());
  } else
    cout << processor_type << " is not a valid processor.\n"
      "(try 'processor list' to see a list of valid processors.\n";
  return 0;
}

Processor * CSimulationContext::add_processor(ProcessorConstructor *pc,
					      const char * processor_new_name)
{
  Processor *  p = pc->ConstructProcessor(processor_new_name);
  if(p) {
    add_processor(p);
    p->m_pConstructorObject = pc;
  }
  else
    cout << " unable to add a processor (BUG?)\n";
  return p;
}

Processor * CSimulationContext::add_processor(Processor *p)
{
    processor_list.insert(CProcessorList::value_type(p->name(), p));
    p->initializeAttributes();
    active_cpu = p;
    //p->processor_id = 
    active_cpu_id = ++cpu_ids;
    if(verbose) {
      cout << p->name() << '\n';
      cout << "Program Memory size " <<  p->program_memory_size() << '\n';
      cout << "Register Memory size " <<  p->register_memory_size() << '\n';
    }

    trace.switch_cpus(p);
    // Tell the gui or any modules that are interfaced to gpsim
    // that a new processor has been declared.
    gi.new_processor(p);

    return p;

  return 0;
}

int CSimulationContext::LoadProgram(const char *filename,
                                    const char *pProcessorType,
                                    Processor **ppProcessor,
				    const char *pProcessorName)
{
  bool bReturn = false;
  Processor *pProcessor;
  FILE * pFile = fopen_path (filename, "rb");
  if(pFile == NULL) {
    char cw[_MAX_PATH];

    perror((string("failed to open program file ") + filename).c_str());
    getcwd(cw, sizeof(cw));
    cerr << "current working directory is ";
    cerr << cw;
    cerr << endl;
    return false;
  }
  if(pProcessorType != NULL) {
    pProcessor = SetProcessorByType(pProcessorType, NULL);
    if(pProcessor != NULL) {
      bReturn  = pProcessor->LoadProgramFile(filename, pFile, pProcessorName);
    }
  }
  else if(!m_DefProcessorName.empty()) {
    pProcessor = SetProcessorByType(m_DefProcessorName.c_str(), NULL);
    if(pProcessor != NULL) {
      bReturn  = pProcessor->LoadProgramFile(filename, pFile, pProcessorName);
    }
  }
  else {
    pProcessor = NULL;
    if (!m_DefProcessorNameNew.empty())
      pProcessorName = m_DefProcessorNameNew.c_str();
    // use processor defined in program file
    bReturn  = ProgramFileTypeList::GetList().LoadProgramFile(
			   &pProcessor, filename, pFile, pProcessorName);

  }

  fclose(pFile);
  if(bReturn) {
    // Tell all of the interfaces that a new program exists.
    gi.new_program(pProcessor);
  }
  if(ppProcessor != NULL) {
    *ppProcessor = pProcessor;
  }
  return bReturn;
}

//------------------------------------------------------------------------
// dump_processor_list - print out all of the processors a user is 
//                       simulating.

void CSimulationContext::dump_processor_list(void)
{

  cout << "Processor List\n";

  bool have_processors = 0;
  CProcessorList::iterator processor_iterator; 
  for (processor_iterator = processor_list.begin();
       processor_iterator != processor_list.end(); 
       processor_iterator++) {
      CProcessorList::value_type vt = *processor_iterator;
      Processor *p = vt.second;
      cout << p->name() << '\n';
      have_processors = 1;
    }

  if(!have_processors)
    cout << "(empty)\n";

}

void CSimulationContext::Clear() {
  GetBreakpoints().clear_all(GetActiveCPU());
  CProcessorList::iterator processor_iterator; 
  for (processor_iterator = processor_list.begin();
       processor_iterator != processor_list.end(); 
       processor_iterator++) {
      CProcessorList::value_type vt = *processor_iterator;
      Processor *p = vt.second;
      delete p;
    }
  GetSymbolTable().clear_all();
  processor_list.clear();
}

void CSimulationContext::Reset(RESET_TYPE r) {
  Symbol_Table &ST = get_symbol_table();
  Symbol_Table::module_symbol_iterator it;
  Symbol_Table::module_symbol_iterator itEnd = ST.endModuleSymbol();
  for(it = ST.beginModuleSymbol(); it != itEnd; it++) {
      Module *m = (*it)->get_module();
      if(m) {
        m->reset(r);
      }
  }
}

void CSimulationContext::NotifyUserCanceled() {
  if(m_pbUserCanceled != NULL) {
    *m_pbUserCanceled = true;
    m_pbUserCanceled = NULL;
    return;
  }
  if(CSimulationContext::GetContext()->GetActiveCPU()->simulation_mode
    == eSM_RUNNING) {
    // If we get a CTRL->C while processing a command file
    // we should probably stop the command file processing.
    CSimulationContext::GetContext()->GetBreakpoints().halt();
  }
}

extern Symbol_Table symbol_table;  // There's only one instance of "the" symbol table
Symbol_Table & CSimulationContext::GetSymbolTable() {
  return symbol_table;
}

Breakpoints & CSimulationContext::GetBreakpoints() {
  return get_bp();
}

Processor * CSimulationContext::GetActiveCPU() {
  return get_active_cpu();
}

CSimulationContext::CProcessorList::iterator
CSimulationContext::CProcessorList::findByType(const key_type& _Keyval) {
  // First find a ProcessorConstructor that matches the
  // processor type we are looking for. This should handle
  // naming variations.
  ProcessorConstructorList * pcl = ProcessorConstructorList::GetList();
  ProcessorConstructor * pc = pcl->findByType(_Keyval.c_str());
  if(pc == NULL)
    return end();
  // Now find the specific allocated processor that
  // was created with the ProcessorConstructor object 
  // we found above.
  iterator it;
  iterator itEnd = end();
  for(it = begin(); it != itEnd; it++) {
    if(it->second->m_pConstructorObject == pc) {
      return it;
    }
  }
  return itEnd;
}
