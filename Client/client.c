// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <readline/readline.h>  //for {readline}
#include <readline/history.h>  //for {readline}
#define PORT 8000
#define SIZE 1024

char ** tokenizer(char *input_line, char *delimiter);
void write_file(int sockfd, char *filename, int numlines);
int receive_int(int *num, int fd);
void printProgress(float percentage);

int main(int argc, char const *argv[])
{   

    char *input = readline("Client > ");
    
    if(input[0]=='g' && input[1] == 'e' && input[2] == 't' && input[3] == ' ' && strlen(input)>4)
    {
        struct sockaddr_in address;
        int sock = 0, valread;
        struct sockaddr_in serv_addr;
        char buffer[1024] = {0};
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return -1;
        }

        memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                    // which is meant to be, and rest is defined below

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        // Converts an IP address in numbers-and-dots notation into either a 
        // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
        if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
        {
            printf("\nInvalid address/ Address not supported \n");
            return -1;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
        {
            printf("\nConnection Failed \n");
            return -1;
        }

    
        send(sock , input , strlen(input) , 0 );  // send the message.
        printf("Filenames sent\n");

        char **inputlist = tokenizer(input," ");
        int inputcount = 0;
        while(inputlist[inputcount] != NULL)inputcount++;

        for(int i = 1;i < inputcount;i++)
        {
            FILE *fp;
            printf("%s\n",inputlist[i]);
            char *filename = inputlist[i];
            int numl;
            receive_int(&numl,sock);
            printf("\n");

            write_file(sock,filename,numl);
            printf("\n[+]Data written in the file successfully.\n");
        }
    }
    else
    {
        printf("Wrong command Expected : get <filename1> <filename2>");
    }

    return 0;
}

void write_file(int sockfd,char *filename, int numlines){
  int n;
  FILE *fp;
  char buffer[SIZE];

  fp = fopen(filename, "w");
  int jiter = 0;
  while (jiter<numlines) {
    jiter++;
    float perc = (float)jiter/(float)numlines;
    n = recv(sockfd, buffer, SIZE, 0);
    if (n <= 0){
      printf("\nThere was an error in reading the file \n");
      printf("A Blank File will be written\n");
      break;
      return;
    }
    fprintf(fp, "%s", buffer);
    bzero(buffer, SIZE);
    printProgress(perc);
    sleep(1);
  }
  printf("\nWrote : %d times",jiter);
  return;
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

int receive_int(int *num, int fd)
{
    int32_t ret;
    char *data = (char*)&ret;
    int left = sizeof(ret);
    int rc;
    do {
        rc = read(fd, data, left);
        if (rc <= 0) { /* instead of ret */
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                printf("There is some problem in the socket");
                exit(1);
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
    *num = ntohl(ret);
    return 0;
}

void printProgress(float percentage) 
{
  char str[10];
  float val = (float) (percentage * 100);
  sprintf(str,"\r%.2f", val);
  write(1,str,strlen(str));
}