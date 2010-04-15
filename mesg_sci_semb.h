#ifndef __MESG_SCI_SEMB_H__
#define __MESG_SCI_SEMB_H__

#include <stdio.h>
#include "base64.h"
#include "pack.h"
#include "semb1200a/semb1200a.h"
#include "mesg_sci.h"

struct mesg_sci {

    /* シリアルポート番号 */
    int sock;
    
    /* 受信サイズ */
  int send_size;

    /* 送信バッファ */
  char send_enc_buff[1000];

    /* 送信バッファ */
    char send_buff[1000];

    /* 受信バッファ */
    char recv_buff[1000];

    /* 受信サイズ */
  int recv_size;

  mesg_sci_recv_func_t* func_vect;
};

mesg_sci_t* mesg_sci_semb_create (void);
void mesg_sci_semb_delete (mesg_sci_t* self);

int mesg_sci_semb_recv (mesg_sci_t* self);
int mesg_sci_semb_send (mesg_sci_t* self);
int mesg_sci_semb_cast (mesg_sci_t* self);

#endif /* #ifndef __MESG_SCI_SEMB_H__ */
