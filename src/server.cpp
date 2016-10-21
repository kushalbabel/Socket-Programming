#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <set>
#include <map>
#include <queue>
#define backlog 10
#define numeric "0123456789"
#define small "abcdefghijklmnopqrstuvwxyz"
#define big "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
using namespace std;

int server_port;
int worker_port;
string crackedPassword;
string searchspace = "" ;
int spacelen;
int pwd_length_int;
bool is_client_cracked[3];
struct query{
    int sockid;
    string message;
    query(int id, string msg){
        sockid = id;
        message = msg;
    }
};
struct work{
    string start;
    string end;
    work(){
        start = "";
        end = "";
    }
};

struct change
{
    int type; //0 for client and 1 for worker
    int id;
    string message;
    change(){
        type = 0;
        id = 0;
        message = "";
    }
};

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

string encodew(string type, string target, string pwd_length, string start, string end){
    string result = type+","+target+","+pwd_length+","+start+","+end+";";
    return result;
}

void decodeu(string received, string& target, string& pwd_length, string& type){
    int i = 0;
    int j = 0;
    for (;;j++){
        if (received[j] == ','){
            break;
        }
    }
    target = received.substr(i,j-i);
    j++;
    pwd_length = received[j];
    pwd_length_int = pwd_length[0] - '0';
    j+=2;
    type = received.substr(j,3);
    return;
}

string cracked(string message){
    string result;
    if (message[0]=='0'){
        result = "0";
    }
    else{
        result = message.substr(2,pwd_length_int);
    }
    return result;
}


work assign (int& initial, int& final){
    cout<<"spacelen : "<<spacelen<<endl;
    work *assigned = new work();
    assigned->start = to_string(initial/10);
    assigned->start = assigned->start + to_string(initial%10);
    assigned->end = to_string(initial/10);
    assigned->end = assigned->end + to_string(initial%10);
    cout<<"Assign Start : "<<assigned->start<<" Assign End : "<<assigned->end<<endl;
    for (int i=1;i<pwd_length_int;i++){
        assigned->start = assigned->start + "00";
        if(spacelen-1<10)
        {
            assigned->end = assigned->end + to_string(0) + to_string(spacelen-1);
        }
        else
            assigned->end = assigned->end + to_string(spacelen-1);
    }
    initial+=1;
    return *assigned;
}

