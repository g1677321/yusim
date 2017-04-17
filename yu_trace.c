#include "yu_trace.h"

void traceFile2print(FILE *fin) {
    double arrivalTime;
    unsigned devno, blkno, reqSize, reqFlag, userno;
	while(!feof(fin)) {
        fscanf(fin, "%lf%u%u%u%u%u", &arrivalTime, &devno, &blkno, &reqSize, &reqFlag, &userno);
    	fprintf(stderr, "arrivalTime=%lf\ndevno=      %u\nblkno=      %u\nreqSize=    %u\nreqFlag=    %u\nuserno=     %u\n", arrivalTime, devno, blkno, reqSize, reqFlag, userno);
    	getchar();
    }
}

void traceFile2max(FILE *fin) {
    unsigned long maxBlkno = 0;
    double arrivalTime;
    unsigned long devno, blkno, reqSize, reqFlag, userno;
    while(!feof(fin)) {
        fscanf(fin, "%lf%lu%lu%lu%lu%lu", &arrivalTime, &devno, &blkno, &reqSize, &reqFlag, &userno);
        if (maxBlkno < blkno + reqSize - 1)
            maxBlkno = blkno + reqSize - 1;
        //fprintf(stderr, "arrivalTime=%lf\ndevno=      %u\nblkno=      %u\nreqSize=    %u\nreqFlag=    %u\nuserno=     %u\n", arrivalTime, devno, blkno, reqSize, reqFlag, userno);
        //getchar();
    }
    printf("maxBlkno = %lu\n", maxBlkno);
}

int main(int argc, char const *argv[]) {
	printf("This is a file to trace workload.\n");

    // File IO describer
    FILE *fin;

    double arrivalTime;
    unsigned devno, blkno, reqSize, reqFlag, userno;

    fin = fopen(argv[1], "r");
    //fout = fopen(argv[2], "w");
 
    if (argc != 2) {
        printf("Used:%s inputTrace\n", argv[0]);
        exit(1);
    }
 
    if (!fin) {
        printf("Input file open error...\n");
        exit(1);
    }
    
    //if (!fout) {
    //    printf("Output file open error...\n");
    //    exit(1);
    //}
    

    //traceFile2print(fin);
    traceFile2max(fin);

    fclose(fin);
    //fclose(fout);
    return 0;
}
