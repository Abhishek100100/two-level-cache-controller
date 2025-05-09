/**
 * cache.h
 * Basic cache structures and functions
 */

 #ifndef CACHE_H
 #define CACHE_H
 
 #include <stdint.h>
 #include <stdbool.h>
 
 // Cache Configuration Parameters
 #define ADDR_BITS       32
 #define BLOCK_OFFSET_BITS 6
 
 typedef enum {
     LRU,
     RANDOM,
     FIFO
 } ReplacementPolicy;
 
 /**
  * Cache Line Structure
  */
 typedef struct {
    bool valid;
    bool dirty;
    uint32_t tag;
    uint64_t last_access_time;
    uint8_t data[]; // Flexible array member MUST be last
} CacheLine;
 
 /**
  * Cache Structure
  */
 typedef struct {
     CacheLine** sets;
     uint32_t num_sets;
     uint32_t associativity;
     uint32_t block_size;
     uint32_t index_bits;
     uint32_t tag_bits;
     ReplacementPolicy policy;
     uint64_t access_counter;
     
     // Statistics
     uint64_t accesses;
     uint64_t hits;
     uint64_t misses;
     uint64_t write_backs;
 } Cache;
 
 // Cache creation/destruction
 Cache* create_cache(uint32_t size, uint32_t block_size, uint32_t associativity, 
                    uint32_t index_bits, uint32_t tag_bits, ReplacementPolicy policy);
 void free_cache(Cache* cache);
 
 // Core cache operations
 bool cache_read(Cache* cache, uint32_t address, uint8_t* data);
 bool cache_write(Cache* cache, uint32_t address, uint8_t* data);
 void cache_insert(Cache* cache, uint32_t address, uint8_t* data, bool is_dirty);
 
 // Address manipulation
 uint32_t get_tag(uint32_t address, uint32_t tag_bits, uint32_t index_bits);
 uint32_t get_index(uint32_t address, uint32_t index_bits);
 uint32_t get_offset(uint32_t address);
 uint32_t reconstruct_address(uint32_t tag, uint32_t index, uint32_t offset, uint32_t index_bits);
 
 // Statistics
 void print_cache_stats(Cache* cache, const char* name);
 
 #endif // CACHE_H