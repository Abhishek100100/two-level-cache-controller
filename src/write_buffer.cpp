/**
 * write_buffer.cpp
 * Write buffer implementation
 */

 #include "write_buffer.h"
 #include "cache.h"  // For BLOCK_OFFSET_BITS and Cache definition
 #include <stdlib.h>
 #include <string.h>
 
 WriteBuffer* create_write_buffer() {
     WriteBuffer* wb = (WriteBuffer*)malloc(sizeof(WriteBuffer));
     if (!wb) return NULL;
     
     for (int i = 0; i < WRITE_BUFFER_SIZE; i++) {
         wb->entries[i].valid = false;
         wb->entries[i].dirty = false;
         wb->entries[i].address = 0;
         wb->entries[i].insertion_time = 0;
     }
     
     wb->num_entries = 0;
     wb->counter = 0;
     wb->write_backs = 0;
     wb->coalesced_writes = 0;
     
     return wb;
 }
 
 bool check_write_buffer(WriteBuffer* wb, uint32_t address, uint8_t* data) {
     uint32_t block_address = address & ~((1 << BLOCK_OFFSET_BITS) - 1);
     uint32_t offset = address & ((1 << BLOCK_OFFSET_BITS) - 1);
     
     for (int i = 0; i < WRITE_BUFFER_SIZE; i++) {
         if (wb->entries[i].valid && 
             (wb->entries[i].address & ~((1 << BLOCK_OFFSET_BITS) - 1)) == block_address) {
             if (data) {
                 memcpy(data, &wb->entries[i].data[offset], 1);
             }
             return true;
         }
     }
     
     return false;
 }
 
 void insert_write_buffer(WriteBuffer* wb, uint32_t address, uint8_t* data) {
     uint32_t block_address = address & ~((1 << BLOCK_OFFSET_BITS) - 1);
     uint32_t offset = address & ((1 << BLOCK_OFFSET_BITS) - 1);
     
     for (int i = 0; i < WRITE_BUFFER_SIZE; i++) {
         if (wb->entries[i].valid && 
             (wb->entries[i].address & ~((1 << BLOCK_OFFSET_BITS) - 1)) == block_address) {
             memcpy(&wb->entries[i].data[offset], data, 1);
             wb->entries[i].dirty = true;
             wb->entries[i].insertion_time = wb->counter++;
             wb->coalesced_writes++;
             return;
         }
     }
     
     if (wb->num_entries == WRITE_BUFFER_SIZE) {
         uint32_t oldest_idx = 0;
         uint64_t oldest_time = UINT64_MAX;
         
         for (int i = 0; i < WRITE_BUFFER_SIZE; i++) {
             if (wb->entries[i].insertion_time < oldest_time) {
                 oldest_time = wb->entries[i].insertion_time;
                 oldest_idx = i;
             }
         }
         
         wb->write_backs++;
         wb->entries[oldest_idx].valid = false;
         wb->num_entries--;
     }
     
     int empty_idx = -1;
     for (int i = 0; i < WRITE_BUFFER_SIZE; i++) {
         if (!wb->entries[i].valid) {
             empty_idx = i;
             break;
         }
     }
     
     if (empty_idx != -1) {
         wb->entries[empty_idx].valid = true;
         wb->entries[empty_idx].dirty = true;
         wb->entries[empty_idx].address = block_address;
         wb->entries[empty_idx].insertion_time = wb->counter++;
         
         memset(wb->entries[empty_idx].data, 0, WB_BLOCK_SIZE);
         memcpy(&wb->entries[empty_idx].data[offset], data, 1);
         
         wb->num_entries++;
     }
 }
 
 void flush_write_buffer(WriteBuffer* wb, Cache* cache) {
     for (int i = 0; i < WRITE_BUFFER_SIZE; i++) {
         if (wb->entries[i].valid && wb->entries[i].dirty) {
             cache_write(cache, wb->entries[i].address, wb->entries[i].data);
             wb->entries[i].dirty = false;
         }
     }
 }