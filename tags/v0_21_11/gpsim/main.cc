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
along with gpsim; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */



// Portions of this files are (C) by Ian King:

/* picdis.c  - pic disassembler         */
/* version 0.1                          */
/* (c) I.King 1994                      */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <iomanip>

using namespace std;

#include "../config.h"
#include "../cli/input.h"
#include "../src/interface.h"
#include "../src/gpsim_interface.h"
#include "../src/fopen-path.h"
#include "../src/breakpoints.h"
#include "../cli/ui_gpsim.h"

int quit_state;

extern "C" {
#include <popt.h>
}

extern int gui_init (int argc, char **argv);
extern void gui_main(void);
extern void cli_main(void);
// os_dependent.cc'
extern void AddModulePathFromFilePath(string &sFolder);

void initialize_gpsim(void);


int parse_string(char *cmd_string);
extern void initialize_commands();

extern int yydebug;
extern int quit_parse;
extern int abort_gpsim;

#if _DEBUG
  char szBuild[] = "Debug";
#else
  char szBuild[] = "Release";
#endif

void gpsim_version(void)
{
  printf("%s %s\n", szBuild, VERSION);
}

// from ui_gpsim.cc
void initialize_ConsoleUI();

#define FILE_STRING_LENGTH 50

static char *startup_name = "";
static char *processor_name = "";
static char *cod_name     = "";
static char *hex_name     = "";
static char *search_path  = "";
static char *icd_port     = "";
static char *defineSymbol = "";
static char *sExitOn      = "";
static char *sourceEnabled = "";

#define POPT_MYEXAMPLES { NULL, '\0', POPT_ARG_INCLUDE_TABLE, poptHelpOptions, \
			0, "Examples:\n\
  gpsim -s myprog.cod          <-- loads a symbol file\n\
  gpsim -p p16f877 myprog.hex  <-- select processor and load hex\n\
  gpsim -c myscript.stc        <-- loads a script\n\
\nHelp options:", NULL },

struct poptOption optionsTable[] = {
  //  { "help", 'h', 0, 0, 'h',
  //    "this help list" },
  { "processor", 'p', POPT_ARG_STRING, &processor_name, 0,
    "processor (e.g. -pp16c84 for the 'c84)","<processor name>" },
  { "command",   'c', POPT_ARG_STRING, &startup_name, 0,
    "startup command file",0 },
  { "symbol",    's', POPT_ARG_STRING, &cod_name, 0,
    ".cod symbol file",0 } ,
  { "sourcepath", 'L',POPT_ARG_STRING, &search_path, 'L',
    "colon separated list of directories to search.", 0},
  { "version",'v',0,0,'v',
    "gpsim version",0},
  { "echo",'E',POPT_ARG_NONE,0,'E',
    "Echo lines from a command file to the console.",0},
  { "cli",'i',POPT_ARG_NONE,0,'i',
    "command line mode only",0},
  { "source",'S',POPT_ARG_STRING,&sourceEnabled,'S',
    "'enable' or 'disable' the loading of source code. Default is 'enable'. "
    "Useful for running faster regression tests.",0},
  { "icd", 'd',POPT_ARG_STRING, &icd_port, 0,
    "use ICD (e.g. -d /dev/ttyS0).",0 },
  { "define",'D',POPT_ARG_STRING, &defineSymbol,'D',
    "define symbol with value that is added to the gpsim symbol table. "
    "Define any number of symbols.",0},
  { "exit", 'e',POPT_ARG_STRING, &sExitOn, 'e',
    "Causes gpsim to auto exit on a condition. Specifying onbreak will cause "
    "gpsim to exit when the simulation halts, but not until after the current "
    "command script completes.",0 },
  POPT_MYEXAMPLES
  POPT_TABLEEND
};

// copied the format of this from the popt.h include file:


void 
helpme (char *iam)
{
  printf ("\n\nuseage:\n%s [-h] [[-p <device> [<hex_file>]] | [-s <cod_file>]] [-c <stc_file>]\n", iam);
  printf ("\t-h             : this help list\n");
  printf ("\t-p <device>    : processor (e.g. -pp16c84 for the 'c84)\n");
  printf ("\t<hex_file>     : input file in \"intelhex16\" format\n");
  printf ("\t-c <stc_file>  : startup command file\n");
  printf ("\t-s <cod_file>  : .cod symbol file\n");
  printf ("\t-L <path list> : colon separated list of directories to search.\n");
  printf ("\t-d <port>      : Use ICD with serial port <port>\n");
  printf ("\t-D <symbol>=<value> : Define a symbol that will exist in the gpsim\n"
          "\t                      symbol table\n You may define any number.\n");
  printf ("\n\t-v             : gpsim version\n");
  printf ("\n Long options:\n\n");
  printf ("\t--cli          : command line mode only\n");
  printf ("\n\texamples:\n\n");
  printf ("%s -s myprog.cod          <-- loads a symbol file\n",iam);
  printf ("%s -p p16f877 myprog.hex  <-- select processor and load hex\n",iam);
  printf ("%s -c myscript.stc        <-- loads a script\n",iam);

}




