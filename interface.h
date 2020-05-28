/******************************************************************************** 
 * interface.h : User input interface functions
 ********************************************************************************/
#ifndef _INTERFACE_H
#define _INTERFACE_H

#include <sys/types.h>

/* 
 * The actual input length could be shorter than the allocated buffer length.
 * Hence we have two lengths: size of the buffer and size of the input.
 */
typedef struct
{
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

InputBuffer* new_input_buffer();
void read_input(InputBuffer* input_buffer);
void close_input_buffer(InputBuffer* input_buffer);

void print_prompt();

#endif
