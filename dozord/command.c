/*
  This file is part of NightShift.

  NightShift is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NightShift is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NightShift. If not, see <https://www.gnu.org/licenses/>. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "command.h"
#include "trace.h"

short int getNextCommand(Commands * commands)
{
  unsigned short int index;
  short int found = -1;

  TRAP(commands == NULL, "Incoming argument pointer to commands is NULL.\n");

  if (commands->length == 0)
  {
    return found;
  }

  for (index = 0; index < commands->length; index++)
  {
    if (commands->items[index].done == 0)
    {
      found = index;
      break;
    }
  }

  return found;
}

void readCommandsFromFile(Commands * commands, char * fn)
{
  FILE * fp;
  char chunk[MAX_COMMAND_STRING_LENGTH];
  int i = 0;
  int iret = -1;

  TRAP(commands == NULL, "Incoming argument pointer to commands is NULL.\n");
  TRAP(fn == NULL, "Incoming argument pointer to filename is NULL.\n");

  iret = access(fn, R_OK);
  if (iret != 0) {
      ERROR_TRACE("Cannot access %s input file. Last system error description: %s\n",
        fn, trace_strerror(errno));
      return;
  }
  
  fp = fopen(fn, "r");
  if (fp == NULL)
  {
      ERROR_TRACE("Cannot open %s input file. Last system error description: %s\n",
        fn, trace_strerror(errno));
      return;
  }
  
  while (fgets(chunk, sizeof(chunk), fp) != NULL)
  {
    size_t chunk_size = strlen(chunk);
    if (chunk_size > 1)
    {
      strncpy(commands->items[i].value, chunk, chunk_size - 1);
      commands->items[i].value[chunk_size - 1] = '\0';
      
      DEBUG_TRACE("%s:%s: new command - '%s'.\n",
        __FILENAME__, __func__, commands->items[i].value);

      commands->items[i].done = 0;
      commands->items[i].id = i + 1;

      i++;
    }
  }

  commands->length = i;

  TRAP(fclose(fp) != 0, "Cannot close commands file.\n");
}

void readCommandsFromString(Commands * commands, char * command)
{
  int i = 0;

  TRAP(commands == NULL, "Incoming argument pointer to commands is NULL.\n");
  TRAP(command == NULL, "Incoming argument pointer to command string is NULL.\n");

  i = commands->length;
  if (i >= MAX_COMMAND_QUEUE_LENGTH)
  {
    i = 0;
  }

  commands->length = i + 1;
  
  snprintf(commands->items[i].value, MAX_COMMAND_STRING_LENGTH, "%s", command);
  commands->items[i].value[MAX_COMMAND_STRING_LENGTH - 1] = '\0';
  commands->items[i].done = 0;
  commands->items[i].id = i + 1;

  DEBUG_TRACE("%s:%s: new command - '%s'.\n",
    __FILENAME__, __func__, commands->items[i].value);
}