void welcome(void)
{
  printf("\ngpsim - the GNUPIC simulator\nversion: %s %s\n",
    szBuild, VERSION);
  printf("\n\ntype help for help\n");

  return;
}

int 
main (int argc, char *argv[])
{
  bool bEcho = false;
  bool bSourceEnabled = true;
  int c,usage=0;
  bool bUseGUI = true;    // assume that we want to use the gui
  char command_str[256];
  poptContext optCon;     // context for parsing command-line options

  optCon = poptGetContext(0, argc, (const char **)argv, optionsTable, 0);
  //poptSetOtherOptionHelp(optCon, "[-h] [-p <device> [<hex_file>]] [-c <stc_file>]");

  welcome();

  if(argc>=2) {
    while ((c = poptGetNextOpt(optCon)) >= 0  && !usage) {
      switch (c) {

      default:
        printf("'%c' is an unrecognized option\n",c);
      case '?':
      case 'h':
        usage = 1;
        break;

      case 'L':
        set_search_path (search_path);
        break;

      case 'd':
        printf("Use ICD with serial port \"%s\".\n", icd_port);
        break;
	
      case 'v':
        gpsim_version();
        break;

      case 'i':
        bUseGUI = false;
        printf("not using gui\n");
        break;

      case 'D':
        // add symbols on the command line to the symbol table.
        get_symbol_table().AddFromCommandLine(defineSymbol);
        defineSymbol = "";
        break;

      case 'S':
        if(strcmp(sourceEnabled, "enable") == 0) {
          bSourceEnabled = true;
        }
        else if(strcmp(sourceEnabled, "disable") == 0) {
          bSourceEnabled = false;
        }
        else {
          usage = 1;
        }
        break;

      case 'E':
        bEcho = true;
        EnableSTCEcho(true);
        break;

      case 'e':
        if(strcmp(sExitOn, "onbreak") == 0) {
          get_bp().EnableExitOnBreak(true);
        }
        else {
          printf("%s is invalid exit condition for -e option.\n", sExitOn);
        }
        break;
      }	

      if (usage)
        break;
    }

  }

  if (usage) {
    helpme(argv[0]);
    exit (1);
  }

  if(bEcho) {
    for(int index = 0; index < argc; index++) {
      printf("%s ", argv[index]);
    }
    printf("\n");
  }
  if(poptPeekArg(optCon))
	  hex_name=strdup(poptPeekArg(optCon));
  
#ifdef HAVE_GUI  
  get_interface().setGUImode(bUseGUI);
#endif
  InitSourceSearchAsSymbol();
  initialize_ConsoleUI();
  initialize_gpsim_core();
  initialize_gpsim();
  initialize_commands();
  initialize_readline();

  // must be done after initialize_gpsim_core()
  Boolean &bEnableSourceLoad = *get_symbol_table().findBoolean("EnableSourceLoad");
  bEnableSourceLoad = bSourceEnabled;

  // initialize the gui
  
#ifdef HAVE_GUI
  if(bUseGUI) {
    if (gui_init (argc,argv) != 0) {
      std::cerr << "Error initialising GUI, reverting to cmd-line mode."
	        << std::endl;
    }
  }
#endif


  string s;
  string sGpsimPath(argv[0]);
  AddModulePathFromFilePath(sGpsimPath);

  initialization_is_complete();

  yydebug = 0;

  quit_parse = 0;
  abort_gpsim = 0;

  // Convert the remaining command line options into gpsim commands
  if(*cod_name) {

    if(*processor_name)
      cout << "WARNING: command line processor named \"" << processor_name <<
        "\" is being ignored\nsince the .cod file specifies the processor\n";
    if(*hex_name)
      cout << "WARNING: Ignoring the hex file \"" << hex_name <<
        "\"\nsince the .cod file specifies the hex code\n";

    snprintf(command_str, sizeof(command_str),
	     "load s \"%s\"\n",cod_name);
    parse_string(command_str);

  } else  if(*processor_name) {
    snprintf(command_str, sizeof(command_str),
	     "processor \"%s\"\n",processor_name);
    parse_string(command_str);

    if(*hex_name){
      snprintf(command_str, sizeof(command_str),
        "load h \"%s\"\n",hex_name);
      parse_string(command_str);
    }
  }
  if(*icd_port) {
      snprintf(command_str, sizeof(command_str),
	       "icd open \"%s\"\n",icd_port);
      parse_string(command_str);
  }
  
  if(*startup_name) {
      snprintf(command_str, sizeof(command_str),
	       "load c \"%s\"\n",startup_name);
      parse_string(command_str);
  }

  if(abort_gpsim)
    exit_gpsim();


  // Now enter the event loop and start processing user
  // commands.
  try {

#ifdef HAVE_GUI
      gui_main();
#else
      cli_main();
#endif
  }

  catch (char * err_message)
    {
      cout << "FATAL ERROR: " << err_message << endl;
    }

  exit_gpsim();

  return 0;
}


