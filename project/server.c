#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char * argv[]) {
  if (argc != 2) {
    perror("Invalid usage");
    return errno;
  }

  /* 1. Create socket */
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // IPv4, UDP
  if (sockfd < 0) {
    return errno;
  }

  // make socket non-blocking
  int sock_flags = fcntl(sockfd, F_GETFL);
  if (sock_flags < 0) {
    return errno;
  }
  fcntl(sockfd, F_SETFL, sock_flags | O_NONBLOCK);

  // make stdin non-blocking
  int stdin_flags = fcntl(STDIN_FILENO, F_GETFL);
  if (stdin_flags < 0) {
    return errno;
  }
  fcntl(STDIN_FILENO, F_SETFL, stdin_flags | O_NONBLOCK);

  /* 2. Construct server address */
  struct sockaddr_in servaddr;
  memset( & servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY; // accept all connections
  int PORT = atoi(argv[1]); // same as inet_addr("0.0.0.0") 
  servaddr.sin_port = htons(PORT); // Convert port to big endian

  /* 3. Bind the socket to the address */
  int did_bind = bind(sockfd, (struct sockaddr * ) & servaddr, sizeof(servaddr));
  if (did_bind < 0) {
    return errno;
  }

  /* 4. Create buffer to store incoming data */
  int BUF_SIZE = 1024;
  char client_buf[BUF_SIZE];
  struct sockaddr_in clientaddr; // Information about the client
  socklen_t clientsize = sizeof(clientaddr);

  int connected = 0;

  // infinite loop
  while (1) {
    /* 5. Listen for data from clients */
    int bytes_recvd = recvfrom(sockfd, client_buf, BUF_SIZE, 0,
      (struct sockaddr * ) & clientaddr, & clientsize);
    if (bytes_recvd < 0) {
      if (errno == EWOULDBLOCK) {
        continue;
      } else {
        return errno;
      }
    } else if (bytes_recvd == 0 && !connected) {
      continue;
    }

    //the client has connected
    connected = 1;

    /* 6. Inspect and output data from client */
    if (bytes_recvd > 0) {
      write(STDOUT_FILENO, client_buf, bytes_recvd); // iutput to stdout
    }

    /* 7. Send data back to client */
    char server_buf[BUF_SIZE];
    ssize_t bytes_read = read(STDIN_FILENO, server_buf, BUF_SIZE);
    if (bytes_read < 0) {
      if (errno != EWOULDBLOCK) {
        return errno;
      }
    } else if (bytes_read > 0) {
      int did_send = sendto(sockfd, server_buf, bytes_read, 0,
        (struct sockaddr * ) & clientaddr, sizeof(clientaddr));
      if (did_send < 0) {
        return errno;
      }
    }
  }

  /* 8. You're done! Terminate the connection */     
  close(sockfd);
  return 0;
}