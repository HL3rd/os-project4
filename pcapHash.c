// Ale Lopez, Bailey Blum, and Horacio Lopez
// CSE34341 - Project 4
// Due: 3/28/2019

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

struct PacketHolder {
    int bIsValid;           // 0 if no, 1 if yes
    char byData[2400];      // The actual packet data
    uint32 nHash;           // Hash of the packet contents
};

void DumpAllPacketLengths (FILE *fp)
{
    /* We are going to assume that fp is just after the global header */
    uint32_t     nPacketLength;
    char         theData[2400];
    char         theHashData[2348];
    uint32_t     choppedPacketLength;

    while(!feof(fp)) {
        /* Skip the ts_sec field */
        fseek(fp, 4, SEEK_CUR);

        /* Skip the ts_usec field */
        fseek(fp, 4, SEEK_CUR);

        /* Read in the incl_len field */
        fread(&nPacketLength, 4, 1, fp);

        /* Skip the orig_len field */
        fseek(fp, 4, SEEK_CUR);

        /* Let’s do a sanity check before reading */
        if(nPacketLength < 128) {
            printf("Skipping %d bytes ahead - packet is too small\n", nPacketLength);
            fseek(fp, nPacketLength, SEEK_CUR);
        }
        else if (nPacketLength > 2400) {
            printf("Skipping %d bytes ahead - packet is too big\n", nPacketLength);
            fseek(fp, nPacketLength, SEEK_CUR);
        }
        else {

            /* Might not be a bad idea to pay attention to this return value */
            fread(theData, 1, nPacketLength, fp); //this is the packet.

            printf("Packet length was %d\n", nPacketLength);
            printf("Orignal Packet----------\n");
            for (int i = 0; i < nPacketLength; i++) {
                printf("%hhx", theData[i]);
            }
            printf("\n");
            /* This is the value we will want to eliminate the first 52 bytes of to then be hashed. */
            int j = 0;
            choppedPacketLength = nPacketLength - 52;
            for (int i = 52; i < nPacketLength; i++) {
                theHashData[j] = theData[i];
                j++;
            }
            printf("Chopped packet length was %d\n", choppedPacketLength);
            printf("Chopped Packet----------\n");
            for (int i = 0; i < choppedPacketLength; i++) {
                printf("%hhx", theHashData[i]);
            }
            printf("\n");

        }

        /* At this point, we have read the packet and are onto the next one */
    }
}

char checkPacketsForDuplicates() {

    struct PacketHolder g_MyBigTable[30000];


}

int main(int argc, char* argv[])
{
    FILE *fp;
    fp = fopen("Dataset-Small.pcap", "r+");

    /* Display the Magic Number and skip over the rest */

    uint32_t   theMagicNum;

    if(fread(&theMagicNum, 4, 1, fp) < 4) {
      // There was an error
    }

    printf("Magic Number was %X\n", theMagicNum);

    /* Jump through the rest (aka the other 20 bytes) to get past the global header */
    fseek(fp, 24, SEEK_CUR);

    DumpAllPacketLengths(fp);

}
