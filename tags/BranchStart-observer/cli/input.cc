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


#include "../src/gpsim_def.h"

#ifdef HAVE_GUI
#include <glib.h>
#include <gdk/gdktypes.h>
#include <gdk/gdk.h>
#endif

#include <stdio.h>
extern "C" {
#include <readline/readline.h>
}

extern "C" {
#include <readline/history.h>
}

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//#include <slang/slang.h>
#include <string>
#include <list>
#include <csignal>


#include "command.h"
#include "input.h"

#define MAX_LINE_LENGTH 256  

int yyparse(void);
void initialize_readline (void);
extern "C" {
void exit_gpsim(void);
#ifdef HAVE_GUI
  void quit_gui(void);

#endif

void redisplay_prompt(void);
}

bool using_readline=1;
int input_mode = 0;
int allocs = 0;
/* When non-zero, this global means the user is done using this program. */
int done = 0;

const char *gpsim = "gpsim> ";  // Normal prompt
const char *gpsim_cont = "> ";  // command continuation prompt

extern int quit_parse;

// COMMAND_MODES command_mode;


//==========================================================================
//extern void catch_control_c(int); // In breakpoints.cc
/*
void initialize_signals(void)
{

  static struct sigaction action;

  action.sa_handler = catch_control_c;
  sigemptyset(&action.sa_mask);
  action.sa_flags=0;

  sigaction(SIGINT, &action, 0);


}
*/

char *cmd_string_buf = NULL;
FILE *cmd_file = NULL;


/*******************************************************
 */
char * strip_cmd(char *buff)
{
  char *comment;

  if(!buff)
    return buff;

  // Strip leading spaces
  while(*buff == ' ') 
    *buff++;

  comment = strchr(buff, '#');

  // If there's a comment, remove it.
  if(comment) {
    *comment = '\0';
  }

  return buff;
}

/*******************************************************
 * parse_string
 * 
 * This routine will run a string through the command parser
 *this is useful if you want to execute a command but do not
 *wish to go through the readline stuff.
 */
int parse_string(char *cmd_string)
{
  static int save_readline_state = 0;
  static char last_line[256] = {0};

  int retval;

  save_readline_state = using_readline;
  using_readline = 0;

  if(!cmd_string)
    return 0;

  if(verbose & 2)
    printf("   %s: %s\n",__FUNCTION__,cmd_string);

  cmd_string = strip_cmd(cmd_string);

  cmd_string_buf = strdup(cmd_string); // free'd by gpsim_read


  if( strlen (cmd_string) == 0)
    {
      if(*last_line)
	cmd_string_buf = strdup(last_line);

    }
  else
    {

      if(strlen(cmd_string)) {
	add_history (cmd_string);

	strncpy(last_line,cmd_string,256);
      }
    }


  init_parser();
  retval = yyparse();

  using_readline = save_readline_state;

  if(quit_parse)
    exit_gpsim();

  return retval;

}

char * gets_from_cmd_file(char **ss)
{
  char str[MAX_LINE_LENGTH];

  str[0] = 0;

  if(verbose&4)
    cout << __FUNCTION__ <<"()\n";

  if(cmd_file) {
    fgets(str, MAX_LINE_LENGTH, cmd_file);
    *ss= strdup(str); // free'd by gpsim_read
    allocs++;
    //cout << "allocs++" << allocs << '\n';
  }

  return *ss;

}

void process_command_file(char * file_name)
{

  int save_readline_state;
  FILE *save_cmd_file;

  if(verbose&4)
    cout << __FUNCTION__ <<"()\n";

  save_cmd_file = cmd_file;

  cmd_file = fopen(file_name,"r");

  save_readline_state = using_readline;
  using_readline = 0;
  cmd_string_buf = NULL;

  if(cmd_file)
    {
      // cout << "processing a command file\n";

      quit_parse = 0;
      while( !quit_parse )
	{
	  init_parser();
	  yyparse();
	}

      fclose(cmd_file);
      cmd_file = save_cmd_file;
    }


  using_readline = save_readline_state;

}

