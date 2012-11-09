#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static void usage(char *argv[])
{
    fprintf(stderr, 
            "Usage: %s <hostname> <port>   (client)\n"
            "       %s [ port ]            (server)\n", 
            argv[0], argv[0]);
    exit(EXIT_FAILURE);
}

static void handle_error(bool failure_condition, const char *msg)
{
    if (failure_condition) {
        perror(msg);
        exit(1);
    }
}

static int run_server(uint16_t port)
{
    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    handle_error(listen_sock < 0, "failed to create socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    // sin_addr == 0
    // sin_port == 0
    addr.sin_port = htons(port);

    int rc = bind(listen_sock, (struct sockaddr *) &addr, sizeof(addr));
    handle_error(rc < 0, "bind failed");

    rc = listen(listen_sock, 5);
    handle_error(rc < 0, "listen failed");

    printf("Listening on port %d\n", ntohs(addr.sin_port));

    int client_sock = accept(listen_sock, NULL, 0);
    handle_error(client_sock < 0, "accept failed");

    char buf[1024];
    int bytes_recvd = 0;
    while (bytes_recvd < 1024) {
        int bytes = recv(client_sock, buf + bytes_recvd, 1024 - bytes_recvd, 0);
        printf("received %d bytes\n", bytes);
        handle_error(bytes < 0, "failed to recv bytes");
        handle_error(bytes == 0, "client closed connection");
        
        bytes_recvd += bytes;
    }

    printf("Received: %s\n", buf);
    
    int bytes = send(client_sock, buf, 1024, 0);
    handle_error(bytes != 1024, "failed to send bytes");

    close(client_sock);
    close(listen_sock);
    return 0;
}

static int run_client(const char *server, uint16_t port)
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    handle_error(sock < 0, "failed to create socket");

    struct hostent *he = gethostbyname(server);
    handle_error(he == NULL, "name lookup failed");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    // sin_addr == 0
    // sin_port == 0
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    printf("Resolved %s to %s\n", server, inet_ntoa(addr.sin_addr));

    int rc = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
    handle_error(rc < 0, "connection failed");

    char buf[1024];
    memset(buf, 0, 1024);
    sprintf(buf, "Hello world!\n");

    int bytes = send(sock, buf, 1024, 0);
    handle_error(bytes != 1024, "failed to send bytes");

    int bytes_recvd = 0;
    while (bytes_recvd < 1024) {
        int bytes = recv(sock, buf + bytes_recvd, 1024 - bytes_recvd, 0);
        handle_error(bytes < 0, "failed to recv bytes");
        handle_error(bytes == 0, "client closed connection");
        
        bytes_recvd += bytes;
    }
    
    printf("Received: %s\n", buf);
    close(sock);
    return 0;
}

int main(int argc, char *argv[])
{
    const char *server = NULL;
    uint16_t port = 0;

    if (argc > 1) {
        if (argc != 3 && argc != 2) usage(argv);
        if (argc == 2) {
            port = atoi(argv[1]);
        } else {
            server = argv[1];
            port = atoi(argv[2]);
        }
    }

    if (server != NULL) {
        return run_client(server, port);
    } else {
        return run_server(port);
    }
}
