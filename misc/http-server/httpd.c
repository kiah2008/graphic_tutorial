#include "httpd.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>

#define CONNMAX 3
#define MAX_RCV_SIZE 1024
#define MAX_HEAD_SIZE 7

static int listenfd, clients[CONNMAX];
static void error(char *);
static void startServer(const char *);
static void respond(int);

typedef struct
{
    char *cmd;
    int duration;
    int argc;
    char *argv[0];
} cmd_t;

typedef struct
{
    char *name, *value;
} header_t;
static header_t reqhdr[MAX_HEAD_SIZE] = {{"\0", "\0"}};

static int clientfd;

static char *buf;

void serve_forever(const char *PORT)
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen;
    char c;

    int slot = 0;

    printf(
        "Server started %shttp://127.0.0.1:%s%s\n",
        "\033[92m", PORT, "\033[0m\n");

    // Setting all elements to -1: signifies there is no client connected
    int i;
    for (i = 0; i < CONNMAX; i++)
        clients[i] = -1;

    startServer(PORT);

    // Ignore SIGCHLD to avoid zombie threads
    signal(SIGCHLD, SIG_IGN);

    // ACCEPT connections
    while (1)
    {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen);

        if (clients[slot] < 0)
        {
            perror("accept() error");
        }
        else
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                respond(slot);
                exit(0);
            }
            else
            {
                waitpid(pid, NULL, 0);
                clients[slot] = -1;
            }
        }
    }
}

// start server
void startServer(const char *port)
{
    struct addrinfo hints, *res, *p;

    // getaddrinfo for host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        perror("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p != NULL; p = p->ai_next)
    {
        int option = 1;
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (listenfd == -1)
            continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
    }
    if (p == NULL)
    {
        perror("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if (listen(listenfd, 1000000) != 0)
    {
        perror("listen() error");
        exit(1);
    }
}

// get request header
char *request_header(const char *name)
{
    header_t *h = reqhdr;
    while (h->name)
    {
        if (strcmp(h->name, name) == 0)
            return h->value;
        h++;
    }
    return NULL;
}

// client connection
void respond(int n)
{
    int rcvd, fd, bytes_read;
    char *ptr;

    buf = malloc(MAX_RCV_SIZE);
    rcvd = recv(clients[n], buf, MAX_RCV_SIZE, 0);

    if (rcvd < 0) // receive error
        fprintf(stderr, ("recv() error\n"));
    else if (rcvd == 0) // receive socket closed
        fprintf(stderr, "Client disconnected upexpectedly.\n");
    else // message received
    {
        buf[rcvd] = '\0';
        printf("rcv content %s\n", buf);
        method = strtok(buf, " \t\r\n");
        uri = strtok(NULL, " \t");
        prot = strtok(NULL, " \t\r\n");

        fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);

        if (qs = strchr(uri, '?'))
        {
            *qs++ = '\0'; // split URI
        }
        else
        {
            qs = uri - 1; // use an empty string
        }

        header_t *h = reqhdr;
        char *t, *t2;
        while (h < reqhdr + MAX_HEAD_SIZE)
        {
            char *k, *v;
            k = strtok(NULL, "\r\n: \t");
            if (!k)
                break;
            v = strtok(NULL, "\r\n");
            while (*v && *v == ' ')
                v++;
            h->name = k;
            h->value = v;
            h++;
            printf("[H] %s: %s\n", k, v);
            t = v + 1 + strlen(v);
            if (t[1] == '\r' && t[2] == '\n')
                break;
        }
        t+=3;                                   // now the *t shall be the beginning of user payload
        t2 = request_header("Content-Length"); // and the related header if there is
        payload = t;
        payload_size = t2 ? atol(t2) : (rcvd - (t - buf));
        if (payload_size > 0) {
            t[payload_size] = '\0';
            h->name = "payload";
            h->value = payload;
        }
        
        // bind clientfd to stdout, making it easier to write
        clientfd = clients[n];
        dup2(clientfd, STDOUT_FILENO);
        close(clientfd);

        // call router
        route();

        struct timeval tv;
        gettimeofday(&tv, NULL);
        printf("index: %d; %s ts:%.2f\r\n", n, payload, tv.tv_sec + tv.tv_usec / 1.0E6);
        // tidy up
        fflush(stdout);
        shutdown(STDOUT_FILENO, SHUT_WR);
        close(STDOUT_FILENO);
    }

    // Closing SOCKET
    shutdown(clientfd, SHUT_RDWR); // All further send and recieve operations are DISABLED...
    close(clientfd);
    clients[n] = -1;
}
