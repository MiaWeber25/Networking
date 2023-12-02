/* 
Created by: Mia Weber
12-02-2023
Description: Client program that supports two modes - echo (1) and file transfer (2).
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

// Functionality for echo mode - sends message to server & reads the response
void echoUtil(int sockfd, const char *message) {
    char buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, MAX_BUFFER_SIZE);        // Clear the buffer
    write(sockfd, message, strlen(message));   // Send message to the server
    read(sockfd, buffer, MAX_BUFFER_SIZE - 1); // Read response from server into the buffer until full
    printf("Echo from server: %s\n", buffer);  // Print the server's response
}

// Echo mode logic - continuously sends message to the server & recieves echoed messages
void echo(int sockfd) {
    char buffer[MAX_BUFFER_SIZE];
    while (1) {
        printf("Enter message: ");
        fgets(buffer, MAX_BUFFER_SIZE, stdin); // Read user input
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline char

       echoUtil(sockfd, buffer); // Use echo utility function (send to server and read response)

        if (strncmp(buffer, "close", 5) == 0) { // Break loop is "close" is entered
            break;
        }
    }
}

// File transfer mode logic - recieves a file from server and writes it to specified local file
void fileTransfer(int sockfd, const char *filename) {
    FILE *file = fopen(filename, "wb"); // Open file for writing in binary mode
    if (file == NULL) {
        error("ERROR opening file");
        return;
    }

    char buffer[MAX_BUFFER_SIZE];
    int n;

    // Recieve data from server and write to file
    while ((n = recv(sockfd, buffer, MAX_BUFFER_SIZE, 0)) > 0) {
        if (fwrite(buffer, 1, n, file) != n) {
            error("ERROR writing to file");
            break;
        }
    }

    fclose(file); // Close the file after recieving data
}

// Sets up the client and handles user-specified mode (1 or 2)
int main(int argc, char *argv[]) {
    if (argc < 4) { // Validate command line arguments
        fprintf(stderr, "Usage: %s <server_ip> <port> <mode> [filename]\n", argv[0]);
        exit(1);
    }

    // Parse command line arguments to get port number and mode
    int portno = atoi(argv[2]);
    int mode = atoi(argv[3]); // 1 for echo, 2 for file transfer
    const char *filename = (mode == 2 && argc == 5) ? argv[4] : "recieved_file";

    // Create a new socket of type Internet and TCP/IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    // Configure server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); // Initialize server address structure
    serv_addr.sin_family = AF_INET;           // Set family to Internet
    serv_addr.sin_port = htons(portno);       // Convert port number to network byte order 
    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        error("Invalid address/ Address not supported");
    }

    // Establish a connection to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("Connection Failed");
    }

    // Sending the mode to the server
    char modeBuffer[2];
    sprintf(modeBuffer, "%d", mode);
    write(sockfd, modeBuffer, strlen(modeBuffer));

    // Handle client operation based on selected mode
    if (mode == 1) {
        echo(sockfd); // Invoke echo mode functionality
    } else if (mode == 2) {
        fileTransfer(sockfd, filename); // Invoke file transfer mode functionality
    } else {
        fprintf(stderr, "Invalid mode. Use 1 for echo, 2 for file transfer.\n");
    }
    close(sockfd); // Close the socket, release resources
    return 0;
}