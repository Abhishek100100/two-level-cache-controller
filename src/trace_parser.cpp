/**
 * trace_parser.cpp
 * Trace parsing functions
 */

#include "trace_parser.h"
#include "controller.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void hex_string_to_bytes(const char* hex_str, uint8_t* bytes, int num_bytes) {
    for (int i = 0; i < num_bytes; i++) {
        sscanf(hex_str + 2*i, "%2hhx", &bytes[i]);
    }
}

int read_usimm_trace(const char* filename, TraceEntry** traces, int* num_traces) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open trace file: %s\n", filename);
        return -1;
    }
    
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        count++;
    }
    
    *traces = (TraceEntry*)malloc(sizeof(TraceEntry) * count);
    if (!*traces) {
        fprintf(stderr, "Failed to allocate memory for traces\n");
        fclose(file);
        return -1;
    }
    
    rewind(file);
    int i = 0;
    while (fgets(line, sizeof(line), file) && i < count) {
        if (line[0] == '#' || line[0] == '\n') continue;
        
        char op;
        uint32_t address;
        int size = 0;
        char data_str[32] = {0};
        
        // Parse the line based on operation type
        int parsed = sscanf(line, "%c %x, %d, %s", &op, &address, &size, data_str);
        
        if (parsed < 2) continue;  // Skip malformed lines
        
        TraceEntry entry;
        entry.address = address;
        
        switch (toupper(op)) {
            case 'I':
                entry.op = TRACE_INSTRUCTION;
                entry.size = (parsed >= 3) ? size : 0;
                memset(entry.data, 0, sizeof(entry.data));
                break;
            case 'L':
                entry.op = TRACE_LOAD;
                entry.size = (parsed >= 3) ? size : 4;  // Default to 4 bytes
                memset(entry.data, 0, sizeof(entry.data));
                break;
            case 'S':
                entry.op = TRACE_STORE;
                entry.size = (parsed >= 3) ? size : 4;  // Default to 4 bytes
                if (parsed >= 4) {
                    hex_string_to_bytes(data_str, entry.data, entry.size);
                } else {
                    memset(entry.data, 0, sizeof(entry.data));
                }
                break;
            case 'M':
                entry.op = TRACE_MODIFY;
                entry.size = (parsed >= 3) ? size : 4;  // Default to 4 bytes
                if (parsed >= 4) {
                    hex_string_to_bytes(data_str, entry.data, entry.size);
                } else {
                    memset(entry.data, 0, sizeof(entry.data));
                }
                break;
            default:
                continue;  // Skip unknown operations
        }
        
        (*traces)[i++] = entry;
    }
    
    *num_traces = i;
    fclose(file);
    return 0;
}

void process_usimm_trace(CacheController* controller, const char* filename) {
    TraceEntry* traces = NULL;
    int num_traces = 0;
    
    if (read_usimm_trace(filename, &traces, &num_traces) < 0) {
        fprintf(stderr, "Failed to read trace file\n");
        return;
    }
    
    printf("Processing %d trace entries from %s\n", num_traces, filename);
    
    for (int i = 0; i < num_traces; i++) {
        TraceEntry entry = traces[i];
        
        switch (entry.op) {
            case TRACE_INSTRUCTION:
                // Instruction fetch - treat as read
                controller_read(controller, entry.address, NULL);
                break;
            case TRACE_LOAD:
                // Load operation - read
                {
                    uint8_t data[8];
                    controller_read(controller, entry.address, data);
                }
                break;
            case TRACE_STORE:
            case TRACE_MODIFY:
                // Store or modify operation - write
                controller_write(controller, entry.address, entry.data);
                break;
        }
        
        if ((i + 1) % 10000 == 0 || i == num_traces - 1) {
            printf("Processed %d/%d entries (%.1f%%)\n", 
                  i + 1, num_traces, (float)(i + 1) / num_traces * 100);
            
            if ((i + 1) % 100000 == 0) {
                print_controller_stats(controller);
            }
        }
    }
    
    // Print final statistics
    printf("\nFinal Statistics:\n");
    print_controller_stats(controller);
    
    free(traces);
}

void process_usimm_trace_enhanced(EnhancedCacheController* controller, const char* filename) {
    TraceEntry* traces = NULL;
    int num_traces = 0;
    
    if (read_usimm_trace(filename, &traces, &num_traces) < 0) {
        fprintf(stderr, "Failed to read trace file\n");
        return;
    }
    
    printf("Processing %d trace entries from %s with enhanced controller\n", num_traces, filename);
    
    for (int i = 0; i < num_traces; i++) {
        TraceEntry entry = traces[i];
        
        switch (entry.op) {
            case TRACE_INSTRUCTION:
                // Instruction fetch - treat as read
                {
                    uint8_t data[8];
                    enhanced_read(controller, entry.address, data);
                }
                break;
            case TRACE_LOAD:
                // Load operation - read
                {
                    uint8_t data[8];
                    enhanced_read(controller, entry.address, data);
                }
                break;
            case TRACE_STORE:
            case TRACE_MODIFY:
                // Store or modify operation - write
                enhanced_write(controller, entry.address, entry.data);
                break;
        }
        
        if ((i + 1) % 10000 == 0 || i == num_traces - 1) {
            printf("Processed %d/%d entries (%.1f%%)\n", 
                  i + 1, num_traces, (float)(i + 1) / num_traces * 100);
        }
    }
    
    flush_write_buffer(controller->write_buffer, controller->L1);
    print_enhanced_stats(controller);
    free(traces);
}