/******************************************************************************** 
 * interface.h : User input interface functions
 ********************************************************************************/
#include "interface.h"

#include <stdio.h>
#include <stdlib.h>

/* 
 * Creates a new InputBuffer struct and initializes values.
 */
InputBuffer* new_input_buffer() {
  InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

/* 
 * Reads input from a given InputBuffer.
 */
void read_input(InputBuffer* input_buffer) {
  ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  if (bytes_read <= 0) {
    printf("Error reading input.\n");
    exit(EXIT_FAILURE);
  }

  /* Ignore trailing newline */
  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

/* 
 * Destroys a given InputBuffer and frees memory.
 */
void close_input_buffer(InputBuffer* input_buffer) {
  free(input_buffer->buffer);
  free(input_buffer);
}

/* 
 * Prompt for the database program.
 */
void print_prompt() { printf("db > "); }
