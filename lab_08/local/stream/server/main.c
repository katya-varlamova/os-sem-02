#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>

#define SOCKET_NAME "mysocket.s"
#define BUF_SIZE 256
#define OK 0

static int sockfd;

void cleanup_socket(void)
{
    close(sockfd);
    unlink(SOCKET_NAME);
}

void sigint_handler(int signum)
{
    cleanup_socket();
    fprintf(stdout, "Server stopped listening\n");
    exit(OK);
}

int main(void)
{
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("Failed to create socket");
        return EXIT_FAILURE;
    }

    struct sockaddr from;
    unsigned fromlen;
    struct sockaddr srvr_name;
    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCKET_NAME);
    if (bind(sockfd, &srvr_name, strlen(srvr_name.sa_data) + 1 + sizeof(srvr_name.sa_family)) < 0)
    {
        perror("Failed to bind socket");
        return EXIT_FAILURE;
    }

    signal(SIGINT, sigint_handler);
    if (listen(sockfd, 1) < 0)
    {
        perror("Failed to move into listening mode");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Server is listening.\nTo stop server press Ctrl + C.\n");
    char buf[BUF_SIZE];
    for (;;)
    {
        int ns;
        if ((ns = accept(sockfd, &from, &fromlen)) < 0)
        {
            perror("Failed to accept");
            return EXIT_FAILURE;
        }
        int bytes = recv(ns, buf, sizeof(buf), 0);
        if (bytes <= 0) {
            perror("Failed to recv");
            cleanup_socket();
            close(ns);
            return EXIT_FAILURE;
        }
        fprintf(stdout, "Server got: %s\n", buf);

        snprintf(buf, BUF_SIZE, "Server: my pid is %d", getpid());
        bytes = sendto(ns, buf, strlen(buf), 0, &from, fromlen);
        if (bytes <= 0) {
            perror("Failed to send");
            cleanup_socket();
            close(ns);
            return EXIT_FAILURE;
        }
        close(ns);
        fprintf(stdout, "Server sent: %s\n", buf);
    }

}