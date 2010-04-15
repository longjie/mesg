#include "mesg.h"

static int debug = 0;

extern int mesg_udp_menu_numb;
extern mesg_udp_menu_t mesg_udp_menu_vect[];

/**
 * オプション
 * --udp-self 127.0.0.1:10000
 * --udp-send 127.0.0.1:10000
 */
int
mesg_udp_getopt (int argc, char** argv, char* self_addr, int* self_port, char** send_addr, int* send_port, int* send_numb)
{
    int c;
    int i;
    int digit_optind = 0;

    static struct option long_options[] = {
        {"udp-self", 1, 0, 0},
        {"udp-send", 1, 0, 1},
        {"help", 0, 0, 2},
        {0, 0, 0, 0}
    };

    /* 知らないオプションは無視 */
    opterr = 0;
    
    *send_numb = 0;
    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;

        c = getopt_long(argc, argv, "", long_options, &option_index);
        if (c == -1) {
            break;
        }
        /* printf("option %s", long_options[option_index].name);
        if (optarg) {
            printf(" with arg %s", optarg);
        }
        printf("\n");
        */
        switch (c) {
        case 0:
            // self_portのコピー
            for (i=0; i<128; i++) {
                if (optarg[i] == ':') {
                    optarg[i] = '\0';
                    strcpy (self_addr, optarg);
                    break;
                }
            }		
            *self_port = atoi (&optarg[i+1]);
            break;
        case 1:
            // sendのコピー
            for (i=0; i<128; i++) {
                if (optarg[i] == ':') {
                    optarg[i] = '\0';
                    /*
                    fprintf (stderr, "send_numb: %d\n", *send_numb);
                    fprintf (stderr, "send_addr: %s\n", send_addr[*send_numb]);
                    */
                    strcpy (send_addr[*send_numb], optarg);
                    break;
                }
            }
            send_port[*send_numb] = atoi (&optarg[i+1]);
            (*send_numb) ++;
            break;
        case 2:
            // help
            fprintf (stderr, "test_mesg_udp\n");
            fprintf (stderr, "options: ");
            fprintf (stderr, "--udp-self: address:port of this process.");
            fprintf (stderr, "--udp-send: address:port of the process to send message.");
            exit (0);
            break;
        }
    }
}


/**
 *  mesg_udpを生成する
 */
mesg_udp_t*
mesg_udp_create (char* self_addr, int self_port, char** send_addr, int* send_port, int send_numb)
{
    mesg_udp_t* self;
    struct sockaddr_in addr;
    int i;

    /* オブジェクト生成 */
    self = (mesg_udp_t*) object_create (sizeof(*self));

    /* 受信の設定 */
    strcpy (self->self_addr, self_addr);
    self->self_port = self_port;

    /* 送信の設定 */
    self->send_numb = send_numb;
    for (i=0; i<send_numb; i++) {
        strcpy (self->send_addr[i], send_addr[i]);
        self->send_port[i] = send_port[i];
    }
    /* ソケットの生成 */
    self->sock = socket (AF_INET, SOCK_DGRAM, 0);
    if (self->sock < 0) {
        perror ("socket");
        return 0;
    }
    /* ソケットのbind */
    addr.sin_family = AF_INET;
    addr.sin_port = htons (self->self_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind (self->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        perror ("bind");
        return 0;
    }

    /* 送信バッファサイズの設定 */
    if (0) {
        int size;
        socklen_t len;
        getsockopt (self->sock, SOL_SOCKET, SO_SNDBUF, (void*) &size, &len);
        fprintf (stderr, "sndbuf[%d]: %d\n\n", len, size);
        exit (0);
    }
    /* TTL */
    if (1) {
        int ttl = 1;
        if (setsockopt (self->sock, IPPROTO_IP, IP_TTL, (char *)&ttl, sizeof(ttl) != 0)) {
            perror ("setsockopt");
            exit (0);
        }
    }
    /* ノンブロッキング設定 */
    if (0) {
        unsigned long val = 1;
#ifdef WIN32
        ioctlsocket (self->sock, FIONBIO, &val);
#else
        ioctl (self->sock, FIONBIO, &val);
#endif
    }
    if (0) {
        int val = 1;
        setsockopt (self->sock, SOL_SOCKET, SO_RCVTIMEO, (void*)&val, sizeof(val));
        setsockopt (self->sock, SOL_SOCKET, SO_SNDTIMEO, (void*)&val, sizeof(val));
        //exit (0);
    }
    if (0) {
        int val = 1;
        setsockopt (self->sock, SOL_SOCKET, SO_DONTROUTE, (void*)&val, sizeof(val));
        //exit (0);
    }
    return self;
}

/* 終了処理 */
void mesg_udp_delete (mesg_udp_t* self)
{
    close (self->sock);
    object_delete (self);
}

int
mesg_udp_recv (mesg_udp_t* self)
{
    int mesg = -1;
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr_in);

    /* 通信用socketの受信処理 */
    do {
        mesg = -1;
        self->recv_size = recvfrom (self->sock, self->recv_buff, sizeof(self->recv_buff), 0, (struct sockaddr*)&addr, &len);
        //self->size = recv (self->sock, self->buff, sizeof(self->buff), 0);

        if (debug) {
            fprintf (stderr, "recv[%d] from %s port %d\n", self->recv_size, inet_ntoa (addr.sin_addr), ntohs (addr.sin_port));
            if (self->recv_size < 0) {
                fprintf (stderr, "errno=%d\n", errno);
            }
        }
        if (self->recv_size > 1) {
            char menu[8];
            int i;
            strcpy (self->recv_addr, inet_ntoa (addr.sin_addr));

            self->recv_port = ntohs (addr.sin_port);

            /* メッセージ名を取得 */
            pack_load (self->recv_buff, "c8", menu);

            if (debug) {
                fprintf (stderr, "menu: %s\n", menu);
            }

            /* 受信関数を呼び出し */
            for (i=0; i<mesg_udp_menu_numb; i++) {
                if (strcmp (mesg_udp_menu_vect[i].name, menu) == 0) {
                    return mesg_udp_menu_vect[i].func (self);
                }
            }
        }
    } while (self->recv_size > 0);
    return mesg;
}

/**
 * ユーザは直接使わない
 */
int mesg_udp_send (mesg_udp_t* self)
{
    struct sockaddr_in addr;
    int size;
    int i;

    for (i=0; i<self->send_numb; i++) {
        /* 送信先を設定 */
        addr.sin_family = AF_INET;
        addr.sin_port = htons (self->send_port[i]);
        addr.sin_addr.s_addr = inet_addr (self->send_addr[i]);
        size = sendto (self->sock, self->send_buff, self->send_size, 0, (struct sockaddr *)&addr, sizeof(addr));
        if (debug) {
            fprintf (stderr, "host = %s, port = %d, size = %d\n", self->send_addr[i], self->send_port[i], size);
        }
    }
    return 0;
}
