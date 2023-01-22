
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int FILE_MAX_LENGTH = 255;
int MAX_TOKENS = 150;
int MAX_COMMANDS = 10;
int MAX_VARS_SPACE = 100;

struct command
{
  int name;
  int continuation;
  char *first_arg;
  char *second_arg;
};

struct token
{
  int key;
  char *value;
};

struct binding
{
  char *name;
  char *value;
};

#define NL_TOKEN 0   // Next line
#define END_TOKEN 1  // End of code
#define IFNL_TOKEN 2 // If last eval was false, next line
#define AS_TOKEN 3   // Assignment
#define SIN_TOKEN 4  // Standard input
#define SOUT_TOKEN 5 // Standard output
#define NE_TOKEN 6   // Non equal
#define VAL_TOKEN 7  // Value

#define NL_CN 0   // Next line continuation
#define INFL_CN 1 // If last eval was false, next line
#define END_CN 2  // End continuation

#define AS_ST 0  // Assignment statement
#define NE_ST 1  // Non Equal Compare Statement
#define OUT_ST 2 // Output Statement
#define IN_ST 3  // In statement

// Receives the string and returns the token identifier number
int tokenize(char *to_tokenize)
{

  if (strcmp(to_tokenize, ">>") == 0)
    return NL_TOKEN;
  if (strcmp(to_tokenize, "<<") == 0)
    return END_TOKEN;
  if (strcmp(to_tokenize, "^>>") == 0)
    return IFNL_TOKEN;
  if (strcmp(to_tokenize, "<") == 0)
    return AS_TOKEN;
  if (strcmp(to_tokenize, "stdin") == 0)
    return SIN_TOKEN;
  if (strcmp(to_tokenize, "stdout") == 0)
    return SOUT_TOKEN;
  if (strcmp(to_tokenize, "<>") == 0)
    return NE_TOKEN;

  return VAL_TOKEN;
}

int index_of_binding(struct binding *bindings, int bindings_length, char *name)
{
  for (int i = 0; i < bindings_length; i += 1)
  {
    if (strcmp(name, bindings[i].name) == 0)
    {
      return i;
      break;
    }
  }

  return -1; // New one :D
}

int main(int argc, char **argv)
{
  /* Step 1: Read the file */

  // Open the file
  FILE *file_pointer = fopen(argv[1], "r");

  // Read the contents
  char buff[FILE_MAX_LENGTH];
  fgets(buff, FILE_MAX_LENGTH, (FILE *)file_pointer);

  /* Step 2: Tokenize the file */

  // Allocate space for the tokens
  struct token *tokens = malloc(sizeof(struct token) * MAX_TOKENS);

  // Keep track of the last token index
  int last_token_index = 0;

  // Iterate while things to tokenize
  char *next_to_tokenize = strtok(buff, " ");
  while (next_to_tokenize != NULL)
  {
    tokens[last_token_index].key = tokenize(next_to_tokenize);
    tokens[last_token_index].value = next_to_tokenize;

    last_token_index += 1;
    next_to_tokenize = strtok(NULL, " ");
  }

  /* Step 3: Parse tokens into commands */

  // Allocate space for the commands
  struct command *commands = malloc(sizeof(struct token) * MAX_COMMANDS);
  int command_index = 0;

  struct command current_command;

  for (int index = 0; index < last_token_index; index += 4)
  {
    if (
        tokens[index].key == SOUT_TOKEN)
    {
      commands[command_index].name = OUT_ST;
      commands[command_index].first_arg = tokens[index + 2].value;
    }
    else if (
        tokens[index + 2].key == SIN_TOKEN)
    {
      commands[command_index].name = IN_ST;
      commands[command_index].first_arg = tokens[index].value;
    }
    else if (
        tokens[index + 1].key == NE_TOKEN)
    {
      commands[command_index].name = NE_ST;
      commands[command_index].first_arg = tokens[index].value;
      commands[command_index].second_arg = tokens[index + 2].value;
    }
    else
    {
      commands[command_index].name = AS_ST;
      commands[command_index].first_arg = tokens[index].value;
      commands[command_index].second_arg = tokens[index + 2].value;
    }

    // End of statement
    if (
        tokens[index + 3].key == NL_TOKEN)
    {
      commands[command_index].continuation = NL_CN;
    }
    else if (
        tokens[index + 3].key == END_TOKEN)
    {
      commands[command_index].continuation = END_CN;
    }
    else if (
        tokens[index + 3].key == IFNL_TOKEN)
    {
      commands[command_index].continuation = INFL_CN;
    }

    // DEBUG Command
    // printf("%d %s %s %d \n", commands[command_index].name, commands[command_index].first_arg, commands[command_index].second_arg, commands[command_index].continuation);

    command_index += 1;
  }

  struct binding *bindings = malloc(MAX_VARS_SPACE);
  int last_binding_index = 0;

  int last_eval;
  int end = 0;

  for (int current_command_index = 0; current_command_index < command_index; current_command_index += 1)
  {

    int new_index;
    switch (commands[current_command_index].name)
    {
    case AS_ST:

      new_index = index_of_binding(bindings, last_binding_index, commands[current_command_index].first_arg);
      if (new_index == -1)
      {
        new_index = last_binding_index;
        last_binding_index += 1;
      }
      bindings[new_index].name = commands[current_command_index].first_arg;
      bindings[new_index]
          .value = commands[current_command_index].second_arg;
      last_eval = 1;

      break;
    case NE_ST:
      last_eval = strcmp(
                      bindings[index_of_binding(bindings, last_binding_index, commands[current_command_index].first_arg)].value,
                      bindings[index_of_binding(bindings, last_binding_index, commands[current_command_index].second_arg)].value) == 0
                      ? 1
                      : 0;
      break;
    case OUT_ST:
      last_eval = printf("%s \n", bindings[index_of_binding(bindings, last_binding_index, commands[current_command_index].first_arg)].value);

      break;
    case IN_ST:
      new_index = index_of_binding(bindings, last_binding_index, commands[current_command_index].first_arg);

      if (new_index == -1)
      {
        new_index = last_binding_index;
        last_binding_index += 1;
      }
      bindings[new_index].name = commands[current_command_index].first_arg;

      char input[30];
      fgets(input, 20, stdin);
      input[strcspn(input, "\n")] = 0;

      bindings[new_index].value = input;
      last_eval = 1;
    }

    switch (commands[current_command_index].continuation)
    {
    case NL_CN:
      break;
    case END_CN:
      end = 1;
      break;
    case INFL_CN:
      if (last_eval == 0)
      {
        end = 1;
      }
      break;
    }

    if (end)
    {
      break;
    }
  }

  fclose(file_pointer);
}
