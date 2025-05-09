/**
 * trace_parser.h
 * Trace parsing functions
 */

#ifndef TRACE_PARSER_H
#define TRACE_PARSER_H

#include "controller.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TRACE_INSTRUCTION,
    TRACE_LOAD,
    TRACE_STORE,
    TRACE_MODIFY
} TraceOperation;

typedef struct {
    TraceOperation op;
    uint32_t address;
    uint32_t size;
    uint8_t data[8];  // Maximum 8 bytes of data
} TraceEntry;

// Trace file functions
int read_usimm_trace(const char* filename, TraceEntry** traces, int* num_traces);
void process_usimm_trace(CacheController* controller, const char* filename);
void process_usimm_trace_enhanced(EnhancedCacheController* controller, const char* filename);

#endif // TRACE_PARSER_H