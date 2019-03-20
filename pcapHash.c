// Ale Lopez, Bailey Blum, and Horacio Lopez
// CSE34341 - Project4
// Due: 3/28/2019

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

int main(int argc, char* argv[])
{
    FILE *fp;
    fp = fopen("Dataset-Small.pcap", "r");

    /* Display the Magic Number and skip over the rest */
 
    uint32_t   theMagicNum;
 
    if(fread(&theMagicNum, 4, 1, fp) < 4) {
      // There was an error
    }
 
    printf("Magic Number was %X\n", theMagicNum);
 
    /* Jump through the rest (aka the other 20 bytes) to get past the global header */
    fseek(fp, 24, SEEK_CUR);


}
