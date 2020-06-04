/******************************************************************************** 
 * internals.c : Internal implementation of data structures
 ********************************************************************************/
#include "internals.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "interface.h"

/* 
 * Row layout
 */
const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096; /* Arbitrary value */

/* 
 * Executes an insert statment, when the Statement and the Table is given.
 */
ExecuteResult execute_insert(Statement* statement, Table* table) {
  if (table->num_rows >= TABLE_MAX_ROWS) {
    return EXECUTE_TABLE_FULL;
  }

  Row* row_to_insert = &(statement->row_to_insert);
  Cursor* cursor = table_end(table);

  serialize_row(row_to_insert, cursor_value(cursor));
  table->num_rows += 1;

  free(cursor);

  return EXECUTE_SUCCESS;
}

/* 
 * Executes a select statement, when the Statement and Table is given.
 */
ExecuteResult execute_select(Statement* statement, Table* table) {
  Row row;
  Cursor* cursor = table_start(table);

  while (!(cursor->end_of_table)) {
    deserialize_row(cursor_value(cursor), &row);
    print_row(&row);
    cursor_advance(cursor);
  }

  free(cursor);

  return EXECUTE_SUCCESS;
}

/* 
 * Opens a database connection. Intializes a table struct and its pager.
 */
Table* db_open(const char* filename) {
  Pager* pager = pager_open(filename);
  uint32_t num_rows = pager->file_length / ROW_SIZE;

  /* There is no new_table method anymore. So this is where new tables are created. */
  Table* table = malloc(sizeof(Table));
  table->pager = pager;
  table->num_rows = num_rows;

  return table;
}

/* 
 * Calculates the memory location for a row, when a cursor is given.
 */
void* cursor_value(Cursor* cursor) {
  uint32_t page_num = cursor->page_num;
  void* page = get_page(cursor->table->pager, page_num);
  return leaf_node_value(page, cursor->cell_num);
}

/* 
 * Copies a given Row to memory.
 */
void serialize_row(Row* source, void* destination) {
  memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
  memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
  memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

/* 
 * Retrieves a Row from a given memory location.
 */
void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

/* 
 * Creates a pager and initializes its values.
 * All the pages in the pager are set to null.
 * They are only loaded to memory when requested, to keep resource usage low.
 */
Pager* pager_open(const char* filename) {
  int fd = open(
      filename,
      O_RDWR |     /* Read/Write mode */
          O_CREAT, /* Create file if it does not exist */
      S_IWUSR |    /* User write permission */
          S_IRUSR  /* User read permission */

  );

  /* 
   * TODO: move this error message elsewhere.
   * Return an error from an enum and catch it in a switch like
   * other errors.
   */
  if (fd == -1) {
    printf("Unable to open file\n");
    exit(EXIT_FAILURE);
  }

  off_t file_length = lseek(fd, 0, SEEK_END);

  Pager* pager = malloc(sizeof(Pager));
  pager->file_descriptor = fd;
  pager->file_length = file_length;
  pager->num_pages = (file_length / PAGE_SIZE);

  if (file_length % PAGE_SIZE != 0) {
    /* TODO: move this error message elsewhere */
    printf("Partial page found. Db file should contain a whole number of pages. Corrupted file.\n");
    exit(EXIT_FAILURE);
  }

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    pager->pages[i] = NULL;
  }

  return pager;
}

/* 
 * Returns the page with the given page number from a pager.
 * If the page is not in the cache (in-memory), reads it from disk.
 * If the requested page number is not found in the file, it allocates a new 
 * blank page. This blank page will not be persisted to the disk until flushed.(eg: using db_close())
 */
void* get_page(Pager* pager, uint32_t page_num) {
  /* TODO: Move this error message elsewhere */
  if (page_num > TABLE_MAX_PAGES) {
    printf("Tried to fetch page number out of bounds: %d > %d\n", page_num, TABLE_MAX_PAGES);
    exit(EXIT_FAILURE);
  }

  if (pager->pages[page_num] == NULL) {
    /* Cache miss. Allocate memory for page and read from file. */
    void* page = malloc(PAGE_SIZE);
    uint32_t num_pages = pager->file_length / PAGE_SIZE;

    /* 
     * There might be a partially filled page at the end of the file.
     * It should be counted as well.
     */
    if (pager->file_length % PAGE_SIZE) {
      num_pages += 1;
    }

    /* 
     * If the requested page has been used before, load it to memory.
     * Otherwise we can return the already allocated blank page.
     */
    if (page_num <= num_pages) {
      lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
      ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
      if (bytes_read == -1) {
        /* TODO: move this error message elsewhere */
        printf("Error reading file: %d\n", errno);
        exit(EXIT_FAILURE);
      }
    }

    pager->pages[page_num] = page;

    /* 
     * If the requested page is a new page, update the pagers total accordingly.
     */
    if (page_num >= pager->num_pages) {
      pager->num_pages = page_num + 1;
    }
  }

  return pager->pages[page_num];
}

