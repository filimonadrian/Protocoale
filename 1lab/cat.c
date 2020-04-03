#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> 
#include <errno.h>
#include <string.h>

int main(void)
{
    int fd;
    int copiat;
    char buf[1024];
    int i = 0;
    fd = open("test1.txt", O_RDONLY);
 
    while ((copiat = read(fd, buf, sizeof(buf)))) {
        if (copiat < 0)
            printf("Eroare la citire");
        printf("%s", buf);
        memset(buf, 0, sizeof(buf));
    }

    close(fd);
    return 0;
}