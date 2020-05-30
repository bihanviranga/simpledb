#ifndef _PROCESSOR_H
#define _PROCESSOR_H

#include "interface.h"
#include "internals.h"

MetaCommandResult do_meta_command(InputBuffer* input_buffer);

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);
PrepareResult prepare_insert(InputBuffer* input_buffer, Statement* statement);

ExecuteResult execute_statement(Statement* statement, Table* table);

#endif
