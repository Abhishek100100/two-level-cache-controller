/**
 * cache.cpp
 * Cache implementation
 */

 #include "cache.h"
 #include "replacement.h"  // Add this line
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 
 Cache* create_cache(uint32_t size, uint32_t block_size, uint32_t associativity, 
    uint32_t index_bits, uint32_t tag_bits, ReplacementPolicy policy) {
// Calculate the actual size needed per cache line
size_t cacheline_size = sizeof(CacheLine) + block_size;

Cache* cache = (Cache*)calloc(1, sizeof(Cache));
if (!cache) return NULL;

cache->num_sets = size / (block_size * associativity);
cache->associativity = associativity;
cache->block_size = block_size;
cache->index_bits = index_bits;
cache->tag_bits = tag_bits;
cache->policy = policy;
cache->access_counter = 0;

// Initialize statistics
cache->accesses = 0;
cache->hits = 0;
cache->misses = 0;
cache->write_backs = 0;

// Allocate sets
cache->sets = (CacheLine**)calloc(cache->num_sets, sizeof(CacheLine*));
if (!cache->sets) {
free(cache);
return NULL;
}

// Allocate cache lines for each set
for (uint32_t i = 0; i < cache->num_sets; i++) {
// Allocate all ways contiguously
cache->sets[i] = (CacheLine*)calloc(associativity, cacheline_size);
if (!cache->sets[i]) {
// Clean up previously allocated memory
for (uint32_t j = 0; j < i; j++) {
 free(cache->sets[j]);
}
free(cache->sets);
free(cache);
return NULL;
}

// Initialize each cache line
for (uint32_t j = 0; j < associativity; j++) {
CacheLine* line = (CacheLine*)((char*)cache->sets[i] + j * cacheline_size);
line->valid = false;
line->dirty = false;
line->tag = 0;
line->last_access_time = 0;
memset(line->data, 0, block_size);
}
}

return cache;
}

 void free_cache(Cache* cache) {
     if (cache) {
         if (cache->sets) {
             for (uint32_t i = 0; i < cache->num_sets; i++) {
                 free(cache->sets[i]);
             }
             free(cache->sets);
         }
         free(cache);
     }
 }
 
bool cache_read(Cache* cache, uint32_t address, uint8_t* data) {
    if (!cache || !cache->sets) return false;
    
    cache->accesses++;  // Move this to the start of the function
    
    uint32_t tag = get_tag(address, cache->tag_bits, cache->index_bits);
    uint32_t set_index = get_index(address, cache->index_bits);
    uint32_t offset = get_offset(address);
    
    if (set_index >= cache->num_sets) return false;
    
    size_t cacheline_size = sizeof(CacheLine) + cache->block_size;
    
    for (uint32_t i = 0; i < cache->associativity; i++) {
        CacheLine* line = (CacheLine*)((char*)cache->sets[set_index] + i * cacheline_size);
        if (line->valid && line->tag == tag) {
            // Cache hit
            cache->hits++;
            line->last_access_time = cache->access_counter++;
            
            if (data && offset < cache->block_size) {
                memcpy(data, &line->data[offset], 1);
            }
            return true;
        }
    }
    
    // Cache miss
    cache->misses++;
    return false;
}
 
 bool cache_write(Cache* cache, uint32_t address, uint8_t* data) {
     cache->accesses++;
     
     uint32_t tag = get_tag(address, cache->tag_bits, cache->index_bits);
     uint32_t set_index = get_index(address, cache->index_bits);
     uint32_t offset = get_offset(address);
     
     // Check if we have a hit
     for (uint32_t i = 0; i < cache->associativity; i++) {
         CacheLine* line = &cache->sets[set_index][i];
         
         if (line->valid && line->tag == tag) {
             // Cache hit
             cache->hits++;
             line->last_access_time = cache->access_counter++;
             line->dirty = true;
             
             // Write data
             if (data) {
                 memcpy(&line->data[offset], data, 1);
             }
             
             return true;
         }
     }
     
     // Cache miss
     cache->misses++;
     return false;
 }
 
 void cache_insert(Cache* cache, uint32_t address, uint8_t* data, bool is_dirty) {
     uint32_t tag = get_tag(address, cache->tag_bits, cache->index_bits);
     uint32_t set_index = get_index(address, cache->index_bits);
     
     // Find an empty way or select a victim
     uint32_t way = cache->associativity;
     for (uint32_t i = 0; i < cache->associativity; i++) {
         if (!cache->sets[set_index][i].valid) {
             way = i;
             break;
         }
     }
     
     // If no invalid line found, use replacement policy
     if (way == cache->associativity) {
         way = get_victim_way(cache, set_index);
     }
     
     // Update cache line
     cache->sets[set_index][way].valid = true;
     cache->sets[set_index][way].dirty = is_dirty;
     cache->sets[set_index][way].tag = tag;
     cache->sets[set_index][way].last_access_time = cache->access_counter++;
     
     // Copy data
     if (data) {
         memcpy(cache->sets[set_index][way].data, data, cache->block_size);
     }
 }
 
 uint32_t get_tag(uint32_t address, uint32_t tag_bits, uint32_t index_bits) {
    (void)tag_bits;  // Mark as unused to avoid warnings
    return address >> (BLOCK_OFFSET_BITS + index_bits);
}
 
 uint32_t get_index(uint32_t address, uint32_t index_bits) {
     uint32_t mask = (1 << index_bits) - 1;
     return (address >> BLOCK_OFFSET_BITS) & mask;
 }
 
 uint32_t get_offset(uint32_t address) {
     return address & ((1 << BLOCK_OFFSET_BITS) - 1);
 }
 
 uint32_t reconstruct_address(uint32_t tag, uint32_t index, uint32_t offset, uint32_t index_bits) {
     uint32_t address = tag;
     address = (address << index_bits) | index;
     address = (address << BLOCK_OFFSET_BITS) | offset;
     return address;
 }
 
void print_cache_stats(Cache* cache, const char* name) {
    printf("===== %s Statistics =====\n", name);
    printf("Total accesses: %llu\n", cache->accesses);
    printf("Hits: %llu (%.2f%%)\n", cache->hits, 
          cache->accesses > 0 ? (double)cache->hits / cache->accesses * 100 : 0);
    printf("Misses: %llu (%.2f%%)\n", cache->misses, 
          cache->accesses > 0 ? (double)cache->misses / cache->accesses * 100 : 0);
    printf("Hit rate: %.2f%%\n",
          cache->accesses > 0 ? (double)cache->hits / cache->accesses * 100 : 0);
    printf("Miss rate: %.2f%%\n",
          cache->accesses > 0 ? (double)cache->misses / cache->accesses * 100 : 0);
    printf("Write-backs: %llu\n", cache->write_backs);
    printf("==========================\n");
}