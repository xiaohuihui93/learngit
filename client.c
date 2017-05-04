/* client.c
 * cpe555 Real-Time and Embedded Systems
 * Jiahui Chen
 * 10422306
 * reference : client.c on Canvas provided by professor
 *
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
    //since we use the eg:TCP so do not have to bind and listen
    //then we connect
     //bind(sockfd,struc sockaddr *my_addr,int addrlen);
    if(connect(sockfd,servinfo->ai_addr,servinfo->ai_addrlen)==-1){
        printf("error in connecting");
        close(sockfd);
        return 1;
    }
    //else we building connection with server
    while (1) {

        //prompt user for enter a number
        printf("Enter a number to send:");
        //create a char [] to store what we want to send
        char send_string[140];
        //create a char[] to store the result of sum from server
        char rec_string[140];
        fgets(send_string,140,stdin);
        int bytes=send(sockfd,send_string,140,0);
        if(bytes==-1){
            printf("error in sending number\n");
        }
        printf("send a number %s to server\n",send_string);
        //receive sum from server
        bytes=recv(sockfd,rec_string,140,0);
        if(bytes==-1){
            printf("error in receiving sum from server\n");
        }
        rec_string[bytes]='\0';//since there need a \0 at the end
        printf("sum is : %s \n",rec_string);
    }
    close(sockfd);
    return 0;
}
