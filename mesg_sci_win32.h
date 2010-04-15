#ifndef __MESG_SCI_WIN32__
#define __MESG_SCI_WIN32__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <windows.h>
#include "base64.h"
#include "pack.h"

typedef struct {
  
  HANDLE handle;

  char send_buff[1024];
  char recv_buff[1024];
  int send_size;
  int recv_size;

} mesg_sci_t;

mesg_sci_t* mesg_sci_create (char* port, int baud);
void mesg_sci_delete (mesg_sci_t* self);

int mesg_sci_send (mesg_sci_t* self);
int mesg_sci_recv (mesg_sci_t* self);

  typedef int (*mesg_sci_recv_func_t)(mesg_sci_t* self);

  typedef struct {
    char name[8];

    int (*func)(mesg_sci_t* self);

  } mesg_sci_menu_t;

  extern int mesg_sci_menu_numb;
  extern mesg_sci_menu_t mesg_sci_menu_vect[];

#ifdef __cplusplus
}
#endif

#endif /* __MESG_SCI_WIN32__ */
