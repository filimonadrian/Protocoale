#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void read_from_file(char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd <= 0) {
        perror("Can't read from this file");
    }
    char buffer[1024];

    int ret;
    while (ret = read(fd, buffer, sizeof(buffer))) {
        if (ret < 0) {
            perror("Can't read from this file!");
        }
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    close(fd);
}

int main(void) {

    char *filename = "test1.txt";

    read_from_file(filename);

    return 0;
}