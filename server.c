#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE     100
#define QUEUE_LENGTH      5
#define MAX_USHORT    65536
#define CLEAR_CMD "\33\143"
#define ARROW_UP     "\e[A"
#define ARROW_DOWN   "\e[B"
#define ENTER          "\r"

int imin(int a, int b) {
    if (a > b) return b;
    else return a;
}

int imax(int a, int b) {
    if (a > b) return a;
    else return b;
}

int menu_main(int client_sock, int crs_position) {
    char main_menu[] = "OpcjaA \n\rOpcjaB \n\rKoniec \r\n";
    char option_A[] = "\rA";
    int main_menu_crs[3] = {6, 15, 24};
    int num_options = 3;
    ssize_t snd_len, rd_len;
    char buffer[BUFFER_SIZE];

    /* set cursor on given position */
    main_menu[main_menu_crs[crs_position]] = '<';

    /* clear and send menu */
    snd_len = write(client_sock, CLEAR_CMD, strlen(CLEAR_CMD));
    if (snd_len != (long) strlen(CLEAR_CMD)) {
        fprintf(stderr, "Error writing to client\n");
        return 1;
    }
    snd_len = write(client_sock, main_menu, strlen(main_menu));
    if (snd_len != (long) strlen(main_menu)) {
        fprintf(stderr, "Error writing to client\n");
        return 1;
    }

    while (1) {
        rd_len = read(client_sock, buffer, sizeof(buffer));
        if (rd_len < 0) {
            fprintf(stderr, "Error reading from client\n");
            return 1;
        }
        else {
            if (strncmp(buffer, ARROW_UP, 3) == 0) {
                return menu_main(client_sock, imax(0, crs_position - 1));
            }
            else if (strncmp(buffer, ARROW_DOWN, 3) == 0) {
                return menu_main(client_sock, imin(num_options - 1, crs_position + 1));
            }
            else if (strncmp(buffer, ENTER, 3) == 0) {
                if (crs_position == 0) {
                    snd_len = write(client_sock, option_A, strlen(option_A));
                    if (snd_len != (long) strlen(option_A)) {
                        fprintf(stderr, "Error writing to client\n");
                        return 1;
                    }
                }
                else if (crs_position == 1) {
                    //opcja B
                }
                else if (crs_position == 2) {
                    return 0;
                }
            }
        }
    }

    return 0;
}

int menu_B() {
    char B_menu[] = "OpcjaB1 \n\rOpcjaB2 \n\rWstecz \r\n";
    int B_menu_crs[3] = {7, 17, 26};

    return 0;
}

int main(int argc, char *argv[])
{
    int sock, msg_sock, ret_val;
    long port_num;
    char *strtol_end;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_address_len;

    char send_char_command[] = "\377\375\042\377\373\001";
    ssize_t snd_len;

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

        /* send this command so telnet will send every character of input separately */
        snd_len = write(msg_sock, send_char_command, strlen(send_char_command));
        if (snd_len != (long) strlen(send_char_command)) {
            fprintf(stderr, "Error writing to client\n");
            return 1;
        }

        ret_val = menu_main(msg_sock, 0);
        if (ret_val != 0) {
            return 1;
        }
    
        printf("ending connection\n");
        if (close(msg_sock) < 0) {
            fprintf(stderr, "Error at close()");
            return 1;
        }
    }

    return 0;
}
