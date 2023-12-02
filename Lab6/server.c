/* 
Created by: Mia Weber
12-02-2023
Description: Server with two modes 1 for echo server and 2 for file transfer.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // POSIX API - read, writet, close
#include <sys/types.h>  // Destinations for network operations and socket types
#include <sys/socket.h> // socket(), bind(), listen(), etc.
#include <netinet/in.h> // Internet operations like struct sockaddr_in
#include <arpa/inet.h>  // Internet operations and internet addresses

#define MAX_BUFFER_SIZE 101 // 100 chars + null terminator 

// Error handling function
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Logic for echo mode (1). Echo server functionality
void echo(int socket) {
    char buffer[MAX_BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, MAX_BUFFER_SIZE); // Clear the buffer
        int n = read(socket, buffer, MAX_BUFFER_SIZE - 1); // Read data from socket until buffer is full
        if (n < 0) error("ERROR reading from socket");

        // If the client sends "close" echo "goodbye"
        if (strncmp(buffer, "close", 5) == 0) { 
            write(socket, "goodbye!", 7); // Send "goodbye" and exit
            break;
        }

        printf("Echoing: %s\n", buffer); // Print echoed message

        n = write(socket, buffer, strlen(buffer)); // Write buffer back to socket
        if (n < 0) error("ERROR writing to socket");
    }
}

// Logic for file transfer mode (2). Send specified file to client
void fileTransferMode(int socket, const char *filename) {
    FILE *file = fopen(filename, "rb"); // Open the file in binary mode to read
    if (file == NULL) {
        error("ERROR opening file");
        return;
    }

    char buffer[MAX_BUFFER_SIZE];
    int n;
    while ((n = fread(buffer, 1, MAX_BUFFER_SIZE, file)) > 0) { // Read file in chunks. Don't overflow buffer.
        if (send(socket, buffer, n, 0) != n) { // Read and send all bytes to client
            error("ERROR sending file");
            break;
        }
    }
    fclose(file); // Close the file after sending it
}

int main(int argc, char *argv[]) {
    if (argc < 3) { // Validate command line arguments
        fprintf(stderr, "Usage: %s <port> <file-to-transfer>\n", argv[0]);
        exit(1);
    }

    // Extract port number and filename from command line arguments
    int portno = atoi(argv[1]);
    const char *filename = argv[2];

    // Create a new socket of type Internet and TCP/IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // Initialize socket structure
    struct sockaddr_in serv_addr;
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;         // Family = Internet
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Any IP can connect
    serv_addr.sin_port = htons(portno);     // Convert port num to network byte order

    // Bind the host address using bind call
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    // Begin listening for clients
    listen(sockfd, 5); // Maximum of 5 connection requests queued
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    printf("Server listening on port %d/n", portno);

    // Accept and process client connections
    while (1) {
        // Accept a client connection
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");

        printf("Connection established with client\n");

        // Read mode from client. 1 for echo 2 for file transfer
        char modeBuffer[2];
        memset(modeBuffer, 0, 2); // Clear the buffer to store mode
        int n = read(newsockfd, modeBuffer, 1); // Read 1 byte from client (the mode)
        if (n < 0) error("ERROR reading from socket");

        int mode = atoi(modeBuffer); // Convert mode to integer
        if (mode == 1) {
            echo(newsockfd); // Echo mode selected
        } else if (mode == 2) {
            fileTransferMode(newsockfd, filename); // File transfer mode selected
        } else {
            printf("Invalid mode recieved\n");
        }
        close(newsockfd); // Close the client socket 
        printf("Connection closed\n");
    }

    // Shutdown and close the server socket
    shutdown(sockfd, SHUT_RDWR); // Shutdown socket for read and write
    close(sockfd); // Close socket, release resources

    return 0;
}