#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  int sock, yes = 1;
  struct addrinfo hints, *serverinfo, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int rv = 0;
  if ((rv = getaddrinfo(NULL, "5000", &hints, &serverinfo) != 0)) {
    perror("getaddrinfo");
  }

  for (p = serverinfo; p!= NULL; p=p->ai_next) {
    if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      return -1;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      return -1;
    }

    if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
      close(sock);
      perror("bind");
      continue;
    }
    break;
  }

  freeaddrinfo(serverinfo);
  if (p == NULL) {
    fprintf(stderr, "failed to bind");
    return -1;
  }
  if (listen(sock, 10) == -1) {
    perror("listen");
    return -1;
  }
  
    printf("server: waiting for connections...\n");
  socklen_t sin_size;
  struct sockaddr_storage their_addr;
  int new_fd;

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sock, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        printf("Got client\n");
        if (send(new_fd, "SYN-ACK", 13, 0) == -1)
            perror("send");
        char buf[50];
        if (recv(new_fd, buf, 49, 0) == -1) perror("recv");
        printf("%s\n", buf);
        if (recv(new_fd, buf, 49, 0) == -1) perror("recv");
        printf("%s\n", buf);
        close(new_fd);  // parent doesn't need this
    }

  return 0;
}
