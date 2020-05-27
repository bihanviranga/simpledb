#ifndef _PROCESSOR_H
#define _PROCESSOR_H

#include <stdint.h>

#include "interface.h"

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 225

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

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
  STATEMENT_INSERT,
  STATEMENT_SELECT
} StatementType;

typedef struct {
  StatementType type;
  Row row_to_insert;  // for insert statement
} Statement;

typedef enum {
  EXECUTE_TABLE_FULL,
  EXECUTE_SUCCESS
} ExecuteResult;

MetaCommandResult do_meta_command(InputBuffer* input_buffer);

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);

ExecuteResult execute_statement(Statement* statement);

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

void serialize_row(Row* source, void* destination);

void deserialize_row(void* source, Row* destination);

#define TABLE_MAX_PAGES 100

typedef struct {
  uint32_t num_rows;
  void* pages[TABLE_MAX_PAGES];
} Table;

void* row_slot(Table* table, uint32_t row_num);

ExecuteResult execute_insert(Statement* statement, Table* table);
ExecuteResult execute_select(Statement* statement, Table* table);

#endif
