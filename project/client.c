#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char * argv[]) {
  if (argc != 3) {
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
  struct sockaddr_in serveraddr;
  memset( & serveraddr, 0, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET; // Use IPv4

  //if the second argument is “localhost” and use the IP “127.0.0.1”—otherwise, pass in the second argument verbatim to inet_addr. 
  if (strcmp(argv[1], "localhost") == 0) {
    inet_aton("127.0.0.1", & serveraddr.sin_addr);
  } else {
    inet_aton(argv[1], & serveraddr.sin_addr);
  }

  // set sending port
  int SEND_PORT = atoi(argv[2]);
  serveraddr.sin_port = htons(SEND_PORT);

  int BUF_SIZE = 1024;
  char server_buf[BUF_SIZE];
  char client_buf[BUF_SIZE];

  // infinite loop
  while (1) {
    /* 3. Listen for response from server (non-blocking) */
    socklen_t serversize = sizeof(serveraddr);
    int bytes_recvd = recvfrom(sockfd, server_buf, BUF_SIZE, 0,
      (struct sockaddr * ) & serveraddr, & serversize);
    if (bytes_recvd < 0) {
      if (errno != EWOULDBLOCK) {
        return errno;
      }
    } else if (bytes_recvd > 0) {
      write(STDOUT_FILENO, server_buf, bytes_recvd); // Output to stdout
    }

    /* 4. Send data to server (non-blocking) */
    ssize_t bytes_read = read(STDIN_FILENO, client_buf, BUF_SIZE);
    if (bytes_read > 0) {
      int did_send = sendto(sockfd, client_buf, bytes_read, 0,
        (struct sockaddr * ) & serveraddr, sizeof(serveraddr));
      if (did_send < 0) {
        return errno;
      }
    } else if (bytes_read < 0 && errno != EWOULDBLOCK) {
      return errno;
    }
  }

  /* 6. You're done! Terminate the connection */     
  close(sockfd);
  return 0;
}