#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* EECS 482 Discussion: socket programming
 *
 * A simple client-server application.
 * 
 * Operation:
 * 1) Server listens for connections.
 * 2) Client connects, server accepts.
 * 3) Client sends BUFSIZE bytes.
 * 4) Server receives BUFSIZE bytes and prints them as a string.
 *    Server then sends the exact same BUFSIZE bytes back.
 *    Server closes connection and exits.
 * 5) Client receives BUFSIZE bytes, prints them, closes connection and exits.
 *
 * Optional exercises left to the student:
 *  - Make the server able to handle more than one client.
 *    (i.e. the server should continue accepting connections after the first one)
 *  - Support multiple simultaneous clients.
 *  - Change the protocol to send an arbitrary-length message.
 *  - Change the client to send data read from standard input.
 *    This is similar to a program called 'netcat' (or 'nc' for short).
 *     - Bonus: allow the server to send replies at any time.
 *       (this one is trickier; ask me if you're interested)
 *  - Change the protocol to send other data besides ASCII strings.
 *     (Or wait for the discussion on RPC for this one)
 *
 *  - And of course, clean up the code a bit to remove duplication. :-)
 *
 * If you want to play around with this and get feedback, 
 *  feel free to fork the repo on github and try some of these tweaks.
 */

static const size_t BUFSIZE = 1024;

static void handle_error(bool failure_condition, const char *msg)
{
    if (failure_condition) {
        perror(msg);
        exit(1);
    }
}

static int run_server(uint16_t port)
{
    // creates a socket.  PF_INET == IPv4; SOCK_STREAM == TCP
    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    handle_error(listen_sock < 0, "failed to create socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; // this is an IPv4 address
    // sin_addr == 0; meaning is "listen on all local network interfaces"
    // sin_port == 0; meaning is "OS, please assign me a port"
    addr.sin_port = htons(port); // convert from host to network byte order

    socklen_t addrlen = sizeof(addr);

    // bind() assigns the address to the socket.
    int rc = bind(listen_sock, (struct sockaddr *) &addr, addrlen);
    handle_error(rc < 0, "bind failed");

    // update addr with (possibly OS-assigned) port number
    rc = getsockname(listen_sock, (struct sockaddr *) &addr, &addrlen);
    handle_error(rc < 0, "getsockname failed");

    // tell OS that you want to accept connections on this socket.
    // 5 == backlog queue; how many clients can be waiting to connect
    rc = listen(listen_sock, 5);
    handle_error(rc < 0, "listen failed");

    printf("Listening on port %d\n", 
           ntohs(addr.sin_port)); // don't forget byte order conversion

    // actually accept a connection.  accept() returns a NEW socket,
    //  one that can be used to send and receive data from the connecting client.
    int client_sock = accept(listen_sock, NULL, 0);
    handle_error(client_sock < 0, "accept failed");

    
    char buf[BUFSIZE + 1]; // just in case the client doesn't send me a NUL byte
    memset(buf, 0, BUFSIZE + 1);

    // XXX: loop-recv should be in a function.
    int bytes_recvd = 0;
    while (bytes_recvd < 1024) {
        int bytes = recv(client_sock, buf + bytes_recvd, 1024 - bytes_recvd, 0);
        printf("received %d bytes\n", bytes);
        handle_error(bytes < 0, "failed to recv bytes");
        handle_error(bytes == 0, "client closed connection");
        
        bytes_recvd += bytes;
    }

    printf("Received: %s\n", buf);
    
    // Aside: if the socket is non-blocking, or if you pass the MSG_DONTWAIT flag,
    //  send could return before copying the entire message into the kernel buffer.
    // However, we're not doing non-blocking I/O here, and the kernel buffer is
    //  currently empty anyways.
    int bytes = send(client_sock, buf, 1024, 0);
    handle_error(bytes != 1024, "failed to send bytes");

    // A very brief conversation indeed. 
    //  (How would you enable a longer interaction between client and server,
    //   or multiple interactions?)
    close(client_sock);
    close(listen_sock);
    return 0;
}

static int run_client(const char *server, uint16_t port)
{
    // same as server side, creates a socket.  PF_INET == IPv4; SOCK_STREAM == TCP
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    handle_error(sock < 0, "failed to create socket");

    // look up the IP address of the given domain name.
    struct hostent *he = gethostbyname(server);
    handle_error(he == NULL, "name lookup failed");
    // XXX: this should actually use the herror() function, I think,
    //      to print information about why the lookup failed.

    // copy the IP address out of the hostent struct
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; // this is an IPv4 address
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

    // XXX: loop-recv should be in a function.
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

static void usage(char *argv[])
{
    fprintf(stderr, 
            "Usage: %s <hostname> <port>   (client)\n"
            "       %s [ port ]            (server)\n", 
            argv[0], argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    const char *server_hostname = NULL;
    uint16_t port = 0;

    if (argc > 1) {
        if (argc != 3 && argc != 2) usage(argv);
        if (argc == 2) {
            // server with port number
            port = atoi(argv[1]);
        } else {
            // client with hostname, port
            server_hostname = argv[1];
            port = atoi(argv[2]);
        }
    } // else; server with OS-assigned listen port

    if (server_hostname != NULL) {
        return run_client(server_hostname, port);
    } else {
        return run_server(port);
    }
}
