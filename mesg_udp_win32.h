#ifndef __MESG_UDP_H__
#define __MESG_UDP_H__

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <unistd.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <getopt.h>
#include "taco.h"
#include "pack.h"

typedef struct {
    /* IP */
    char host[256];

    /* ポート番号 */
    int port;

} mesg_unit_t;

mesg_unit_t* mesg_unit_create (char* host, int port);
taco_hash_t* mesg_unit_hash_create (char* file);


typedef struct {

    int sock;

    /* unit hash table */
    // taco_hash_t* hash;

    /* 自分のアドレス */
    char self_addr[128];

    /* 自分のポート番号 */
    int self_port;

    /* 受信アドレス */
    char recv_addr[128];

    /* 受信ポート番号 */
    int recv_port;

    /* 送信アドレス */
    char send_addr[10][128];

    /* 送信ポート番号 */
    int send_port[10];

    /* 送信先の数 */
    int send_numb;

    /* 送信バッファ */
    char send_buff[3000];
    int send_size;

    /* 受信バッファ */
    char recv_buff[3000];
    int recv_size;

    /* mesg_recv_func_t* func_vect; */
} mesg_udp_t;

typedef struct {
    char name[8];

    int (*func)(mesg_udp_t* self);

} mesg_udp_menu_t;

typedef int (*mesg_udp_recv_func_t) (mesg_udp_t*);

int mesg_udp_getopt (int argc, char** argv, char* self_addr, int* self_port, char** send_addr, int* send_port, int* send_numb);

mesg_udp_t* mesg_udp_create (char* recv_addr, int recv_port, char** send_addr, int* send_port, int send_numb);
void mesg_udp_delete (mesg_udp_t* self);
void mesg_udp_clear_cast (mesg_udp_t* self);
int mesg_udp_append_cast(mesg_udp_t* self, char* name);

int mesg_udp_recv (mesg_udp_t* self);
int mesg_udp_send (mesg_udp_t* self);
int mesg_udp_cast (mesg_udp_t* self);

#endif /* #ifndef __MESG_UDP_H__ */
