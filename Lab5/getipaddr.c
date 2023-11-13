/*
    CSCI484 Computer Networks
    Basic Socket Programming C
    Reference: https://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch
    Modifed by: Mia Weber
    11/09/23
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h> // inet_ntop
#include <netinet/in.h> // sin_addr (network byte order)

//int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);

int main(int argc, char *argv[]) {
    int status;
    struct addrinfo hints, *res, *p;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) { // Make sure correct number of command line arguments are given
        fprintf(stderr,"usage: showip hostname\n");
        return 1;
    }

    //struct addrinfo *servinfo; // Will point to the results

    memset(&hints, 0, sizeof(hints)); // Make sure struct is empty - initializes with zeros
    hints.ai_family = AF_UNSPEC; // Work with IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM; // Specifies socket type for TCP sockets

    // Get address info for the host
    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s/n", gai_strerror(status));
        return 2;
    }

    printf("IP addresses for %s:\n\n", argv[1]); // Display host IP addresses

    for (p=res; p!=NULL; p = p->ai_next) { // Iterate over each address result
        void *addr;
        char *ipver;

        if (p->ai_family == AF_INET) { // Checks if the address is IPv4 or IPv6 and gets the correct address
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "Ipv4";
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockeraddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // Convert the IP to a string and print it
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        printf(" %s: %s\n", ipver, ipstr);
    }

    // Get ready to connect
    //status = getaddrinfo("www.example.net", "3490", &hints, &servinfo);

    freeaddrinfo(res); // Free the linked list

    return 0;
}