/**
 * replacement.h
 * Replacement policies implementation
 */

 #ifndef REPLACEMENT_H
 #define REPLACEMENT_H
 
 #include "cache.h"
 
 // Replacement policy implementations
 uint32_t get_victim_way(Cache* cache, uint32_t set_index);
 
 #endif // REPLACEMENT_H