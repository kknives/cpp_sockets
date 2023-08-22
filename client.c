#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
  int sockfd, yes = 1, rv;
  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

if ((rv = getaddrinfo(argv[1], "5000", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

  if (send(sockfd, "SYN", 4, 0) == -1) {
    perror("send");
    return -1;
  }
  printf("SYN\n");
  char msgb[50];
  if (recv(sockfd, msgb, 49, 0) == -1) {
    perror("recv");
    return -1;
  }
  printf("%s\n", msgb);
  if (send(sockfd, "ACK", 4, 0) == -1) {
    perror("send");
    return -1;
  }
  return 0;
}
