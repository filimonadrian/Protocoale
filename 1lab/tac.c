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
    char c;
    int i = 0;
    int offset = 0;
    fd = open("test1.txt", O_RDONLY);
    
    while ((copiat = read(fd, buf, sizeof(buf)))) {
        if (copiat < 0)
            printf("Eroare la citire");
        memset(buf, 0, sizeof(buf));
    }

    char p[100];
    offset = lseek(fd, -1, SEEK_END);
    i = 0;
    while (offset > 0) {     
        read(fd, &c, 1); 
        p[i] = c;
        i++;
        if (p[i - 1] == '\n'){
           // strrev(p);
            p[i] = '\0';           
            printf("%s", p);
            i = 0;
        }
        //printf("%c", c);
        lseek(fd, -2, SEEK_CUR); 
        offset--;    
    }
    p[i] = '\0';       
    printf("%s", p);
    //i = 0;
    //printf("%s", p);
    close(fd);
    return 0;
}