static char *
get_user_input (void)
{
  char *retval = 0;
  int i,len;

  if(verbose&4)
    cout << __FUNCTION__ <<"()\n";

  #ifndef HAVE_GUI

  if(using_readline)
    {

      retval = gnu_readline (gpsim,1);
    }
  else
    {
  #endif

      gets_from_cmd_file(&cmd_string_buf);

      retval = cmd_string_buf;
  #ifndef HAVE_GUI
    }
  #endif

  return retval;
}

//*********************************************
// gpsim_read
//
//  This function is called from the parser. It will either read
// a line of text from a command file or from stdin -- depending
// on the current source for data. Once a string is obtained, it
// is copied into a buffer that the parser has passed to us.

int
gpsim_read (char *buf, unsigned max_size)
{
  char *input_buf;
  static int chars_left = 0;
  int status = 0;

  if(verbose&4)
    cout <<"gpsim_read\n";

  input_buf = get_user_input ();
  chars_left = input_buf ? strlen (input_buf) : 0;

  if (chars_left > 0)
    {
      buf[0] = '\0';

      chars_left = ( (chars_left < max_size) ? chars_left : max_size);

      strncpy (buf, input_buf, chars_left);

      // If the input string does not have a carriage return, then add one.
      if (buf[chars_left-1] != '\n')
	buf[chars_left++] = '\n';
      buf[chars_left] = 0;

      //cout << "chars_left > 0, copied cur_pos into buff\n";
      //cout << "buf[]  " << buf << '\n';

      // If we are reading from a command file (and not stdin), then
      // the string that was read copied into a dynamically allocated
      // buffer that we need to delete.

      if(allocs) {
	allocs--;

	//cout << "freeing input_buf " <<input_buf <<'\n';
	//cout << "allocs--" << allocs << '\n';

	free (input_buf);
      }
      //status = len;
      //chars_left = 0;
    }

  if(verbose&4)
    cout << "leaving gpsim_read\n";

  return chars_left;
}

#if 0
___main_input (void)
{

  COMMAND_MODES command_mode=NO_COMMAND;


  // if there is a startup file then read it and do what it says
  //  if(startup)
  //  process_command_file(startup);

  initialize_readline ();	/* Bind our completer. */

  //  initialize_signals();

  char last_line[256];
  last_line[0] = 0;

  int flag = 0;

  /* Loop reading and executing lines until the user quits. */
  while (!quit_gpsim)
    {
      char *line;
      
      if(command_mode == NO_COMMAND)
	{
	  line = ::readline (gpsim);
	}
      else
	{
	  line = ::readline (gpsim_cont);
	}

      if (!line)
	{
	  done = 1;		/* Encountered EOF at top level. */
	}
      else
	{
	  /* Remove leading and trailing whitespace from the line.
	     Then, if there is anything left, add it to the history list
	     and execute it. */
	  //stripwhite (line);

	  if (*line)
	    {
	      add_history (line);
	      //command_mode = execute_line (line);
	      strncpy(last_line,line,256);
	    }
	  else
	    {
	      //cout << "empty line\n";
	      //if(*last_line)
	      // command_mode = execute_line (last_line);
	    }
	}

      if (line)
	free (line);
      line = NULL;

    }
  exit (0);
}

#endif

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */


/* Generator function for command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
char *
command_generator (char *text, int state)
{
  char  *n;
  static int i = 0;

  /* If this is a new word to complete, initialize now.  */

  if (state == 0)
    i = 0;

  /* Return the next name which partially matches from the command list. */
  while( i<number_of_commands)
    {

      if(strstr(command_list[i]->name, text) == command_list[i]->name)
	{
	  n = (char *) malloc(strlen(command_list[i]->name));
	  if(n == NULL)
	    {
	      //fprintf (stderr, "malloc: Out of virtual memory!\n");
	      abort ();
	    }
	  strcpy(n,command_list[i]->name);
	  i++;
	  return(n);
	}
      i++;
    }

  /* If no names matched, then return NULL. */
  return ((char *)NULL);
}

