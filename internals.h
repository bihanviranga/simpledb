#ifndef _INTERNALS_H
#define _INTERNALS_H

#include <stdint.h>

#include "results.h"

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 225

#define TABLE_MAX_PAGES 100

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

typedef enum {
  STATEMENT_INSERT,
  STATEMENT_SELECT
} StatementType;

typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
  StatementType type;
  Row row_to_insert;  // for insert statement
} Statement;

typedef struct {
  uint32_t num_rows;
  void* pages[TABLE_MAX_PAGES];
} Table;

void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);

void* row_slot(Table* table, uint32_t row_num);

void print_row(Row* row);

ExecuteResult execute_insert(Statement* statement, Table* table);
ExecuteResult execute_select(Statement* statement, Table* table);

Table* new_table();
void free_table(Table* table);

#endif