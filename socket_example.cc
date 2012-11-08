#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(char *argv[])
{
    fprintf(stderr, "Usage: %s [ -c <hostname> ]\n", argv[0]);
    exit(EXIT_FAILURE);
}

static int run_server()
{
    // TODO
    return -1;
}

static int run_client(const char *server)
{
    // TODO
    return -1;
}

int main(int argc, char *argv[])
{
    const char *server = NULL;
    char ch;
    while ((ch = getopt(argc, argv, "c:")) != -1) {
        switch (ch) {
        case 'c':
            server = optarg;
            break;
        default:
            usage(argv);
        }
    }

    if (server != NULL) {
        return run_client(server);
    } else {
        return run_server();
    }
}
