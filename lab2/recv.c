#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001


int main(int argc,char** argv){
  msg r;
  pkt p;
  int check = 0, fd, file_size;
  init(HOST, PORT);


  if (recv_message(&r)<0){
    perror("Receive message");
    return -1;
  }

  //memcpy(p.type, r.payload, sizeof(int));
  //printf("%d %s", r.len, r.payload);
  //type - 0 pentru mesaj, 1 pentru ack
  printf("[recv] NUMELE FISIERULUI ESTE <%s>, dimeniunea <%d>, sending ACK...\n", r.payload, r.len);
  file_size = r.len;

  fd = open("giugiuc", O_CREAT | O_WRONLY | O_APPEND, S_IWUSR | S_IRUSR | S_IWGRP | S_IROTH | S_IWOTH);
  if (fd < 0)
    printf("Eroare la crearea fisierului");
 

  //trimit ACK
  p.type = 1;
  strcpy(p.payload, "");
  r.len = 0;
  memcpy(r.payload, &p, MAX_LEN);
  send_message(&r);
  
  while (check < file_size){

    if (recv_message(&r)<0){
      perror("Receive message");
      return -1;
    }

    memcpy(&p, r.payload, MAX_LEN);
    
    printf("[recv] Got msg with payload: <%s>, sending ACK...\n", p.payload);
    write(fd, p.payload, r.len);
    // Send ACK:

    p.type = 1;
    strcpy(p.payload, "");
    r.len = 0;
    memcpy(r.payload, &p, MAX_LEN);
    send_message(&r);
    check += r.len;

  }
   close (fd);

  /*
  sprintf(r.payload,"%s", "ACK");
  r.len = strlen(r.payload) + 1;
  send_message(&r);
  printf("[recv] ACK sent\n");
  */

  return 0;
}