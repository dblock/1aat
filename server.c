#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>

void error(char * msg) {
  printf("ERROR: %s\n", msg);
  exit(-1);
}

void do_not_linger(int s) {
  struct linger so_linger;
  so_linger.l_onoff = 0;
  so_linger.l_linger = 0;

  if (setsockopt(s, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger)) {
    error("SO_LINGER");
  }
}

void reuse(int s) {
  int optval = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)) {
    error("SO_REUSEADDR");
  }
}

void write_to(int s, char * msg) {
  write(s, msg, strlen(msg) + 1);
}

int main(int argc, char *argv[]) {

  int listen_socket, conn;
  struct sockaddr_in servaddr;
  pid_t  pid;

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(8080);

  while(1) {

    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
      error("socket");

    do_not_linger(listen_socket);
    reuse(listen_socket);

    printf("\n\nListening ...\n");

    while(1) {
      printf("Binding ...\n");
      if (bind(listen_socket, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
        printf("WARNING: Couldn't bind listening socket.\n");
        sleep(1);
      } else {
        break;
      }
    }

    if (listen(listen_socket, 1) < 0)
      error("listen");

  	if ((conn = accept(listen_socket, NULL, NULL)) < 0)
      error("conn");

  	if ((pid = fork()) == 0) {
      printf("request processing.\n");

	    if (close(listen_socket) < 0)
        error("close listening socket in child process");

      sleep(3); // request taking 3 seconds

      write_to(conn, "HTTP 200 OK\r\n");
      write_to(conn, "\r\n");
      write_to(conn, "Hello World\r\n");

	    if (close(conn) < 0)
        error("close connection in child process");

      printf("Exiting child\n");
      exit(0);
  	}

  	if (close(conn) < 0)
      error("closing connection in parent");

    if (close(listen_socket) < 0)
      error("close listen socket in parent");

  	waitpid(-1, NULL, WNOHANG);
  }

  return -1;
}
