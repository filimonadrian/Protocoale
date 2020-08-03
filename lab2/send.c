#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000


int main(int argc, char** argv){
  init(HOST, PORT);
  msg t;
  pkt p;

  int fd;
  int check;

  fd = open("file", O_RDONLY);
  struct stat st;
  stat("file", &st);
  int size = st.st_size;

  strcpy(t.payload, "file");
  t.len = size;
  
  printf("[send] I'll send file name and dimension\n");
  
  int ret = send_message(&t);
  if (ret < 0) {
    perror("HAp ca nu merge");
  }
  printf("pacpacp");
  if (recv_message(&t) < 0){
        perror("Receive error ...");
        return -1;
      }
      else {
        printf("[send] AM PRIMIT ACK PETNRU NUMELE MESAJULUI\n");
      } 

  while ((check = read(fd, p.payload, sizeof(p.payload)))) {
      if (check < 0)
          printf("Eroare la citire");
      //printf("%s", p.payload);
      p.type = 0;
      //p.payload contine mesajul
      t.len = check;

      //copiez in payload ul din msg tot pkt-ul    
      memcpy(t.payload, &p, MAX_LEN);

      //memset(p.payload, 0, sizeof(p.payload));


      printf("[send] Sending message...\n");
      send_message(&t);

        // Check response:
      if (recv_message(&t) < 0){
        perror("Receive error ...");
        return -1;
      }
      else {
        printf("[send] AM PRIMIT ACK");
      }   
  
  }

    close(fd);

  return 0;
}
