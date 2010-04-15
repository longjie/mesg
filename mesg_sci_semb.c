#include "mesg_sci_semb.h"
#include <string.h>

/**
 *  mesg_sci_sembを生成する
 */
mesg_sci_t* mesg_sci_create (void)
{
  static mesg_sci_t self;
  return &self;
}

/* 終了処理 */
void mesg_sci_delete (mesg_sci_t* self)
{
}

static char enc_buff[2048];
static char enc_size = 0;
/**
 *  @brief  受信処理
 *  @return 処理したmesgの数
 *
 *  HOSからはポーリング処理される
 */
int
mesg_sci_recv (mesg_sci_t* self)
{
  short mesg;
  int numb = 0;
  int i;
  char menu[8];

#if 0
  /* 文字の受信 */
  for (;;) {
    int c;
    /* 受信文字なし */
    if (uart1_getchar (&c) == 0) {
      return numb;
    }
    if (c == '\n') {
      break;
    }
    enc_buff[enc_size++] = c;
  }
#endif
  /* 文字の受信 */
  for (;;) {
    int c;
    /* 受信文字なし */
    while (uart1_getchar (&c) == 0) {
      dly_tsk (1);
    }
    if (c == '\n') {
      break;
    }
    enc_buff[enc_size++] = c;
  }
  /* BASE64デコード */
  self->recv_size = base64_decode (self->recv_buff, enc_buff, enc_size);
  enc_size = 0;
  /* mesg番号 */
  pack_load (self->recv_buff, "c8", menu);

  /* 受信関数を呼び出し */
  for (i=0; i<mesg_sci_menu_numb; i++) {
    if (strcmp (mesg_sci_menu_vect[i].name, menu) == 0) {
      mesg_sci_menu_vect[i].func (self);
    }
  }
  numb ++;

  return 0;
  //}
}

/**
 *  @brief  送信処理
 *  
 *  どうするか？送り終わるまで待つのか？
 */
int mesg_sci_send (mesg_sci_t* self)
{
  int size;
  char buff[1000];
  int i;

  /* BASE64エンコード */
  size = base64_encode (buff, self->send_buff, self->send_size);
  /* 送信 */
  for (i=0; i<size; i++) {
    while( ( io_in16( UART_1LSR ) & 0x20 ) == 0 ) {
      dly_tsk (1);
    }
    uart1_putchar (buff[i]);
  }
  uart1_putchar ('\n');
  return size;
}
