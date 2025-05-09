/**
 * write_buffer.h
 * Write buffer implementation
 */

 #ifndef WRITE_BUFFER_H
 #define WRITE_BUFFER_H
 
 #include "cache.h"  // Add this to get Cache definition and BLOCK_OFFSET_BITS
 
 #define WRITE_BUFFER_SIZE 16
 #define WB_BLOCK_SIZE 64  // Same as L1_BLOCK_SIZE
 
 typedef struct {
     bool valid;
     bool dirty;
     uint32_t address;
     uint8_t data[WB_BLOCK_SIZE];
     uint64_t insertion_time;
 } WriteBufferEntry;
 
 typedef struct {
     WriteBufferEntry entries[WRITE_BUFFER_SIZE];
     uint32_t num_entries;
     uint64_t counter;
     
     // Statistics
     uint64_t write_backs;
     uint64_t coalesced_writes;
 } WriteBuffer;
 
 WriteBuffer* create_write_buffer();
 bool check_write_buffer(WriteBuffer* wb, uint32_t address, uint8_t* data);
 void insert_write_buffer(WriteBuffer* wb, uint32_t address, uint8_t* data);
 void flush_write_buffer(WriteBuffer* wb, Cache* cache);
 
 #endif // WRITE_BUFFER_H