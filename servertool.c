#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

double measure_bandwidth(int sockfd, char *data, int data_size) {
    clock_t start_time, end_time;
    double time_taken;
    int bytes_sent, bytes_received;

    start_time = clock();

    // Send data to the client
    bytes_sent = write(sockfd, data, data_size);
    if (bytes_sent < 0)
        error("error sending data");

    // Receive data from the client
    char buffer[data_size];
    bytes_received = read(sockfd, buffer, data_size);
    if (bytes_received < 0)
        error("error receiving data");

    end_time = clock();

    // Calculate time taken
    time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    // Calculate bandwidth (in kilobytes per second)
    double bandwidth = (bytes_sent + bytes_received) / (time_taken * 1024);

    return bandwidth;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Port number not provided\n");
        exit(EXIT_FAILURE);
    }

    int sockfd, newsockfd, portno;
    socklen_t clilen; // Change from int to socklen_t
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");

    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); // Pass &clilen instead of clilen
    if (newsockfd < 0) 
        error("ERROR on accept");

    printf("Chat session established.\n");

    // Start chat session
    while(1) {
        bzero(buffer,256);
        n = read(newsockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        printf("Client: %s\n",buffer);
        printf("Server: ");
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        n = write(newsockfd,buffer,strlen(buffer));
        if (n < 0) error("ERROR writing to socket");

        // Check for termination message
        if (strncmp(buffer, "bye", 3) == 0)
            break;
    }

    // Measure and display bandwidth
    double bandwidth = measure_bandwidth(newsockfd, buffer, strlen(buffer));
    printf("Bandwidth: %.2f KB/s\n", bandwidth);

    close(newsockfd);
    close(sockfd);
    return 0; 
}




