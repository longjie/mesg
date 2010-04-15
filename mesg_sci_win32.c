#include "mesg_sci_win32.h"

mesg_sci_t* mesg_sci_create (char* port, int baud)
{
    mesg_sci_t* self;

    // インスタンス生成
    self = malloc (sizeof(*self));
    // シリアルポートを開く
    self->handle = CreateFile(port,
			      GENERIC_READ|GENERIC_WRITE,	// アクセスモード
			      0,				// 共有モード
			      NULL,			// セキュリティ属性
			      OPEN_EXISTING,		// 作成方法フラグ
			      FILE_ATTRIBUTE_NORMAL,	// ファイル属性
			      NULL);			// テンプレート
  
    // シリアルポートの設定
    DCB dcb;
    if (GetCommState(self->handle, &dcb)==FALSE){
	fprintf(stderr, "ERROR! (serial port cannot open)\n");
	CloseHandle(self->handle);
	free (self);
	return 0;
    }
    dcb.BaudRate	= baud;
    dcb.ByteSize 	= 8;
    dcb.Parity 	= NOPARITY;
    dcb.fParity	= FALSE;
    dcb.StopBits	= ONESTOPBIT;
    SetCommState(self->handle, &dcb);

    // タイムアウトの設定
    COMMTIMEOUTS cto;
    GetCommTimeouts(self->handle, &cto);
    cto.ReadIntervalTimeout		= 100;
    cto.ReadTotalTimeoutMultiplier 	= 1;
    cto.ReadTotalTimeoutConstant		= 100;
    cto.WriteTotalTimeoutMultiplier	= 0;
    cto.WriteTotalTimeoutConstant 	= 0;
    if(SetCommTimeouts(self->handle, &cto) == 0) {
	fprintf (stderr, "ERROR! (timeouts cannot set)\n");
	return 0;
    }
    return self;
}

void mesg_sci_delete (mesg_sci_t* self)
{
    CloseHandle(self->handle);
    free (self);
}

/**
 * コマンドを送信
 */
int mesg_sci_send (mesg_sci_t* self)
{
    DWORD	size;
    char enc_buff[1024];
    int enc_size;
    DWORD	count;

    /* パケットのエンコード */
    enc_size = base64_encode (enc_buff, self->send_buff, self->send_size);
    enc_buff[enc_size++] = '\n';
    /* データの送信 */
    WriteFile (self->handle, enc_buff, enc_size, &count, NULL);

    // printf ("enc:%s", enc_buff);
    return 0;
}

/**
 * コマンドを一つ処理する
 */
int mesg_sci_recv (mesg_sci_t* self)
{
    char enc_buff[1024];
    char dec_buff[1024];
    int enc_size;
    int dec_size;
    DWORD	count;
    char menu[8];
    int i;

    for (i=0;;i++) {
	char c;
	do {
	    /* 1行読み込み */
	    ReadFile (self->handle, &c, 1, &count, NULL);
#if 0
	    c = fgetc (stdin);
	    count = 1;
#endif
	} while (count == 0);
	enc_buff[i] = c;
	// fprintf (stderr, "[%d]: %c\n", i, c);
	if (c == '\n') {
	    enc_size = i;
	    break;
	}
    }
    // printf ("size = %d\n", enc_size);
    /* デコード */
    self->recv_size = base64_decode (self->recv_buff, enc_buff, enc_size);

    /* メッセージ名を取得 */
    pack_load (self->recv_buff, "c8", menu);

    /* 受信関数を呼び出し */
    for (i=0; i<mesg_sci_menu_numb; i++) {
	if (strcmp (mesg_sci_menu_vect[i].name, menu) == 0) {
	    return mesg_sci_menu_vect[i].func (self);
	}
    }
    return 0;
}
