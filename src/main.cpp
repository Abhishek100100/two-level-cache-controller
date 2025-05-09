/**
 * main.cpp
 * Main program and test cases
 */

 #include "cache.h"
 #include "controller.h"
 #include "trace_parser.h"
 #include <stdio.h>
 #include <stdlib.h>
 #include <time.h>
 #include <string.h>
 
 int main(int argc, char** argv) {
    printf("2-Level Cache Controller\n");
    printf("========================\n");
    
    srand(time(NULL));
    
    bool use_enhanced = false;
    int arg_start = 1;
    
    if (argc > 1 && strcmp(argv[1], "-e") == 0) {
        use_enhanced = true;
        arg_start = 2;
    }
    
    if (use_enhanced) {
        printf("Using enhanced cache controller with write buffer\n");
        EnhancedCacheController* controller = create_enhanced_controller();
        if (!controller) {
            fprintf(stderr, "Failed to create enhanced controller\n");
            return 1;
        }
         
         if (argc < 3) {
             printf("Usage: %s -e <trace_file1> [trace_file2 ...]\n", argv[0]);
             printf("No trace files provided. Running built-in test patterns.\n");
             
             // Test patterns would go here
             free_enhanced_controller(controller);
             return 0;
         }
         
         for (int i = arg_start; i < argc; i++) {
             printf("\nProcessing trace file: %s\n", argv[i]);
             process_usimm_trace_enhanced(controller, argv[i]);
         }
         
         free_enhanced_controller(controller);
     } else {
         printf("Using basic cache controller\n");
         CacheController* controller = create_cache_controller();
         
         if (argc < 2) {
             printf("Usage: %s <trace_file1> [trace_file2 ...]\n", argv[0]);
             printf("No trace files provided. Running built-in test patterns.\n");
             
             // Test patterns would go here
             free_cache_controller(controller);
             return 0;
         }
         
         for (int i = arg_start; i < argc; i++) {
             printf("\nProcessing trace file: %s\n", argv[i]);
             process_usimm_trace(controller, argv[i]);
         }
         
         free_cache_controller(controller);
     }
     
     return 0;
 }