/* client.c
 * cpe555 Real-Time and Embedded Systems
 * Jiahui Chen
 * 10422306
 * reference : server.c on Canvas provided by professor
 *the function atoi can work ,but itoa cannot ,find a new sprintf through the internet
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
//define client port which is same as server port
#define PORT_NUM "5550"
int main()
{
    struct addrinfo hints;//structure with information about the address
    struct addrinfo *servinfo;
    //clear hints
    memset(&hints,0,sizeof(hints));
    //fill in local IP address
    hints.ai_family=AF_INET;//for ipv4
    hints.ai_socktype=SOCK_STREAM;//for TCP
    hints.ai_flags=AI_PASSIVE;//setting options
    int sum=0;
    //use getaddrinfo return value to judge if we get the info
    if(getaddrinfo(NULL,PORT_NUM,&hints,&servinfo)!=0){
        printf("error in getaddrinfo\n");
        return 1;
    }
    //creating a socket
    int sockfd;//the file descriptor for socket
    sockfd=socket(servinfo->ai_family,servinfo->ai_socktype, servinfo->ai_protocol);
    //see if create socket succeed
    if (sockfd==-1)
    {printf("error in creating socket");
    return 1;}
    //as server need to bind to the port we set
    if(bind(sockfd,servinfo->ai_addr,servinfo->ai_addrlen)==-1){
        printf("error in bind\n");
        close(sockfd);
        return 1;
    }
    //then we listen
    if(listen(sockfd,10)==-1){
        printf("error in listening\n");
        close(sockfd);
        return 1;
    }
    //finally to accept connection
    struct addrinfo clientinfo;
    socklen_t clientinfo_size;
    clientinfo_size=sizeof(clientinfo);
    int connect_sockfd=accept(sockfd,(struct sockaddr*)&clientinfo,&clientinfo_size);
    if(connect_sockfd==-1){
        printf("error in accept connecting");
        close(sockfd);
        return 1;
    }
    //else we building connection with server
    while (1) {

        //create a char [] to store what we want to send
        char send_string[140];
        //create a char[] to store the result of sum from server
        char rec_string[140];
        //receive number from client
        int bytes=recv(connect_sockfd,rec_string,140,0);
        if(bytes==-1){
            printf("error in receiving number\n");
            close(connect_sockfd);
            close(sockfd);
            return 1;
        }
        rec_string[bytes]='\0';//since there need a \0 at the end
        printf("received : %s",rec_string);
        sum+=atoi(rec_string);//change string to int
        printf("now the sum is:%d \n",sum);
        sprintf(send_string,"%d",sum);//change int to string and I don not know why itoa() cannot run in linux
        bytes=send(connect_sockfd,send_string,10,0);

        if(bytes==-1){
            printf("error in sending sum to client\n");
        }
         printf("send the sum %d to client \n",sum);

    }
//close the sockets
close(connect_sockfd);
    close(sockfd);
    return 0;

}
