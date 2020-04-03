#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

int main(int argc, char **argv){

    printf("%s ", argv[1]);
    int filedesc = open("testfile.txt", O_CREAT | O_WRONLY | O_APPEND);

    if (filedesc < 0) {
        return -1;
    }
    

    for (int i = 0; i < 5; i++){
        if (write(filedesc, "This will be output to testfile.txt\n", 36) != 36) {
            write(2, "There was an error writing to testfile.txt\n", 43);
            return -1;
        }
    }
 
    return 0;
}