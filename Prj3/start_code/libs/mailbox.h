#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#define MAILBOX_SIZE 16
#define MAIL_SIZE 64

#include "sched.h"

typedef struct buffer
{
    char content[MAIL_SIZE];
    int avail;
    void *next;
    void *prev;
} buffer_t;

typedef struct mailbox
{
    char name[32];
    int contents;
    int busy;
    queue_t queue;
    buffer_t mail[MAILBOX_SIZE];
    
    mutex_lock_t lock;
    condition_t empty;
    condition_t full;
} mailbox_t;


void mbox_init();
mailbox_t *mbox_open(char *);
void mbox_close(mailbox_t *);
void mbox_send(mailbox_t *, void *, int);
void mbox_recv(mailbox_t *, void *, int);

#endif