int main(int argc, char* argv[]){
    server_port = stoi(argv[1]);
    worker_port = stoi(argv[1]);
    queue<query> queries;
    string target;
    string pwd_length;
    string type;
    map<int, bool> isFree;
    int initial = 0;
    int final = 0;
    int listenfd,connectfd;
    struct sockaddr_in server_addr, user_addr;
    set<int> users, workers;
    set<int> :: iterator worker_it;
    int nbytes;
    char buffer[256];
    socklen_t sin_size;
    cout<<"Listening"<<endl;
    sin_size = sizeof(struct sockaddr_in);
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int yes = 1;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    if((listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        cout<<"ERROR opening socket"<<endl;
        return -1;
    }
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        cout<<"Server-setsockopt() error"<<endl;
        return -1;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero),'\0',8);
    bind(listenfd,(struct sockaddr*)&server_addr,sizeof(struct sockaddr_in));
    if(listen(listenfd, backlog) == -1)
    {
        cout<<"Server-listen() error"<<endl;
        return -1;
    }
    FD_SET(listenfd, &master);
    fdmax = listenfd; //keeping track of largest file descriptor
    while(true)
    {
        read_fds = master;
        select(fdmax+1, &read_fds, NULL, NULL, NULL);
        for(int i = 0; i <= fdmax; i++)
        {
            if(FD_ISSET(i, &read_fds))
            {   //there has been some activity
                if(i == listenfd)
                {   //new connection
                    connectfd = accept(listenfd,(struct sockaddr*)&user_addr,&sin_size);
                    FD_SET(connectfd, &master); // add to master set
                    if(connectfd > fdmax)
                    {
                        //keeping track of largest file descriptor
                        fdmax = connectfd;
                    }
                    printf("New connection from %s on socket %d\n", inet_ntoa(user_addr.sin_addr), connectfd);
                }
                else
                {
                    // handle data from a client, i.e either user or worker
                    if((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0)
                    {
                        // Error or connection closed by client
                        if(nbytes == 0)
                        // connection closed
                            printf("socket %d hung up\n", i);
                        
                        else
                            printf("recv() error on socket : %d\n", i);
                        
                        //close it.
                        close(i);
                        // remove from master set
                        FD_CLR(i, &master);
                    }
                    else
                    {   //received some data successfully, handle it
                        if (buffer[0] == ',')
                        {   //greeting message
                            if (buffer[1] == 'w')
                            {   //it's a worker hence add it to worker set
                                workers.insert(i);
                                cout<<"Added "<<i<<" into worker set"<<endl;
                                isFree[i] = true;
                                if (initial!=final)
                                        {
                                            for(worker_it=workers.begin();worker_it!=workers.end();worker_it++)
                                            {
                                                if(isFree[*worker_it])
                                                {
                                                    work currassign;
                                                    currassign = assign(initial,final);
                                                    string temp1 = type+','+target+','+pwd_length+','+currassign.start+","+currassign.end;
                                                    bzero(buffer,256);
                                                    strcpy(buffer, temp1.c_str());
                                                    cout<<"Sending to worker : ";
                                                    printf("%s\n", buffer);
                                                    send(*(worker_it), buffer, sizeof(buffer), 0);
                                                    isFree[*worker_it] = false;
                                                }
                                            }
                                        }
                            }
                            else if (buffer[1] == 'u')
                            {   //it's a user, hence add it to user set
                                users.insert(i);
                                cout<<"Added "<<i<<" into user set"<<endl;
                            }
                        }
                        else
                        {   //query from user or response from worker
                            int k=0;
                            string message="",decodedMessage="";
                            cout<<"Received buffer : ";
                            printf("%s\n", buffer);
                            while(k<256 && buffer[k]!=';')
                            {
                                message = message + buffer[k];
                                k++;
                            }
                            if (workers.find(i) != workers.end())
                            {   // the client is a worker
                                decodedMessage = cracked(message);
                                if(message[0] == '0')
                                {   // if message starts with zero -> couldn't find password, hence assign new work
                                    isFree[i] = true;
                                    if(!queries.empty())
                                    {
                                        //assign it more work
                                        if (initial!=final)
                                        {
                                            cout<<"Worker couldn't find password, reassigning work"<<endl;
                                            work currassign;
                                            currassign = assign(initial,final);
                                            string temp1 = type+','+target+','+pwd_length+','+currassign.start+","+currassign.end+';';
                                            bzero(buffer,256);
                                            strcpy(buffer, temp1.c_str());
                                            cout<<"Sending to worker : ";
                                            printf("%s\n", buffer);
                                            send(i, buffer, sizeof(buffer), 0);
                                            isFree[i] = false;
                                        }
                                    }
                                }
                                else if(message[0] == '1')
                                {
                                    //success, return password to user and assign new work
                                    isFree[i] = true;
                                    crackedPassword = decodedMessage;
                                    strcpy(buffer, crackedPassword.c_str());
                                    send((queries.front()).sockid, buffer, sizeof(buffer), 0);
                                    queries.pop(); //clearing the query, since it's password has been cracked
                                    if(!queries.empty())
                                    {   //pending queries, allot work to workers
                                        decodeu(queries.front().message, target, pwd_length, type);
                                        setspace(type);
                                        initial = 0;
                                        final = spacelen;
                                        cout<<"Target : "<<target<<" pwd_length : "<<pwd_length<<" type : "<<type<<endl;
                                        if (initial!=final)
                                        {
                                            for(worker_it=workers.begin();worker_it!=workers.end();worker_it++)
                                            {
                                                if(isFree[*worker_it])
                                                {
                                                    work currassign;
                                                    currassign = assign(initial,final);
                                                    string temp1 = type+','+target+','+pwd_length+','+currassign.start+","+currassign.end;
                                                    bzero(buffer,256);
                                                    strcpy(buffer, temp1.c_str());
                                                    cout<<"Sending to worker : ";
                                                    printf("%s\n", buffer);
                                                    send(*(worker_it), buffer, sizeof(buffer), 0);
                                                    isFree[*worker_it] = false;
                                                }
                                            }
                                        }
                                    }
                                    
                                }
                            }
                            else if (users.find(i) != users.end())
                            {   //client is a user, i.e a new query
                                query *temp = new query(i,message);
                                //push the query into queue
                                queries.push(*temp);
                                if(queries.front().sockid == i)
                                {   // the new query is the only query, hence process it
                                    // extract the target, pwd_length, type from message
                                    decodeu(message, target, pwd_length, type);
                                    // set searchspace according to type
                                    setspace(type);
                                    initial = 0;
                                    final = spacelen;
                                    cout<<"Target : "<<target<<" pwd_length : "<<pwd_length<<" type : "<<type<<endl;
                                    if (initial!=final)
                                    {   //Work is not complete
                                        for(worker_it=workers.begin();worker_it!=workers.end();worker_it++)
                                        {   //check if worker is free
                                            if(isFree[*worker_it])
                                            {   //worker is free, assign work
                                                work currassign;
                                                //assign a part of pending work
                                                currassign = assign(initial,final);
                                                string temp1 = type+','+target+','+pwd_length+','+currassign.start+","+currassign.end;
                                                bzero(buffer,256);
                                                strcpy(buffer, temp1.c_str());
                                                cout<<"Sending to worker : ";
                                                printf("%s\n", buffer);
                                                //sending work to worker
                                                send(*(worker_it), buffer, sizeof(buffer), 0);
                                                isFree[*worker_it] = false;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    close(connectfd);
    close(listenfd);
    return 0;
}