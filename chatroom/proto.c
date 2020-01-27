#include <stdio.h>
#include <stdlib.h>
#include "proto.h"

void printSplash()
{
    FILE* fp;
    char s[256];

    if((fp=fopen("./aa_speed.txt","r"))== NULL){
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    while(fgets(s,256,fp) != NULL){
        printf("%s",s);
    }

    fclose(fp);
}
