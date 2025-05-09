/**
 * controller.cpp
 * Cache controller implementation
 */

 #include "controller.h"
 #include "replacement.h"
 #include "write_buffer.h"
 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>
 
 CacheController* create_cache_controller() {
     CacheController* controller = (CacheController*)malloc(sizeof(CacheController));
     if (!controller) return NULL;
     
     controller->L1 = create_cache(L1_SIZE, L1_BLOCK_SIZE, L1_ASSOCIATIVITY, 
                                  L1_INDEX_BITS, L1_TAG_BITS, LRU);
     if (!controller->L1) {
         free(controller);
         return NULL;
     }
                                  
     controller->L2 = create_cache(L2_SIZE, L2_BLOCK_SIZE, L2_ASSOCIATIVITY, 
                                  L2_INDEX_BITS, L2_TAG_BITS, LRU);
     if (!controller->L2) {
         free_cache(controller->L1);
         free(controller);
         return NULL;
     }
     
     controller->memory_accesses = 0;
     return controller;
 }
 
 void free_cache_controller(CacheController* controller) {
     if (controller) {
         free_cache(controller->L1);
         free_cache(controller->L2);
         free(controller);
     }
 }
 
 EnhancedCacheController* create_enhanced_controller() {
     EnhancedCacheController* controller = (EnhancedCacheController*)malloc(sizeof(EnhancedCacheController));
     if (!controller) return NULL;
     
     controller->L1 = create_cache(L1_SIZE, L1_BLOCK_SIZE, L1_ASSOCIATIVITY, 
                                  L1_INDEX_BITS, L1_TAG_BITS, LRU);
     if (!controller->L1) {
         free(controller);
         return NULL;
     }
                                  
     controller->L2 = create_cache(L2_SIZE, L2_BLOCK_SIZE, L2_ASSOCIATIVITY, 
                                  L2_INDEX_BITS, L2_TAG_BITS, LRU);
     if (!controller->L2) {
         free_cache(controller->L1);
         free(controller);
         return NULL;
     }
     
     controller->write_buffer = create_write_buffer();
     if (!controller->write_buffer) {
         free_cache(controller->L1);
         free_cache(controller->L2);
         free(controller);
         return NULL;
     }
     
     controller->memory_accesses = 0;
     return controller;
 }
 
 void free_enhanced_controller(EnhancedCacheController* controller) {
     if (controller) {
         free_cache(controller->L1);
         free_cache(controller->L2);
         free(controller->write_buffer);
         free(controller);
     }
 }
 
 void handle_eviction(CacheController* controller, Cache* cache, uint32_t set_index, uint32_t way) {
     CacheLine* line = &cache->sets[set_index][way];
     
     if (line->valid && line->dirty) {
         uint32_t address = reconstruct_address(line->tag, set_index, 0, cache->index_bits);
         
         if (cache == controller->L1) {
             cache_write(controller->L2, address, line->data);
         } else if (cache == controller->L2) {
             controller->memory_accesses++;
         }
         
         cache->write_backs++;
     }
     
     line->valid = false;
     line->dirty = false;
 }
 
