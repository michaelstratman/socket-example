#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

static int run_server(uint16_t port)
{
    // TODO
    return -1;
}

static int run_client(const char *server, uint16_t port)
{
    // TODO
    return -1;
}

int main(int argc, char *argv[])
{
    const char *server = NULL;
    uint16_t port = 0;

    if (argc  > 1) {
        if (argc != 3) usage(argv);
        server = argv[1];
        port = atoi(argv[2]);
    }

    if (server != NULL) {
        return run_client(server, port);
    } else {
        return run_server(port);
    }
}
