/*
 * server.c
 * CPE-555 Real-Time and Embedded Systems
 * Instructor: Richard Prego
 * Stevens Institute of Technology
 * Spring 2017
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define PORT_NUM "5550"
int sum = 0;
int run = 1;

void sigint_handler()
{
    /* set the 'run' flag to 0 so that the loop in main() will be terminated,
     * and the socket can be closed */
    run = 0;
}

int main()
{
    struct addrinfo hints;
    struct addrinfo *servinfo;
    int sockfd;
    int connected_sockfd;
    struct addrinfo clientinfo;
    socklen_t clientinfo_size;
    char send_string[140];
    char rcv_string[140];
    int bytes;
    int flags;

    /* catch SIGINT (signal caused by Control-C
     * we do this so that we can close the socket before the program is terminated
     */
    signal(SIGINT, sigint_handler);

    /* clear hints structure */
    memset(&hints, 0, sizeof(hints));

    /* don't care IPv4 or IPv6
     * TCP stream socket
     * fill in local IP address automatically
     */
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* getaddrinfo populates servinfo structure with local address info */
    if (getaddrinfo(NULL, PORT_NUM, &hints, &servinfo) != 0)
    {
        printf("Error in getaddrinfo\n");
        return 1;
    }

    /* get a file descriptor for the new socket */
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd == -1)
    {
        printf("Error creating socket\n");
        return 1;
    }

    /* bind the socket to the port we're interested in */
    if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        printf("Error binding to port\n");
        close(sockfd);
        return 1;
    }

    /* listen for connections on the socket */
    if (listen(sockfd, 10) == -1)
    {
        printf("Error in listen()\n");
        close(sockfd);
        return 1;
    }
    printf("Server is waiting for connections on port %s...\n", PORT_NUM);
    
    /* at this point we have received a connection
     * so, we accept it
     * also set the socket to be non-blocking so that we can terminate the
     * program with control-C
     */
    clientinfo_size = sizeof(clientinfo);
    connected_sockfd = accept(sockfd, (struct sockaddr*)&clientinfo, &clientinfo_size);
    flags = fcntl(connected_sockfd, F_GETFL, 0);
    fcntl(connected_sockfd, F_SETFL, flags|O_NONBLOCK);
    if (connected_sockfd == -1)
    {
        printf("Error accepting connection\n");
        return 1;
    }
    printf("Received connection from client\n");

    /* receive a message from the socket */
    while (run == 1)
    {
        bytes = recv(connected_sockfd, rcv_string, 140, 0);
        if (bytes > 0)
        {
            rcv_string[bytes] = '\0';
            printf("Received: %s", rcv_string);

        }
        sum=sum+atoi(rcv_string);
        printf("now the sum= %d",sum);
        //send message to client the current sum
        send_string=itoa(sum);
        bytes=send(connected_sockfd,send_string,10.0);
        if(bytes==-1)
        {printf("error in sending message\n");
        printf("send sum to client\n");
    }

    /* close the sockets */
    close(connected_sockfd);
    close(sockfd);

    return 0;
}
