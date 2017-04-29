/*
 * client.c
 * CPE-555 Real-Time and Embedded Systems
 * Instructor: Richard Prego
 * Stevens Institute of Technology
 * Spring 2017
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define PORT_NUM "5550"

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
    char send_string[140];
    int bytes;

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

    /* getaddrinfo populates servinfo structure with server's address info */
    if (getaddrinfo("localhost", PORT_NUM, &hints, &servinfo) != 0)
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

    /* connect the socket */
    if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        printf("Error connecting the socket\n");
        return 1;
    }

    while (run == 1)
    {
        /* prompt user for some data to send */
        printf("Enter string to send: ");
        fgets(send_string, 140, stdin);

        /* send the message on the socket */
        bytes = send(sockfd, send_string, strlen(send_string), 0);
        if (bytes == -1)
        {
            printf("Error sending message\n");
        }
        printf("Sent %d bytes to localhost port %s\n", bytes, PORT_NUM);
    }

    /* close the socket */
    close(sockfd);

    return 0;
}
