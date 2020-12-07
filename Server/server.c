#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <fcntl.h> 
#include <errno.h>

#define PORT 8000
#define SIZE 1024


char ** tokenizer(char *input_line, char *delimiter);
void send_file(FILE *fp, int sockfd);
int count_lines(FILE *fp);
int send_int(int num, int fd);


int main(int argc, char const *argv[])

{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;  
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                       (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    valread = read(new_socket , buffer, 1024);  // read infromation received into the buffer
    printf("%s\n",buffer);
    //send(new_socket , hello , strlen(hello) , 0 );  // use sendto() and recvfrom() for DGRAM
    //printf("Hello message sent\n");
    printf("Now we will tokenize this and send each file\n");

    char **bufferlist = tokenizer(buffer," ");
    int buffer_count = 0;
    while(bufferlist[buffer_count] != NULL)buffer_count++;
    for(int i = 1;i < buffer_count;i++)
    {
        FILE *fp;
        printf("%s\n",bufferlist[i]);
        char *filename = bufferlist[i];
        fp = fopen(filename, "r");
        if (fp == NULL) 
        {
        perror("[-]Error in reading file.");
        exit(1);
        }
        int lines = count_lines(fp);
        send_int(lines, new_socket);
        printf("Number of lines sent\n");
        fclose(fp);
        fp = fopen(filename, "r");
        if (fp == NULL) 
        {
        perror("[-]Error in reading file.");
        exit(1);
        }
        send_file(fp, new_socket);
        printf("[+]File data sent successfully.\n");
    }

    return 0;

}


char ** tokenizer(char *input_line, char *delimiter) 
{
    
    char **alltokens = malloc(256 * sizeof(char *));
    if (alltokens == NULL) {
        perror("malloc failed");
        exit(1);
    }  
    char *token;
    int position = 0;

    token = strtok(input_line, delimiter);
    while(token != NULL) {
        alltokens[position] = token;
        position++;

        token = strtok(NULL, delimiter);
    }
    alltokens[position] = NULL;
    return alltokens;
}


void send_file(FILE *fp, int sockfd)
{
  int n;
  int iter = 0;
  int count;
  char data[SIZE] = {0};
  while(fgets(data, SIZE, fp) != NULL) 
  {
    iter++;  
    if (send(sockfd, data, sizeof(data), 0) == -1) {
      perror("[-]Error in sending file.");
      exit(1);
    }
    bzero(data, SIZE);
  }
  printf("Iters taken = %d\n",iter);
  iter = 0;
}

int count_lines(FILE *fp)
{
    char c;
    int count = 0;
    for (c = getc(fp); c != EOF; c = getc(fp)) 
    if (c == '\n') // Increment count if this character is newline 
      count = count + 1; 
    printf("Number of lines in the file is : %d\n",count);
    return count;
}


int send_int(int num, int fd)
{
    int32_t conv = htonl(num);
    char *data = (char*)&conv;
    int left = sizeof(conv);
    int rc;
    do {
        rc = write(fd, data, left);
        if (rc < 0) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                // use select() or epoll() to wait for the socket to be writable again
            }
            else if (errno != EINTR) {
                return -1;
            }
        }
        else {
            data += rc;
            left -= rc;
        }
    }
    while (left > 0);
    return 0;
}