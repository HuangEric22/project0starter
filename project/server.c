#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include  <stdio.h>
#include <errno.h>

int main(int argc, char * argv[]) {
  if (argc != 2) {
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

  /* 2. Construct our address */
  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET; // use IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY; // accept all connections
  // same as inet_addr("0.0.0.0") 
  // "Address string to network bytes"
  // Set receiving port
  int PORT = atoi(argv[1]);
  servaddr.sin_port = htons(PORT); // Big endian

  /* 3. Let operating system know about our config */
  int did_bind = bind(sockfd, (struct sockaddr * ) & servaddr,
    sizeof(servaddr));
  // Error if did_bind < 0 :(
  if (did_bind < 0) return errno;

  /* 4. Create buffer to store incoming data */
  int BUF_SIZE = 1024;
  char client_buf[BUF_SIZE];
  struct sockaddr_in clientaddr; // Same information, but about client
  socklen_t clientsize = sizeof(clientaddr);

  int connected = 0;
  //infinite loop
  while (1) {
    /* 5. Listen for data from clients */
    int bytes_recvd = recvfrom(sockfd, client_buf, BUF_SIZE,
      // socket  store data  how much
      0, (struct sockaddr * ) & clientaddr, &
      clientsize);
    // Execution will stop here until `BUF_SIZE` is read or termination/error
    // Error if bytes_recvd < 0 :(
    if (bytes_recvd < 0) return errno;

    else if (bytes_recvd == 0 && !connected) continue;

    //as soon as the client has sent data over, we can assume connected is established
    connected = 1;

    /* 6. Inspect data from client */
    char * client_ip = inet_ntoa(clientaddr.sin_addr);
    // "Network bytes to address string"
    int client_port = ntohs(clientaddr.sin_port); // Little endian
    // Print out data
    if (bytes_recvd > 0)
      write(STDOUT_FILENO, client_buf, bytes_recvd);

    /* 7. Send data back to client */
    char server_buf[BUF_SIZE];
    if (fgets(server_buf, sizeof(server_buf), STDIN_FILENO) != 0) {
      int did_send = sendto(sockfd, server_buf, strlen(server_buf),
        // socket  send data   how much to send
        0, (struct sockaddr * ) & clientaddr,
        // flags   where to send
        sizeof(clientaddr));
      if (did_send < 0) return errno;
    }
  }

  /* 8. You're done! Terminate the connection */
  close(sockfd);
  return 0;
}