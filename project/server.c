#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <dirent.h>

char* find_matching_file(const char* filename) {
    DIR* dir;
    struct dirent* entry;
    char* matching_filename = NULL;

    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir failed");
        exit(1);
    }

    while ((entry = readdir(dir)) != NULL) {
        // Check if the entry is a file
        if (entry->d_type == DT_REG) {
            // Check if the filename matches (case-insensitive)
            if (strcasecmp(filename, entry->d_name) == 0) {
                matching_filename = strdup(entry->d_name);
                break;
            }
        }
    }

    closedir(dir);

    return matching_filename;
}


void url_decode(char *src) {
    char *dest = src;
    while (*src != '\0') {
        if (*src == '%' && *(src + 1) && *(src + 2)) {
            char hex[3];
            hex[0] = *(src + 1);
            hex[1] = *(src + 2);
            hex[2] = '\0';
            int hex_value = strtol(hex, NULL, 16);
            *dest = (char) hex_value;
            dest++;
            src += 3;
        } else {
            *dest = *src;
            dest++;
            src++;
        }
    }
    *dest = '\0';
}


void to_lower(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}


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

        printf("Here is the message: %s\n\n",buffer);



        // parsing the slash part 

        char* request_path = strtok(buffer, " \t\n");
        if (request_path != NULL) {
            request_path = strtok(NULL, " \t\n"); // get the path
        }

        
        url_decode(request_path);
        to_lower(request_path);
        // prepare response message
        char response[1024];
        memset(response, 0, sizeof(response));
        printf("Here is the request path: %s\n\n",request_path);

        if (request_path != NULL) {

            // different file types requested
            int found = 0;

            // getting extension
            char* extension = strrchr(request_path, '.');
            if (extension == NULL || strchr(extension, '/') != NULL) {
    

                printf("raw file detected\n\n");
                printf("Here is the raw name: %s\n\n\n", request_path + 1);

                FILE* file = NULL;
                char* match = find_matching_file(request_path + 1);
                if (match != NULL) {
                    file = fopen(match, "r");
                }
                
                if (file != NULL) {
                    // file found, read contents
                    found = 1;
                    char file_contents[1048576];
                    int file_size = fread(file_contents, 1, 1048576, file);

                    // build response
                    char response_header[128];
                    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: %d\r\n\r\n", file_size);
                    printf("Here is the reponse header name: %s\n\n\n", response_header);

                    // send response header
                    
                    n = write(newsockfd, response_header, strlen(response_header));
                    if (n < 0) {
                        perror("ERROR writing to socket");
                        exit(1);
                    }

                    // send file contents
                    //n = write(newsockfd, file_contents, file_size);
                    n = send(newsockfd, file_contents, file_size, 0);

                    if (n < 0) {
                        perror("ERROR writing to socket");
                        exit(1);
                    }
                    fclose(file);
                }
            }
           
            // if it is a .txt file that is found
            else if (strcmp(request_path + strlen(request_path) - 4, ".txt") == 0) {

                printf("txt file detected\n\n");
                printf("Here is the txt name: %s\n\n\n", request_path + 1);

                //FILE* file = fopen(request_path + 1, "r");
                

                FILE* file = NULL;
                char* match = find_matching_file(request_path + 1);
                if (match != NULL) {
                    file = fopen(match, "r");
                }

                if (file != NULL) {
                    // file found, read contents
                    found = 1;
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
                    fclose(file);
                }
            }
             // if it is a .html file that is found
            else if (strcmp(request_path + strlen(request_path) - 5, ".html") == 0) {

                printf("html file detected\n\n");
                printf("Here is the html name: %s\n\n\n", request_path + 1);

                // FILE* file = fopen(request_path + 1, "r");
                

                FILE* file = NULL;
                char* match = find_matching_file(request_path + 1);
                if (match != NULL) {
                    file = fopen(match, "r");
                }

                if (file != NULL) {
                    // file found, read contents
                    found = 1;
                    char file_contents[4096];
                    int file_size = fread(file_contents, 1, 4096, file);

                    // build response
                    char response_header[128];
                    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", file_size);
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
                    fclose(file);
                }
            }

            else if (strcmp(request_path + strlen(request_path) - 4, ".htm") == 0) {

                printf("htm file detected\n\n");
                printf("Here is the htm name: %s\n\n\n", request_path + 1);

                // FILE* file = fopen(request_path + 1, "r");
                

                FILE* file = NULL;
                char* match = find_matching_file(request_path + 1);
                if (match != NULL) {
                    file = fopen(match, "r");
                }

                if (file != NULL) {
                    // file found, read contents
                    found = 1;
                    char file_contents[4096];
                    int file_size = fread(file_contents, 1, 4096, file);

                    // build response
                    char response_header[128];
                    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", file_size);
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
                    fclose(file);
                }
            }

            else if (strcmp(request_path + strlen(request_path) - 4, ".jpg") == 0) {

                printf("jpg file detected\n\n");
                printf("Here is the jpg name: %s\n\n\n", request_path + 1);

                //FILE* file = fopen(request_path + 1, "rb");

                FILE* file = NULL;
                char* match = find_matching_file(request_path + 1);
                printf("matching file is: %s\n\n", match);
                if (match != NULL) {
                    file = fopen(match, "rb");
                }

                if (file != NULL) {
                    // file found, read contents
                    found = 1;
                    char file_contents[32768];
                    int file_size = fread(file_contents, 1, 32768, file);

                    // build response
                    char response_header[128];
                    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", file_size);
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
                    fclose(file); 
                }
            }

            else if (strcmp(request_path + strlen(request_path) - 5, ".jpeg") == 0) {

                printf("jpeg file detected\n\n");
                printf("Here is the jpeg name: %s\n\n\n", request_path + 1);

                //FILE* file = fopen(request_path + 1, "rb");

                FILE* file = NULL;
                char* match = find_matching_file(request_path + 1);
                printf("matching file is: %s\n\n", match);
                if (match != NULL) {
                    file = fopen(match, "rb");
                }

                if (file != NULL) {
                    // file found, read contents
                    found = 1;
                    char file_contents[32768];
                    int file_size = fread(file_contents, 1, 32768, file);

                    // build response
                    char response_header[128];
                    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %d\r\n\r\n", file_size);
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
                    fclose(file); 
                }
            }

            else if (strcmp(request_path + strlen(request_path) - 4, ".png") == 0) {

                printf("png file detected\n\n");
                printf("Here is the png name: %s\n\n\n", request_path + 1);

                //FILE* file = fopen(request_path + 1, "rb");

                FILE* file = NULL;
                char* match = find_matching_file(request_path + 1);
                printf("matching file is: %s\n\n", match);
                if (match != NULL) {
                    file = fopen(match, "rb");
                }

                if (file != NULL) {
                    // file found, read contents
                    found = 1;
                    char file_contents[32768];
                    int file_size = fread(file_contents, 1, 32768, file);

                    // build response
                    char response_header[128];
                    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\nContent-Length: %d\r\n\r\n", file_size);
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
                    fclose(file); 
                }
            }



            // checking if file is not found
            if (found == 0) {
                printf("File not found uh oh\n\n");   
                n = write(newsockfd, "HTTP/1.1 404 Not Found\r\n\r\n", 26);
            }

        }
        else {
            sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Request no GET</h1></body></html>");
            n = write(newsockfd, response, strlen(response));
            if (n < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
        }

        
        // close socket
        
        close(newsockfd);
        printf("END\n\n\n");
    }

    // close socket
    close(sockfd);

    return 0;
}