/* 
 * Closes the database connection.
 * The pages in the memory are flushed and written to disk.
 * Then the pager and table memories are freed.
 */
void db_close(Table* table) {
  Pager* pager = table->pager;

  for (uint32_t i = 0; i < pager->num_pages; i++) {
    if (pager->pages[i] == NULL) {
      continue;
    }
    pager_flush(pager, i);
    free(pager->pages[i]);
    pager->pages[i] = NULL;
  }

  int result = close(pager->file_descriptor);
  if (result == -1) {
    /* TODO: move this error message elsewhere. */
    printf("Error closing db file.\n");
    exit(EXIT_FAILURE);
  }

  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    void* page = pager->pages[i];
    if (page) {
      free(page);
      pager->pages[i] = NULL;
    }
  }
  free(pager);
  free(table);
}

void pager_flush(Pager* pager, uint32_t page_num) {
  if (pager->pages[page_num] == NULL) {
    /* TODO: move this error message elsewhere */
    printf("Tried to flush null page.\n");
    exit(EXIT_FAILURE);
  }

  off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

  if (offset == -1) {
    /* TODO: move this error message elsewhere */
    printf("Error seeking: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);

  if (bytes_written == -1) {
    /* TODO: move this error message elsewhere */
    printf("Error writing: %d\n", errno);
    exit(EXIT_FAILURE);
  }
}

/* 
 * Creates a cursor pointing to the start of the table.
 */
Cursor* table_start(Table* table) {
  Cursor* cursor = malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = table->root_page_num;
  cursor->cell_num = 0;

  void* root_node = get_page(table->pager, table->root_page_num);
  uint32_t num_cells = *leaf_node_num_cells(root_node);
  cursor->end_of_table = (num_cells == 0);

  return cursor;
}

/* 
 * Creates a cursor pointing to the end of the table.
 */
Cursor* table_end(Table* table) {
  Cursor* cursor = malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = table->root_page_num;

  void* root_node = get_page(table->pager, table->root_page_num);
  uint32_t num_cells = *leaf_node_num_cells(root_node);
  cursor->cell_num = num_cells;

  cursor->end_of_table = true;
}

/*
 * Advances a cursor by one row.
 */
void cursor_advance(Cursor* cursor) {
  uint32_t page_num = cursor->page_num;
  void* node = get_page(cursor->table->pager, page_num);

  cursor->cell_num += 1;
  if (cursor->cell_num >= (*leaf_node_num_cells(node))) {
    cursor->end_of_table = true;
  }
}

/******************************************************************************** 
 * B+ tree implementation
 * 
 * I wanted to have the tree in a seperate file. But then I ran into an error when
 * accessing ROW_SIZE and PAGE_SIZE from another file.
 * TODO: Refactor the tree into another file. 
 ********************************************************************************/

/* 
 * Common Node Header Layout
 */
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint8_t COMMON_NODE_HEADER_SIZE =
    NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/* 
 * Leaf Node Header Layout 
 */
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE =
    COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

/* 
 * Leaf Node Body Layout
 * 
 * In the body of a leaf node, there will be an array of cells.
 * A cell is a key followed by its value.
 * (In this case, a serialized row)
 */
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0; /* Offset 0 of the BODY, not counting the HEADER */
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

/* TODO : create a graphic illustrating the node memory structure */

uint32_t* leaf_node_num_cells(void* node) {
  return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void* leaf_node_cell(void* node, uint32_t cell_num) {
  return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num);
}

void* leaf_node_value(void* node, uint32_t cell_num) {
  return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void initialize_leaf_node(void* node) {
  *leaf_node_num_cells(node) = 0;
}