/* Attempt to complete on the contents of TEXT.  START and END show the
   region of TEXT that contains the word to complete.  We can use the
   entire line in case we want to do some simple parsing.  Return the
   array of matches, or NULL if there aren't any. */
char **
gpsim_completion (char *text, int start, int end)
{
  char **matches;
  //  char *command_generator (char *, int);

  matches = (char **)NULL;

  /* If this word is at the start of the line, then it is a command
     to complete.  Otherwise it is the name of a file in the current
     directory. */
  if (start == 0)
    matches = completion_matches (text, command_generator);

  return (matches);
}


#ifdef HAVE_GUI

void myfunc(int data, int fd, GdkInputCondition gdk_cond)
{
  //  static int sequence = 0;
  //  char c=0;


  rl_callback_read_char ();


}

void test_func(void)
{

  char *t;

  t = rl_copy_text(0,1000);

  if(verbose & 2) {
    printf(" ()()()   %s  ()()()\n%s\n", __FUNCTION__ , t);
  }
  parse_string(t);
  free(t);

}

#endif


void exit_gpsim(void)
{

#ifdef HAVE_GUI

	quit_gui();

#endif

	rl_callback_handler_remove ();

	exit(0);
}

/* redisplay_prompt will redisplay the current data in the readline buffer.
   This function is used to restore a prompt that's been obliterated by diagnostic
   data.
*/

void redisplay_prompt(void)
{

  //  rl_redisplay();
  cout << '\n';
  rl_forced_update_display();
}

/* Tell the GNU Readline library how to complete.  We want to try to complete
   on command names if this is the first word in the line, or on filenames
   if not. */
void initialize_readline (void)
{
  //  char **gpsim_completion (char *, int, int);

  static char b[100];

  sprintf(b,"hello");

  rl_initialize ();

#ifdef HAVE_GUI
  gdk_input_add (fileno(stdin), GDK_INPUT_READ, 
                 (GdkInputFunction) myfunc, b);

  rl_callback_handler_install (gpsim, test_func);

#endif

  /* Allow conditional parsing of the ~/.inputrc file. */
  //  rl_readline_name = "gpsim";

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = (CPPFunction *)gpsim_completion;


}


#ifndef HAVE_GUI

char *gnu_readline (char *s, unsigned int force_readline)
{
  static char last_line[256]={0};
  char *retval = 0;


  if(using_readline || force_readline)
    {
      //cout << "about to do readline\n";

      char *tmp;

      if(input_mode == CONTINUING_LINE)
	retval = ::readline (gpsim_cont);
      else
	retval = ::readline (s);
      tmp = retval;

      /*
      if(tmp)
	{
	  cout << "tmp is not null and is " << strlen(tmp) << " chars long\n";
	}
      else
	{
	  cout << "tmp is null";
	}
      */

      if (tmp == NULL)
	{
	  retval = (char *) malloc (2);
	  retval[0] = '\n';
	  retval[1] = '\0';
	}
      else
	{
	  if( strlen (tmp) == 0)
	    {
	      if(*last_line)
		{
		  retval = strdup(last_line);
		}
	      else
		{
		  retval = (char *) malloc (2);
		  retval[0] = '\n';
		  retval[1] = '\0';
		}
	    }
	  else
	    {
	      add_history (tmp);

	      strncpy(last_line,tmp,256);
	    }
	}
    }
  else
    {
      if(cmd_string_buf)
	{
	  //cout << "processing a command string\n";
	  retval = cmd_string_buf;
	}
      else
	{
	  cout << "read line error\n";
	  exit(1);
	}
    }

  return retval;
}


#endif
