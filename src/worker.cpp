#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
using namespace std;
#define numeric "0123456789"
#define small "abcdefghijklmnopqrstuvwxyz"
#define big "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
char* dest_ip;
int dest_port;
string salt = "";
string searchspace = "" ;
int spacelen;

string encode (bool found, string cracked){
    string result;
    if (found){
        result = '1';
    }
    else{
        result = '0';
    }
    result = result + ',' + cracked+";";
    return result;
}

void decode(string received, string& type, string& target, int& pwd_length, string& start, string& end){
    int i = 0;
    int j = 0;
    for (;;j++){
        if (received[j] == ','){
            break;
        }
    }
    type = received.substr(i,j-i);
    j++;
    i = j;
    for (;;j++){
        if (received[j] == ','){
            break;
        }
    }
    target = received.substr(i,j-i);
    j++;
    pwd_length = received[j]-'0';
    j+=2;
    start = received.substr(j,2*pwd_length);
    j = j+2*pwd_length+1;
    end = received.substr(j,2*pwd_length);
    salt = target.substr(0,2);
    return;
}

void next (int* curr, int pwd_length){
    for (int i = pwd_length-1; i >= 0; i--){
        if (curr[i]==spacelen-1){
            curr[i] = 0;
        }
        else{
            curr[i] = curr[i] + 1;
            break;
        }
    }
}

bool match(int* curr, string target, int pwd_length){
    string currstr = "";
    for (int i=0; i<pwd_length;i++){
        currstr = currstr + searchspace[curr[i]];
    }
    char *temp1 = new char[currstr.size()];
    char temp2[2];
    temp2[0] = salt[0];
    temp2[1] = salt[1];
    for (int i=0;i<currstr.size();i++){
        temp1[i] = currstr[i];
    }
    //char* temp3 = new char[target.size()];
    /*for (int i=0;i<target.size();i++){
        temp3[i] = target[i];
    }*/
    string hash = crypt(temp1,temp2);
    if (hash == target) {
        return true;
    }
    else {
        return false;
    }
}

void setspace(string type){
    searchspace="";
    if (type[0]=='1'){
        searchspace += numeric;
    }
    if (type[1]=='1'){
        searchspace += small;
    }
    if (type[2]=='1'){
        searchspace += big;
    }
    spacelen = searchspace.size();
}

bool equality (int* curr, int* final, int pwd_length){
    for (int i=0;i<pwd_length;i++){
        if (curr[i]!=final[i]){
            return false;
        }
    }
    return true;
}
void print (int* curr, int pwd_length){
    for (int i=0;i<pwd_length;i++){
        cout<<curr[i];
    }
    cout<<endl;
    return;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char* argv[])
{
    dest_ip = argv[1];
    dest_port = stoi(argv[2]);
    int sockfd;
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
    cout<<"Connected to Server";
    string greeting = ",w";
    strcpy(buffer, greeting.c_str());
    // greeting the server to let it know that the client is a worker
    send(sockfd, buffer, sizeof(buffer), 0);
    string type,target,start,end;
    int pwd_length;
    while(true)
    {
        bzero(buffer,256);
        recv(sockfd, buffer, sizeof(buffer), 0);
        cout<<"Received from Server : "<<endl;
        printf("%s\n", buffer);
        string received;
        int k = 0;
        while(k<256 && buffer[k]!=';')
        {
            received = received + buffer[k];
            k++;
        }
        //extract type, target, pwd_length, start, end from received message
        decode(received,type,target,pwd_length,start,end);
        
        setspace(type);
        
        bool found = false;
        string result;
        string cracked = "";
        int* curr = new int[pwd_length];
        int* final = new int[pwd_length];
        
        //convert start and end to curr and final encoding in decimals
        for(int i=0;i<pwd_length;i++){
            curr[i] = start[2*i+1] - '0';
            curr[i] = curr[i] + 10*(start[2*i]-'0');
            final[i] = end[2*i+1] - '0';
            final[i] = final[i] + 10*(end[2*i]-'0');
        }
        
        //ready to start iterating over all combinations
        cout<<searchspace<<endl;
        for (;!equality(curr,final,pwd_length);){
            if (match(curr,target,pwd_length)){
                found = true;
                break;
            }
            else{
                next(curr,pwd_length);
            }
        }
        if (match(curr,target,pwd_length)){
            found = true;
        }
        if (found){
            for (int i=0;i<pwd_length;i++){
                cracked = cracked+searchspace[curr[i]];
            }
        }
        string result_final = encode(found, cracked);
        // result_final contains the results. If no password was found, result_final = "0,"
        // else result_final = "1,password"
        cout<<"Final Result : "<<result_final<<endl;
        strcpy(buffer, result_final.c_str());
        printf("Sending to Server : %s\n", buffer);
        send(sockfd, buffer, sizeof(buffer), 0);
    }
    close(sockfd);
    return 0;
}