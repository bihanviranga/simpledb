/******************************************************************************** 
 * main.c : Drives the execution of the simpledb program.
 ********************************************************************************/
#include <stdbool.h>
#include <stdio.h>

#include "interface.h"
#include "internals.h"
#include "processor.h"

/* 
 * Entry point for the simpledb program.
 * 
 * Creates an in-memory table. Reads user input, and if the input is a meta-command
 * executes it. Otherwise it prepares the statement and executes it. 
 */
int main(int argc, char* argv[]) {
  Table* table = new_table();
  InputBuffer* input_buffer = new_input_buffer();

  /* REPL */
  while (true) {
    print_prompt();
    read_input(input_buffer);

    /* Meta-commands begin with a . (dot) character */
    if (input_buffer->buffer[0] == '.') {
      switch (do_meta_command(input_buffer)) {
        case (META_COMMAND_SUCCESS):
          continue;
        case (META_COMMAND_UNRECOGNIZED_COMMAND):
          printf("Unrecognized command '%s'\n", input_buffer->buffer);
          continue;
      }
    }

    Statement statement;
    switch (prepare_statement(input_buffer, &statement)) {
      case (PREPARE_SUCCESS):
        break;
      case (PREPARE_SYNTAX_ERROR):
        printf("Syntax error. Could not parse statement.\n");
        continue;
      case (PREPARE_UNRECOGNIZED_STATEMENT):
        printf("Unrecognized keyword at start of '%s'\n", input_buffer->buffer);
        continue;
      case (PREPARE_STRING_TOO_LONG):
        printf("Maximum string length exceeded.\n");
        continue;
      case (PREPARE_NEGATIVE_ID):
        printf("ID cannot be negative.\n");
        continue;
    }

    switch (execute_statement(&statement, table)) {
      case (EXECUTE_SUCCESS):
        printf("Executed.\n");
        break;
      case (EXECUTE_TABLE_FULL):
        printf("Error: Table full.\n");
        break;
    }
  }
}
