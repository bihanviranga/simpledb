/********************************************************************************
 * internals.h : Internal implementation of data structures
 ********************************************************************************/
#ifndef _INTERNALS_H
#define _INTERNALS_H

#include <stdbool.h>
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

/* Pager manages the pages of the table */
typedef struct {
  int file_descriptor;
  uint32_t file_length;
  uint32_t num_pages;
  void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
  uint32_t root_page_num;
  Pager* pager;
} Table;

/* A Cursor represents a location in the table
 * We can use cursors to represent the beginning, end, and selected rows.
 * It has a table reference in it so we can simply pass the cursor ref to functions.
 * Member end_of_table says whether this cursor points beyond the table.
 *
 * TODO: Do we need to track down existing cursors?
*/
typedef struct {
  Table* table;
  uint32_t page_num;
  uint32_t cell_num;
  bool end_of_table;
} Cursor;

/* NodeType is for the tree implementation */
typedef enum {
  NODE_INTERNAL,
  NODE_LEAF
} NodeType;

void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);

ExecuteResult execute_insert(Statement* statement, Table* table);
ExecuteResult execute_select(Statement* statement, Table* table);

Table* db_open(const char* filename);
void db_close(Table* table);

Pager* pager_open(const char* filename);
void* get_page(Pager* pager, uint32_t page_num);
void pager_flush(Pager* pager, uint32_t page_num);

Cursor* table_start(Table* table);
void* cursor_value(Cursor* cursor);
Cursor* table_find(Table* table, uint32_t key);
void cursor_advance(Cursor* cursor);

uint32_t* leaf_node_num_cells(void* node);
void* leaf_node_cell(void* node, uint32_t cell_num);
uint32_t* leaf_node_key(void* node, uint32_t cell_num);
void* leaf_node_value(void* node, uint32_t cell_num);
void initialize_leaf_node(void* node);
void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value);
Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key);
NodeType get_node_type(void* node);
void set_node_type(void* node, NodeType type);

void print_constants();
void print_leaf_node(void* node);

#endif
