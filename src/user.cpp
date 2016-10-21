#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
using namespace std;
char* dest_ip;
int dest_port;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char* argv[])
{
    dest_ip = argv[1];
    dest_port = stoi(argv[2]);
    string target, pwd_length, type;
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    char buffer[256];
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cout<<"ERROR opening socket"<<endl;
        return -1;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(dest_ip);
    serv_addr.sin_port = htons(dest_port);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        cout<<"ERROR connecting"<<endl;
        return -1;
    }
    string greeting = ",u";
    strcpy(buffer, greeting.c_str());
    send(sockfd, buffer, sizeof(buffer), 0);
    bzero(buffer,256);
    target = argv[3];
    pwd_length = argv[4];
    type = argv[5];
    string message = target + "," + pwd_length + "," + type + ";";
    cout<<"Message : "<<message<<endl;
    strcpy(buffer, message.c_str());
    n = send(sockfd, buffer, sizeof(buffer), 0);
    if (n < 0)
    {
        cout<<"ERROR sending to socket"<<endl;
        return -1;
    }
    bzero(buffer,256);
    recv(sockfd, buffer, 255, 0);
    if (n < 0)
    {
        cout<<"ERROR reading to socket"<<endl;
        return -1;
    }
    printf("Received from Server : %s\n", buffer);
    close(sockfd);
    return 0;
}