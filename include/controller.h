/**
 * controller.h
 * Cache controller structures and functions
 */

 #ifndef CONTROLLER_H
 #define CONTROLLER_H
 
 #include "cache.h"
 #include "write_buffer.h"
 
 // Cache sizes
 #define L1_SIZE         (64*1024)
 #define L1_BLOCK_SIZE   64
 #define L1_ASSOCIATIVITY 8
 #define L1_SETS         (L1_SIZE/(L1_BLOCK_SIZE*L1_ASSOCIATIVITY))
 #define L1_INDEX_BITS   7
 #define L1_TAG_BITS     (ADDR_BITS - L1_INDEX_BITS - BLOCK_OFFSET_BITS)
 
 #define L2_SIZE         (256*1024)
 #define L2_BLOCK_SIZE   64
 #define L2_ASSOCIATIVITY 16
 #define L2_SETS         (L2_SIZE/(L2_BLOCK_SIZE*L2_ASSOCIATIVITY))
 #define L2_INDEX_BITS   5
 #define L2_TAG_BITS     (ADDR_BITS - L2_INDEX_BITS - BLOCK_OFFSET_BITS)
 
 /**
  * Two-level Cache Controller
  */
 typedef struct {
     Cache* L1;
     Cache* L2;
     uint64_t memory_accesses;
 } CacheController;
 
 /**
  * Enhanced Cache Controller with Write Buffer
  */
 typedef struct {
     Cache* L1;
     Cache* L2;
     WriteBuffer* write_buffer;
     uint64_t memory_accesses;
 } EnhancedCacheController;
 
 // Controller creation/destruction
 CacheController* create_cache_controller();
 void free_cache_controller(CacheController* controller);
 EnhancedCacheController* create_enhanced_controller();
 void free_enhanced_controller(EnhancedCacheController* controller);
 
 // Multi-level operations
 bool controller_read(CacheController* controller, uint32_t address, uint8_t* data);
 bool controller_write(CacheController* controller, uint32_t address, uint8_t* data);
 bool enhanced_read(EnhancedCacheController* controller, uint32_t address, uint8_t* data);
 bool enhanced_write(EnhancedCacheController* controller, uint32_t address, uint8_t* data);
 
 // Eviction handling
 void handle_eviction(CacheController* controller, Cache* cache, uint32_t set_index, uint32_t way);
 
 // Statistics
 void print_controller_stats(CacheController* controller);
 void print_enhanced_stats(EnhancedCacheController* controller);
 
 #endif // CONTROLLER_H