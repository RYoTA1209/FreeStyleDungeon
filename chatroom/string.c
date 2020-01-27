#include <stdio.h>
#include "string.h"

void str_trim_lf (char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void str_overwrite_stdout(char* name,char* id) {
    printf("\b%s【ルーム：%s】%s", name,id,"> ");
    fflush(stdout);
}