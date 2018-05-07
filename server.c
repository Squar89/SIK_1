#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "server.h"

int main(int argc, char *argv[]) {
    int sock, inc_sock;
    long port_num;
    char *strtol_end;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    socklen_t client_address_len;
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

    /* bind the socket to a specific address */
    if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        fprintf(stderr, "Error while binding socket\n");
        return 1;
    }

    /* start listening */
    if (listen(sock, QUEUE_LENGTH) < 0) {
        fprintf(stderr, "Error at listen()\n");
        return 1;
    }

    for (;;) {
        client_address_len = sizeof(client_address);
        /* wait for client */
        inc_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_len);
        if (inc_sock < 0) {
            fprintf(stderr, "Error at accept()\n");
            return 1;
        }
        printf("Accepted connection\n");

        /* setup telnet */
        snd_len = write(inc_sock, SETUP_TELNET, strlen(SETUP_TELNET));
        if (snd_len != (long) strlen(SETUP_TELNET)) {
            fprintf(stderr, "Error writing to client\n");
            return 1;
        }

        /* send main menu */
        if (show_menu_main(inc_sock, 0) != 0) {
            return 1;//error occurred in show_menu_main()
        }
    
        /* close connection */
        printf("Ending connection\n");
        if (close(inc_sock) < 0) {
            fprintf(stderr, "Error at close()");
            return 1;
        }
    }

    return 0;
}

int show_menu_main(int client_sock, int crs_position) {
    int main_menu_crs[3] = {6, 15, 24};
    int num_of_options = 3;
    char main_menu[] = "OpcjaA \n\rOpcjaB \n\rKoniec \r\n";
    char option_A[] = "\rA";
    char buffer[BUFFER_SIZE];
    ssize_t snd_len, rd_len;

    /* set cursor(<) on given position */
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
                return show_menu_main(client_sock, imax(0, crs_position - 1));
            }
            else if (strncmp(buffer, ARROW_DOWN, 3) == 0) {
                return show_menu_main(client_sock, imin(num_of_options - 1, crs_position + 1));
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
                    return show_menu_B(client_sock, 0);
                }
                else if (crs_position == 2) {
                    return 0;
                }
            }
        }
    }
}

int show_menu_B(int client_sock, int crs_position) {
    char B_menu_string[] = "OpcjaB1 \n\rOpcjaB2 \n\rWstecz \r\n";
    int B_menu_crs[3] = {7, 17, 26};
    char option_B1[] = "\rB1";
    char option_B2[] = "\rB2";
    int num_of_options = 3;
    ssize_t snd_len, rd_len;
    char buffer[BUFFER_SIZE];

    /* set cursor on given position */
    B_menu_string[B_menu_crs[crs_position]] = '<';

    /* clear and send menu */
    snd_len = write(client_sock, CLEAR_CMD, strlen(CLEAR_CMD));
    if (snd_len != (long) strlen(CLEAR_CMD)) {
        fprintf(stderr, "Error writing to client\n");
        return 1;
    }
    snd_len = write(client_sock, B_menu_string, strlen(B_menu_string));
    if (snd_len != (long) strlen(B_menu_string)) {
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
                return show_menu_B(client_sock, imax(0, crs_position - 1));
            }
            else if (strncmp(buffer, ARROW_DOWN, 3) == 0) {
                return show_menu_B(client_sock, imin(num_of_options - 1, crs_position + 1));
            }
            else if (strncmp(buffer, ENTER, 3) == 0) {
                if (crs_position == 0) {
                    snd_len = write(client_sock, option_B1, strlen(option_B1));
                    if (snd_len != (long) strlen(option_B1)) {
                        fprintf(stderr, "Error writing to client\n");
                        return 1;
                    }
                }
                else if (crs_position == 1) {
                    snd_len = write(client_sock, option_B2, strlen(option_B2));
                    if (snd_len != (long) strlen(option_B2)) {
                        fprintf(stderr, "Error writing to client\n");
                        return 1;
                    }
                }
                else if (crs_position == 2) {
                    return show_menu_main(client_sock, 1);
                }
            }
        }
    }
}
