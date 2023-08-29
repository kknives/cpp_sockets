#include <arpa/inet.h>
#include <chrono>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <poll.h>

int
main(int argc, char** argv)
{
  int sock, yes = 1;
  struct addrinfo hints, *serverinfo, *p;

  struct pollfd pfds[2];
  pfds[0].fd = 0;
  pfds[0].events = POLLIN;

  bool connect_mode = (argc == 3) and (strcmp("--connect", argv[2]) == 0);
  if (connect_mode) {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
  } else {
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
  }

  int rv = 0;
  printf("Trying %s\n", argv[1]);
  if ((rv = getaddrinfo("0.0.0.0", argv[1], &hints, &serverinfo) != 0)) {
    perror("getaddrinfo");
  }

  for (p = serverinfo; p != NULL; p = p->ai_next) {
    if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
    }

    if (connect_mode) {
      if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
        close(sock);
        perror("connect");
        continue;
      }
    } else {
      if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        return -1;
      }
      if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
        close(sock);
        perror("bind");
        continue;
      }
    }
    break;
  }

  freeaddrinfo(serverinfo);
  if (p == NULL) {
    fprintf(stderr, "failed to bind/connect");
    return -1;
  }
  if (not connect_mode) {
    if (listen(sock, 10) == -1) {
      perror("listen");
      return -1;
    }
    
    printf("peer: waiting for connections...\n");
  }

  socklen_t sin_size;
  struct sockaddr_storage their_addr;
  int new_fd;

  while (1) { // main accept() loop
    sin_size = sizeof their_addr;
    if (not connect_mode) {
      new_fd = accept(sock, (struct sockaddr*)&their_addr, &sin_size);
      if (new_fd == -1) {
        perror("accept");
        continue;
      }
      printf("Got peer\n");
    } else {
      new_fd = sock;
    }
    
    pfds[1].fd = new_fd;
    pfds[1].events = POLLIN;
    for (;;) {
      std::string msg;
      std::cout << ">>>" << '\n';
      int events = 0;
      while ((events = poll(pfds, 2, 2000)) == 0);
      if (not events) continue;
      if (pfds[0].revents & POLLIN) {
	      std::getline(std::cin, msg);

	      if (msg == "q") {
		close(new_fd);
		break;
	      }
	      if (send(new_fd, msg.c_str(), msg.size(), 0) == -1) {
		perror("send");
		break;
	      }
      }
      else if (pfds[1].revents & POLLIN) {
	      char msgb[50];
	      if (int got = recv(new_fd, msgb, 49, 0); got == -1) {
		perror("recv");
		break;
	      }
	      else
		msgb[got] = 0;
	      printf("Peer said: %s\n", msgb);
      }
    }
    if (connect_mode) break;
  }

  return 0;
}
