#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    int sockfd, newsockfd, portno, clilen;
    char buffer[4096];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    else {
        printf("socket succesfully connected");
    }

    // initialize server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    portno = 8080;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // bind socket to address
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }
    else {
        printf("socket succesfully binded");
    }

    // listen for incoming connections
    listen(sockfd, 5);

    while(1) {
        // accept connection
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        // read from socket
        memset(buffer, 0, 4096);
        n = read(newsockfd, buffer, 4096);
        if (n < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }

        printf("Here is the message: %s\n\n\n",buffer);



        // parsing the slash part 

        char* request_path = strtok(buffer, " \t\n");
        if (request_path != NULL) {
            request_path = strtok(NULL, " \t\n"); // get the path
        }


        // bool what file or html
        int html = 1;
        // prepare response message
        char response[1024];
        memset(response, 0, sizeof(response));
      

        if (request_path != NULL) {
           
            // if it is a .txt file that is found
            int txt = 0;
            if (strcmp(request_path + strlen(request_path) - 4, ".txt") == 0) {
                txt = 1;
            }

            if (txt) {
                html = 0;
                
                // open file for reading
                printf("Here is the txt name: %s\n\n\n", request_path + 1);

                FILE* file = fopen(request_path + 1, "r");

                if (file == NULL) {
                    // file not found
                    printf("File not foun uh oh");
                    // n = write(newsockfd, "HTTP/1.1 404 Not Found\r\n\r\n", 26);
                    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>No txt match</h1></body></html>");
                    n = write(newsockfd, response, strlen(response));
                } else {
                    // file found, read contents
                    char file_contents[512];
                    int file_size = fread(file_contents, 1, 512, file);

                    // build response
                    char response_header[128];
                    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n", file_size);
                    printf("Here is the reponse header name: %s\n\n\n", response_header);

                    // send response header
                    n = write(newsockfd, response_header, strlen(response_header));
                    if (n < 0) {
                        perror("ERROR writing to socket");
                        exit(1);
                    }

                     // send file contents
                    n = write(newsockfd, file_contents, file_size);
                    if (n < 0) {
                        perror("ERROR writing to socket");
                        exit(1);
                    }
                }
            
            }
            else {
                sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Path, not txt: %s!</h1></body></html>", request_path+1);
            }
        }
        else {
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Request no GET</h1></body></html>");
        }

        // write to socket
        if (html) {
            n = write(newsockfd, response, strlen(response));
            if (n < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
        }

        // close socket
        close(newsockfd);
    }

    // close socket
    close(sockfd);

    return 0;
}