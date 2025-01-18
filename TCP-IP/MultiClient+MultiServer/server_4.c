#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#define BUFFER_SIZE 1024


//Service 4
char* handle_client_service4(long start_seconds)
{
    static char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);   // Clear the buffer - réinitialisation
    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    long seconds = end_time.tv_sec - start_seconds;
    snprintf(buffer, BUFFER_SIZE, "%ld secondes", seconds);
    return buffer; // Return the dynamically allocated buffer

}

void start_server(int port) {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Create server socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind socket to port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening
    if (listen(server_sock, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    // Accept and handle client connections
    while (1) {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected.\n");
        int bytes_read;
        ssize_t bytes_sent;
        time_t start_seconds;
        while (1) {

            // Buffer réinitilisation
            memset(buffer, 0, BUFFER_SIZE);

            // Read client request
            bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_read <= 0) {
                if (bytes_read == 0) {
                    printf("Client disconnected normally\n");
                } else {
                    perror("recv failed");
                }
                break;
            }

            buffer[BUFFER_SIZE] = '\0';

            printf("Received choice : %s \n", buffer);

            if (strcmp(buffer, "4") == 0){
                printf("Accepting request \n");
                //Sends confirmation to the load balancer
                const char* confirm_msg = "OK\n";
                bytes_sent = send(client_sock, confirm_msg, strlen(confirm_msg), 0);
                if (bytes_sent < 0) {
                    perror("Confirmation message send failed");
                    break;
                }
                printf("Sent %zd confirmation bytes to client\n", bytes_sent);

                memset(buffer, 0, BUFFER_SIZE);


                // Gets the start time
                bytes_read = recv(client_sock, &start_seconds, sizeof(time_t), 0);
                if (bytes_read <= 0) {
                    if (bytes_read == 0) {
                        printf("Client disconnected normally\n");
                    } else {
                        perror("recv failed");
                    }
                    break;
                }

                char* response = handle_client_service4(start_seconds); // Get the response
                printf("RESPONSE : %s", response);
                fflush(stdout);
                // Send time lapse between start and end time.
                send(client_sock, response, BUFFER_SIZE, 0);

        


            }

        }

        printf("Client disconnected.\n");
        close(client_sock);
    }

    close(server_sock);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    start_server(port); 

    return 0;
}


        // while ((bytes_read = read(client_sock, &start_seconds, sizeof(time_t))) > 0) {
        //     printf("BUFFFF : %s", buffer);
        //     char* response = handle_client_service4(start_seconds); // Get the response
        //     printf("RESPONSE : %s", response);
        //     fflush(stdout);
        //     send(client_sock, response, BUFFER_SIZE, 0);
        // }