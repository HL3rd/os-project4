// Ale Lopez, Bailey Blum, and Horacio Lopez
// CSE34341 - Project 4
// Due: 3/28/2019

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <getopt.h> 
#include "functions.h"

int main (int argc, char* argv[]) {

    int c;
    int level = 2; // is no level if specified, we will run using Level 2
    int threads = 2; //TODO: specify a default value for threads
    int min_threads = 2; // minimum number of allowed threads
    int max_files = 10; // maximum number of processed files

    int option_index = 0;
    static struct option long_options[] = 
    {
        {"level",   required_argument, NULL,  'l'},
        {"thread",  required_argument, NULL,  't'},
        {NULL,      0,                 NULL,    0}
    };

    int files = argc - 1;

    // Parse command line arguments
    while ((c = getopt_long_only(argc, argv, ":l:t:", long_options, &option_index)) != -1) {
        switch(c) {
			case 'l':
                level = atoi(optarg);
                files = files - 2;
                printf("level: %d\n", level);
                break;
            case 't':
                threads = atoi(optarg);
                files = files - 2;
                printf("threads: %d\n", threads);
                break;
		}
	}

    // Check number of threads
    if (threads < min_threads) {
        printf("error: Minimum of two threads allowed.\n");
        exit(1);
    }
    
    // Save files to be processed in an array
    char *filenames[max_files];
    int i;
    for (i = 0; i < files; i++) {
        filenames[i] = argv[argc-files + i];
    }

    uint32_t initvalue = 4; // value used for calculating hash
    char* test = "bailey";
    uint32_t rtnval = hashlittle(test, sizeof(test), initvalue);
    printf("hash: %d\n", rtnval);
    
}