/**
 * replacement.cpp
 * Replacement policies implementation
 */

 #include "replacement.h"
 #include <stdlib.h>
 #include <time.h>
 
 uint32_t get_victim_way(Cache* cache, uint32_t set_index) {
     uint32_t victim_way = 0;
     
     switch (cache->policy) {
         case LRU: {
             uint64_t min_access_time = UINT64_MAX;
             for (uint32_t i = 0; i < cache->associativity; i++) {
                 if (!cache->sets[set_index][i].valid) {
                     return i;
                 }
                 if (cache->sets[set_index][i].last_access_time < min_access_time) {
                     min_access_time = cache->sets[set_index][i].last_access_time;
                     victim_way = i;
                 }
             }
             break;
         }
         case RANDOM:
             victim_way = rand() % cache->associativity;
             break;
         case FIFO:
             // Using LRU as fallback for FIFO
             victim_way = get_victim_way(cache, set_index);
             break;
     }
     
     return victim_way;
 }