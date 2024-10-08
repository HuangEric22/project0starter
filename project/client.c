#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include  <stdio.h>
#include <errno.h>

int main(int argc, char * argv[]) {

  if (argc > 3) {
    return errno;
  }

  /* 1. Create socket */
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  // use IPv4  use UDP
  //make socket non-blocking

  int sock_flags = fcntl(sockfd, F_GETFL);
  //if there are errors getting socket return error
  if (sock_flags == -1) {
    return errno;
  }
  fcntl(sockfd, F_SETFL, sock_flags | O_NONBLOCK);

  //make server non-blocking
  int stdin_flags = fcntl(STDIN_FILENO, F_GETFL);
  if (stdin_flags == -1) {
    return errno;
  }
  fcntl(STDIN_FILENO, F_SETFL, stdin_flags | O_NONBLOCK);

  /* 2. Construct server address */
  struct sockaddr_in serveraddr;
  //if the second argument is “localhost” and use the IP “127.0.0.1”—otherwise, pass in the second argument verbatim to inet_addr. 

  if (argv[1] == "localhost") {
    inet_aton("127.0.0.1", & serveraddr.sin_addr);
  } else {
    inet_aton(argv[1], & serveraddr.sin_addr);
  }

  serveraddr.sin_family = AF_INET; // use IPv4
  serveraddr.sin_addr.s_addr = INADDR_ANY;
  // Set sending port
  int SEND_PORT = atoi(argv[2]);
  serveraddr.sin_port = htons(SEND_PORT); // Big endian

  int BUF_SIZE = 1024;

  while (1) {

    /* 4. Create buffer to store incoming data */
    char server_buf[BUF_SIZE];
    socklen_t serversize = sizeof(socklen_t); // Temp buffer for recvfrom API

    /* 5. Listen for response from server */
    int bytes_recvd = recvfrom(sockfd, server_buf, BUF_SIZE,
      // socket  store data  how much
      0, (struct sockaddr * ) & serveraddr, &
      serversize);
    // Execution will stop here until `BUF_SIZE` is read or termination/error
    // Error if bytes_recvd < 0 :(
    if (bytes_recvd < 0) return errno;
    // Print out data
    else if (bytes_recvd > 0)
      write(STDOUT_FILENO, server_buf, bytes_recvd);

    /* 3. Send data to server */
    char client_buf[BUF_SIZE];
    if (fgets(client_buf, sizeof(client_buf), STDIN_FILENO) != 0) {
      int did_send = sendto(sockfd, client_buf, strlen(client_buf),
        // socket  send data   how much to send
        0, (struct sockaddr * ) & serveraddr,
        // flags   where to send
        sizeof(serveraddr));
      if (did_send < 0) return errno;
    }

  }

  /* 6. You're done! Terminate the connection */
  close(sockfd);
  return 0;
}