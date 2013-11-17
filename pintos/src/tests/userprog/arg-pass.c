/* Try passing arguments to process */
#include <stdio.h>
#include <string.h>
#include <syscall.h>
#include "tests/lib.h"

const char* test_name = "arg-pass";
static const char* test[] =
{
  "arg-pass", "arg1", "arg2", "arg3", "arg4", "arg5", "arg6", "arg7", 
  "arg8", "arg9", "arg10", "arg11", "arg12", "arg13", "arg14", "arg15",
  "arg16", "arg17", "arg18", "arg19", "arg20", "arg21", "arg22", "arg23",
  "arg24", "arg25", "arg26", "arg27", "arg28", "arg29", "arg30", "arg31",
  "arg32", "arg33", "arg34", "arg35", "arg36", "arg37", "arg38", "arg39",
  "arg40", "arg41", "arg42", "arg43", "arg44", "arg45", "arg46", "arg47",
  "arg48", "arg49", "arg50" };

static const char* long_argument = "ThisIsVeryLongArgument.ThisIsForArgumentPassingTestARGPASS.MaximumLengthOfArgumentCanBePassedIs128ByteIncluding\\0128LimitIsHereFROMNOWONTHISTESTSHOULDNOTBESHOWN";
static char cmd_line_input[4096];

static bool check_argument (int, char*);
static bool check_long_argument (char**);
static bool sequential_check_argument (int, char**);
static char* generate_cmd_line (int);
static char* generate_long_argument_cmd_line (void);

int
main (int argc, char *argv[])
{
  if (argc == 1)
  {
    msg ("Checking 0 Argument Passing");
    CHECK (argc == 1, "passed 0 argument");
    CHECK (check_argument (0, argv[0]), "check argv[0]=%s", argv[0]);

    msg ("Checking argc = 5, argc = 10");
    wait (exec (generate_cmd_line (5)));
    wait (exec (generate_cmd_line (10)));
    msg ("Checking Overflow Argument Passing: argc > 32: arguments = 50");
    wait (exec (generate_cmd_line (50)));
    msg ("Checking Overflow Arugment Length Passing");
    wait (exec (generate_long_argument_cmd_line ()));
  }
  else
  {
    CHECK (argc <= 32 && argc >= 0, "check argc");
    if (argc == 2)
    {
      msg ("Checking Long Argument Passing: passing arugment of length 161");
      CHECK (check_long_argument (argv), "check long argument");
    }
    else
    {
      msg ("Checking %d Arguments Passing", argc);
     CHECK (sequential_check_argument (argc, argv), "check argvs[0..%d]", argc-1);
    }
  }

  return 0;  
}

char*
generate_cmd_line (int size)
{
  int i;
  strlcpy (cmd_line_input, "", 4096);

  for (i = 0; i < size; i++)
  {
    strlcat (cmd_line_input, test[i], 4096);
    strlcat (cmd_line_input, " ", 4096);
  }
  return cmd_line_input;
}

char*
generate_long_argument_cmd_line (void)
{
  strlcpy (cmd_line_input, "", 4096);

  strlcat (cmd_line_input, test[0], 4096);
  strlcat (cmd_line_input, " ", 4096);
  strlcat (cmd_line_input, long_argument, 4096);

  return cmd_line_input;
}

bool
sequential_check_argument (int size, char** str)
{
  int i;
  bool check = true;
  for (i = 0; i < size; i++)
    check = check && !strcmp (test [i], str[i]);
  return check;
}

bool
check_long_argument (char** str)
{
  bool check = true;
  char check_str[128];

  strlcpy (check_str, long_argument, 128);

  check = !strcmp (test[0], str[0]) && !strcmp (check_str, str[1]);

  return check;
}

bool
check_argument (int index, char* str)
{
  return !strcmp (test [index], str);
}

