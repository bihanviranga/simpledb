#ifndef _RESULTS_H
#define _RESULTS_H

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
  PREPARE_SUCCESS,
  PREPARE_UNRECOGNIZED_STATEMENT,
  PREPARE_SYNTAX_ERROR
} PrepareResult;

typedef enum {
  EXECUTE_TABLE_FULL,
  EXECUTE_SUCCESS
} ExecuteResult;

#endif