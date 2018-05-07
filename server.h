#ifndef __serverh__
#define __serverh__

#define BUFFER_SIZE                          10
#define QUEUE_LENGTH                          5
#define MAX_USHORT                        65536
#define CLEAR_CMD                     "\33\143"
#define ARROW_UP                         "\e[A"
#define ARROW_DOWN                       "\e[B"
#define ENTER                              "\r"
#define SETUP_TELNET "\377\375\042\377\373\001"

static int show_menu_main(int client_sock, int crs_position);

static int show_menu_B(int client_sock, int crs_position);

static inline int imin(int a, int b) {
    if (a > b) return b;
    else return a;
}

static int imax(int a, int b) {
    if (a > b) return a;
    else return b;
}

#endif /* __serverh__ */