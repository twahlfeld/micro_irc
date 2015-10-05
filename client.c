//
// Created by Theodore Ahlfeld on 9/27/15.
//

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/errno.h>

#define MAXBUFF 256

pthread_t input_thread;
pthread_t output_thread;

void die_with_err(char *err_msg)
{
    perror(err_msg);
    exit(1);
}

void die_gracefully(int sock)
{
    pthread_kill(input_thread, 0);
    pthread_kill(output_thread, 0);
    close(sock);
    exit(0);
}

void *receiving(void *p_socket)
{
    int sock = *(int *)p_socket;
    ssize_t len;
    char input[MAXBUFF];
    for(;;) {
        if((len = recv(sock, input, MAXBUFF - 1, 0)) < 0 && errno == EAGAIN) {
            // no data to read
        } else if(len > 0) {
            input[len] = '\0';
            fprintf(stdout, "%s", input);
            fflush(stdout);
        } else {
            pthread_kill(output_thread, 0);
            return NULL;
        }
    }
}

void *sending(void *p_socket)
{
    int sock = *(int *)p_socket;
    char output[80];
    int in_fd = fileno(stdin);
    ssize_t len;

    for(;;) {
        if((len = read(in_fd, output, sizeof(output)-1)) < 0 && errno == EAGAIN) {
            // no data to read
        } else if(len > 0) {
            output[len] = '\0';
            send(sock, output, strlen(output), 0);
            if (strcasecmp(output, "logout") == 0 || strcasecmp(output, "quit") == 0) {
                break;
            }
        } else { // Error Occurred
            break;
        }
    }
    pthread_kill(input_thread, 0);
    return NULL;
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    int sock;
    if(argc != 3) {
        die_with_err("Must specify ip and port");
    }
    struct hostent *he;
    if((he = gethostbyname(argv[1])) == NULL) {
        die_with_err("gethostbyname() failed");
    }
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        die_with_err("socket() failed");
    }
    if(bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        die_with_err("bind() failed");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(
            inet_ntoa(*(struct in_addr *)he->h_addr));
    serv_addr.sin_port        = htons(atoi(argv[2]));
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    pthread_create(&input_thread, NULL, receiving, &sock);
    pthread_create(&output_thread, NULL, sending, &sock);
    pthread_join(input_thread, NULL);
    pthread_join(output_thread, NULL);
    close(sock);
    puts("");
    return 0;
}