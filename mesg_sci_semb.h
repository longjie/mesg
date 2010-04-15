#ifndef __MESG_SCI_SEMB_H__
#define __MESG_SCI_SEMB_H__

#include <stdio.h>
#include "base64.h"
#include "pack.h"
#include "semb1200a/semb1200a.h"
#include "mesg_sci.h"

struct mesg_sci {

    /* �V���A���|�[�g�ԍ� */
    int sock;
    
    /* ��M�T�C�Y */
  int send_size;

    /* ���M�o�b�t�@ */
  char send_enc_buff[1000];

    /* ���M�o�b�t�@ */
    char send_buff[1000];

    /* ��M�o�b�t�@ */
    char recv_buff[1000];

    /* ��M�T�C�Y */
  int recv_size;

  mesg_sci_recv_func_t* func_vect;
};

mesg_sci_t* mesg_sci_semb_create (void);
void mesg_sci_semb_delete (mesg_sci_t* self);

int mesg_sci_semb_recv (mesg_sci_t* self);
int mesg_sci_semb_send (mesg_sci_t* self);
int mesg_sci_semb_cast (mesg_sci_t* self);

#endif /* #ifndef __MESG_SCI_SEMB_H__ */