bool controller_read(CacheController* controller, uint32_t address, uint8_t* data) {
    // First try L1
    if (cache_read(controller->L1, address, data)) {
        return true;
    }
    
    // L1 miss - try L2
    uint8_t block_data[L2_BLOCK_SIZE];
    if (cache_read(controller->L2, address, block_data)) {
        // L2 hit - allocate in L1
        uint32_t block_address = address & ~((1 << BLOCK_OFFSET_BITS) - 1);
        uint32_t l1_set_index = get_index(address, controller->L1->index_bits);
        
        // Check if we need to evict from L1
        bool need_eviction = true;
        for (uint32_t i = 0; i < controller->L1->associativity; i++) {
            if (!controller->L1->sets[l1_set_index][i].valid) {
                need_eviction = false;
                break;
            }
        }
        
        if (need_eviction) {
            uint32_t victim_way = get_victim_way(controller->L1, l1_set_index);
            handle_eviction(controller, controller->L1, l1_set_index, victim_way);
        }
        
        cache_insert(controller->L1, block_address, block_data, false);
        
        if (data) {
            uint32_t offset = get_offset(address);
            memcpy(data, &block_data[offset], 1);
        }
        
        return true;
    }
    
    // Both L1 and L2 missed - access main memory
    controller->memory_accesses++;
    
    // Simulate memory read
    uint8_t memory_data[L2_BLOCK_SIZE];
    memset(memory_data, 0xAA, L2_BLOCK_SIZE); // Simulate memory data
    
    uint32_t block_address = address & ~((1 << BLOCK_OFFSET_BITS) - 1);
    
    // First insert into L2
    uint32_t l2_set_index = get_index(address, controller->L2->index_bits);
    bool need_eviction = true;
    for (uint32_t i = 0; i < controller->L2->associativity; i++) {
        if (!controller->L2->sets[l2_set_index][i].valid) {
            need_eviction = false;
            break;
        }
    }
    
    if (need_eviction) {
        uint32_t victim_way = get_victim_way(controller->L2, l2_set_index);
        handle_eviction(controller, controller->L2, l2_set_index, victim_way);
    }
    
    cache_insert(controller->L2, block_address, memory_data, false);
    
    // Then insert into L1
    uint32_t l1_set_index = get_index(address, controller->L1->index_bits);
    need_eviction = true;
    for (uint32_t i = 0; i < controller->L1->associativity; i++) {
        if (!controller->L1->sets[l1_set_index][i].valid) {
            need_eviction = false;
            break;
        }
    }
    
    if (need_eviction) {
        uint32_t victim_way = get_victim_way(controller->L1, l1_set_index);
        handle_eviction(controller, controller->L1, l1_set_index, victim_way);
    }
    
    cache_insert(controller->L1, block_address, memory_data, false);
    
    if (data) {
        uint32_t offset = get_offset(address);
        memcpy(data, &memory_data[offset], 1);
    }
    
    return false;
}

 bool controller_write(CacheController* controller, uint32_t address, uint8_t* data) {
     if (cache_write(controller->L1, address, data)) {
         return true;
     }
     
     if (cache_write(controller->L2, address, data)) {
         uint32_t block_address = address & ~((1 << BLOCK_OFFSET_BITS) - 1);
         uint32_t l2_tag = get_tag(address, controller->L2->tag_bits, controller->L2->index_bits);
         uint32_t l2_index = get_index(address, controller->L2->index_bits);
         
         uint8_t block_data[L2_BLOCK_SIZE];
         for (uint32_t i = 0; i < controller->L2->associativity; i++) {
             CacheLine* line = &controller->L2->sets[l2_index][i];
             if (line->valid && line->tag == l2_tag) {
                 memcpy(block_data, line->data, L2_BLOCK_SIZE);
                 break;
             }
         }
         
         uint32_t l1_set_index = get_index(address, controller->L1->index_bits);
         bool need_eviction = true;
         for (uint32_t i = 0; i < controller->L1->associativity; i++) {
             if (!controller->L1->sets[l1_set_index][i].valid) {
                 need_eviction = false;
                 break;
             }
         }
         
         if (need_eviction) {
             uint32_t victim_way = get_victim_way(controller->L1, l1_set_index);
             handle_eviction(controller, controller->L1, l1_set_index, victim_way);
         }
         
         cache_insert(controller->L1, block_address, block_data, true);
         return true;
     }
     
     controller->memory_accesses++;
     uint8_t memory_data[L2_BLOCK_SIZE];
     memset(memory_data, 0xAA, L2_BLOCK_SIZE);
     
     uint32_t offset = get_offset(address);
     memcpy(&memory_data[offset], data, 1);
     
     uint32_t block_address = address & ~((1 << BLOCK_OFFSET_BITS) - 1);
     uint32_t l2_set_index = get_index(address, controller->L2->index_bits);
     
     bool need_eviction = true;
     for (uint32_t i = 0; i < controller->L2->associativity; i++) {
         if (!controller->L2->sets[l2_set_index][i].valid) {
             need_eviction = false;
             break;
         }
     }
     
     if (need_eviction) {
         uint32_t victim_way = get_victim_way(controller->L2, l2_set_index);
         handle_eviction(controller, controller->L2, l2_set_index, victim_way);
     }
     
     cache_insert(controller->L2, block_address, memory_data, true);
     
     uint32_t l1_set_index = get_index(address, controller->L1->index_bits);
     need_eviction = true;
     for (uint32_t i = 0; i < controller->L1->associativity; i++) {
         if (!controller->L1->sets[l1_set_index][i].valid) {
             need_eviction = false;
             break;
         }
     }
     
     if (need_eviction) {
         uint32_t victim_way = get_victim_way(controller->L1, l1_set_index);
         handle_eviction(controller, controller->L1, l1_set_index, victim_way);
     }
     
     cache_insert(controller->L1, block_address, memory_data, true);
     return false;
 }
 
 bool enhanced_read(EnhancedCacheController* controller, uint32_t address, uint8_t* data) {
     if (check_write_buffer(controller->write_buffer, address, data)) {
         return true;
     }
     
     if (cache_read(controller->L1, address, data)) {
         return true;
     }
     
     uint8_t block_data[L2_BLOCK_SIZE];
     if (cache_read(controller->L2, address, block_data)) {
         uint32_t block_address = address & ~((1 << BLOCK_OFFSET_BITS) - 1);
         uint32_t l1_set_index = get_index(address, controller->L1->index_bits);
         
         bool need_eviction = true;
         for (uint32_t i = 0; i < controller->L1->associativity; i++) {
             if (!controller->L1->sets[l1_set_index][i].valid) {
                 need_eviction = false;
                 break;
             }
         }
         
         if (need_eviction) {
             uint32_t victim_way = get_victim_way(controller->L1, l1_set_index);
             CacheLine* victim_line = &controller->L1->sets[l1_set_index][victim_way];
             if (victim_line->valid && victim_line->dirty) {
                 uint32_t victim_addr = reconstruct_address(victim_line->tag, l1_set_index, 
                                                          0, controller->L1->index_bits);
                 cache_write(controller->L2, victim_addr, victim_line->data);
                 controller->L1->write_backs++;
             }
             victim_line->valid = false;
         }
         
         cache_insert(controller->L1, block_address, block_data, false);
         
         if (data) {
             uint32_t offset = get_offset(address);
             memcpy(data, &block_data[offset], 1);
         }
         
         return true;
     }
     
     controller->memory_accesses++;
     uint8_t memory_data[L2_BLOCK_SIZE];
     memset(memory_data, 0xAA, L2_BLOCK_SIZE);
     
     uint32_t block_address = address & ~((1 << BLOCK_OFFSET_BITS) - 1);
     uint32_t l2_set_index = get_index(address, controller->L2->index_bits);
     
     bool need_eviction = true;
     for (uint32_t i = 0; i < controller->L2->associativity; i++) {
         if (!controller->L2->sets[l2_set_index][i].valid) {
             need_eviction = false;
             break;
         }
     }
     
     if (need_eviction) {
         uint32_t victim_way = get_victim_way(controller->L2, l2_set_index);
         CacheLine* victim_line = &controller->L2->sets[l2_set_index][victim_way];
         if (victim_line->valid && victim_line->dirty) {
             controller->L2->write_backs++;
         }
         victim_line->valid = false;
     }
     
     cache_insert(controller->L2, block_address, memory_data, false);
     
     uint32_t l1_set_index = get_index(address, controller->L1->index_bits);
     need_eviction = true;
     for (uint32_t i = 0; i < controller->L1->associativity; i++) {
         if (!controller->L1->sets[l1_set_index][i].valid) {
             need_eviction = false;
             break;
         }
     }
     
     if (need_eviction) {
         uint32_t victim_way = get_victim_way(controller->L1, l1_set_index);
         CacheLine* victim_line = &controller->L1->sets[l1_set_index][victim_way];
         if (victim_line->valid && victim_line->dirty) {
             uint32_t victim_addr = reconstruct_address(victim_line->tag, l1_set_index, 
                                                      0, controller->L1->index_bits);
             cache_write(controller->L2, victim_addr, victim_line->data);
             controller->L1->write_backs++;
         }
         victim_line->valid = false;
     }
     
     cache_insert(controller->L1, block_address, memory_data, false);
     
     if (data) {
         uint32_t offset = get_offset(address);
         memcpy(data, &memory_data[offset], 1);
     }
     
     return false;
 }
 
 bool enhanced_write(EnhancedCacheController* controller, uint32_t address, uint8_t* data) {
     insert_write_buffer(controller->write_buffer, address, data);
     
     if (controller->write_buffer->num_entries >= WRITE_BUFFER_SIZE * 0.75) {
         flush_write_buffer(controller->write_buffer, controller->L1);
     }
     
     return true;
 }
 
 void print_controller_stats(CacheController* controller) {
    print_cache_stats(controller->L1, "L1 Cache");
    print_cache_stats(controller->L2, "L2 Cache");
    printf("Memory accesses: %llu\n", controller->memory_accesses);
}

void print_enhanced_stats(EnhancedCacheController* controller) {
    print_cache_stats(controller->L1, "L1 Cache");
    print_cache_stats(controller->L2, "L2 Cache");
    
    printf("===== Write Buffer Statistics =====\n");
    printf("Write-backs: %llu\n", controller->write_buffer->write_backs);
    printf("Coalesced writes: %llu\n", controller->write_buffer->coalesced_writes);
    printf("==================================\n");
    
    printf("Memory accesses: %llu\n", controller->memory_accesses);
}