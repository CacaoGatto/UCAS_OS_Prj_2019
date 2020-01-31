#include "string.h"
#include "mailbox.h"

#define MAX_NUM_BOX 32

static mailbox_t mboxs[MAX_NUM_BOX];

void mbox_init()
{
    int i;
    for (i = 0; i < MAX_NUM_BOX; i++) {
        mboxs[i].busy = 0;
    }
}

mailbox_t *mbox_open(char *name)
{
    int i;
    for (i = 0; i < MAX_NUM_BOX; i++)
        if (mboxs[i].busy && (strcmp(mboxs[i].name, name) == 0))
            return &mboxs[i];
    for (i = 0; i < MAX_NUM_BOX; i++) {
        if (!mboxs[i].busy) {
            strcpy(mboxs[i].name, name);
            mboxs[i].contents = 0;
            mboxs[i].busy = 1;
            mboxs[i].queue.head = NULL;
            mboxs[i].queue.tail = NULL;
            condition_init(&mboxs[i].empty);
            condition_init(&mboxs[i].full);
            mutex_lock_init(&mboxs[i].lock);
            int j;
            for (j = 0; j < MAILBOX_SIZE; j++) {
                mboxs[i].mail[j].avail = 1;
                mboxs[i].mail[j].prev = NULL;
                mboxs[i].mail[j].next = NULL;
            }
            return &mboxs[i];
        }
    }
    return NULL;
}

void mbox_close(mailbox_t *mailbox)
{
    mailbox->busy = 0;
}

void mbox_send(mailbox_t *mailbox, void *msg, int msg_length)
{
    mutex_lock_acquire(&mailbox->lock);
    while (mailbox->contents == MAILBOX_SIZE)
        condition_wait(&mailbox->lock, &mailbox->full);
    
    if (msg_length < MAIL_SIZE) {
        int i;
        for (i = 0; i < MAILBOX_SIZE; i++) {
            if (mailbox->mail[i].avail) {
                strcpy(mailbox->mail[i].content, msg);
                mailbox->mail[i].avail = 0;
                queue_push_b(&mailbox->queue, &mailbox->mail[i]);
                break;
            }
        }
        mailbox->contents++;
    }
    
    condition_broadcast(&mailbox->empty);
    mutex_lock_release(&mailbox->lock);
}

void mbox_recv(mailbox_t *mailbox, void *msg, int msg_length)
{
    mutex_lock_acquire(&mailbox->lock);
    while (mailbox->contents == 0)
        condition_wait(&mailbox->lock, &mailbox->empty);
    
    if (msg_length < MAIL_SIZE) {
        buffer_t* temp = (buffer_t*)queue_dequeue_b(&mailbox->queue);
        strcpy(msg, temp->content);
        temp->avail = 1;
        mailbox->contents--;
    }
    
    condition_broadcast(&mailbox->full);
    mutex_lock_release(&mailbox->lock);
}
