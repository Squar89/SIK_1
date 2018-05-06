#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

//#define BUFFER_SIZE   2000
#define QUEUE_LENGTH     5
#define MAX_USHORT   65536

int main(int argc, char *argv[])
{
    int sock, msg_sock;
    long port_num;
    char *strtol_end;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_address_len;

    //char buffer[BUFFER_SIZE];
    //ssize_t len, snd_len;

    /* check if server was run with correct parameters */
    if (argc != 2) {
        fprintf(stderr, "Wrong number of parameters\nCorrect format is: {./server port_num}\n");
        return 1;
    }
    port_num = strtol(argv[1], &strtol_end, 10);
    if (port_num <= 0 || port_num > MAX_USHORT) {
        fprintf(stderr, "Incorrect port number\n");
        return 1;
    }

    /* create socket */
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Error while creating socket\n");
        return 1;
    }

    server_address.sin_family = AF_INET;//IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);//listening on all interfaces
    server_address.sin_port = htons(port_num);//listening on port port_num

    /* bind the socket to a concrete address */
    if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "Error while binding socket\n");
        return 1;
    }

    /* switch to listening (passive open) */
    if (listen(sock, QUEUE_LENGTH) < 0) {
        fprintf(stderr, "Error at listen()\n");
        return 1;
    }

    printf("accepting client connections on port %hu\n", ntohs(server_address.sin_port));
    for (;;) {
        client_address_len = sizeof(client_address);
        // get client connection from the socket
        msg_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
        if (msg_sock < 0) {
            fprintf(stderr, "Error at accept()\n");
            return 1;
        }
        printf("Accepted connection\n");
        /*
        do {
            len = read(msg_sock, buffer, sizeof(buffer));
            if (len < 0) {
                //syserr("reading from client socket");
            }
            else {
                printf("read from socket: %zd bytes: %.*s\n", len, (int) len, buffer);
                snd_len = write(msg_sock, buffer, len);
                if (snd_len != len) {
                    //syserr("writing to client socket");
                }
            }
        } while (len > 0);
        */
        printf("ending connection\n");
        if (close(msg_sock) < 0) {
            //syserr("close");
        }
    }

    return 0;
}
