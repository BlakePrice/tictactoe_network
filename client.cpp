#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <iomanip>

using namespace std;

#define BUF_SIZE 100

int main(int argc, char *argv[])
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, s;
  string message = ""; //might need to be a char*
  size_t len;
  ssize_t nread;
  char response[BUF_SIZE];

  if (argc != 3) 
  {
    fprintf(stderr, "Usage: %s host port...\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  //Obtain address matching host and port
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
  hints.ai_flags = 0;
  hints.ai_protocol = 0;          /* Any protocol */

  s = getaddrinfo(argv[1], argv[2], &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  // getaddrinfo() returns a list of address structures.
  // Try each address until we successfully connect(2).
  // If socket(2) (or connect(2)) fails, we (close the socket
  // and) try the next address.

  for (rp = result; rp != NULL; rp = rp->ai_next) 
  {
    sfd = socket(rp->ai_family, rp->ai_socktype,
        rp->ai_protocol);
    if (sfd == -1)
      continue;

    if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
      break; //Success

    close(sfd);
  }

  if (rp == NULL) //No address succeeded
  {  
    fprintf(stderr, "Could not connect\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(result); //No longer needed

  //Send remaining command-line arguments as separate
  //datagrams, and read responses from server
  
  while(true) 
  {
    //aquire message from user
    cout<<"Enter coordinate here: ";
    cin>>message; //setw() limits size of user input
    len = message.length() + 1; //+1 for terminating null byte, this is used only if message is a string
    
    
    //create message limitations
    //if(stoi(message) < 0 && stoi(message) > 9)
    if(len > 2) //2 refers to message size + 1 for null byte
    {
      fprintf(stderr, "Exceeding message limitations\n");
      continue;
    }
    //just in case message is larger than BUF_SIZE (useful for messages larger than intended for tictactoe)
    if (len + 1 > BUF_SIZE) 
    {
      fprintf(stderr, "Ignoring long message\n");
      continue;
    }


    //begin writing message
    if (write(sfd, message.c_str(), len) != len) //write message, if message is to be a string then convert to c string using c_str()
    {
      fprintf(stderr, "partial/failed write\n");
      exit(EXIT_FAILURE);
    }

    nread = read(sfd, response, BUF_SIZE); //read in response
    if (nread == -1) 
    {
      perror("read");
      exit(EXIT_FAILURE);
    }

    printf("Received %ld bytes: %s\n", (long) nread, response);
  }

  exit(EXIT_SUCCESS);
}
