/******************************************************************************** 
 * internals.c : Internal implementation of data structures
 ********************************************************************************/
#ifndef _INTERNALS_H
#define _INTERNALS_H

#include <stdint.h>

#include "results.h"

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

/* Arbitrary value. Table is composed of pages, each of which has rows. */
#define TABLE_MAX_PAGES 100

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

typedef enum {
  STATEMENT_INSERT,
  STATEMENT_SELECT
} StatementType;

/* 
 * The additional byte (+1) is given for the null byte at the end.
 * Without this, the test fails.
 */
typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
  StatementType type;
  Row row_to_insert; /* Used only by the insert statement */
} Statement;

typedef struct {
  int file_descriptor;
  uint32_t file_length;
  void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
  uint32_t num_rows;
  Pager* pager;
} Table;

void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);

void* row_slot(Table* table, uint32_t row_num);

void print_row(Row* row);

ExecuteResult execute_insert(Statement* statement, Table* table);
ExecuteResult execute_select(Statement* statement, Table* table);

Table* db_open(const char* filename);
void free_table(Table* table);

#endif
