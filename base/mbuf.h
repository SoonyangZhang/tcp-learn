#ifndef MBUF_H
#define MBUF_H
#include "queue.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STAILQ_LAST
#define STAILQ_LAST(head, type, field)                       \
    (STAILQ_EMPTY((head))                                    \
     ?  NULL                                                 \
     : ((struct type *)(void *) ((char *)((head)->stqh_last) \
             - (size_t)(&((struct type *)0)->field))))
#endif

struct context;
TAILQ_HEAD(mhdr, mbuf);
struct mbuf {
    TAILQ_ENTRY(mbuf) next;
    uint8_t *pos;//read marker
    uint8_t *last;//write marker
    uint8_t *start;//start of buf
    uint8_t *end; //end of buf
    struct mhdr *queue; // the queue contain the buf
};

struct mbuf *mbuf_get(struct context *);
void mbuf_recycle(struct context *, struct mbuf *);
int mbuf_read_size(struct mbuf *);
int mbuf_write_size(struct mbuf *);
int mbuf_write(struct mbuf * buf,void *data,int len);
int mbuf_read(struct mbuf * buf,void*dst,int len);
int mbuf_peek(struct mbuf * buf,void*dst,int len);
bool mbuf_can_recycle(struct mbuf * buf);
bool mhdr_readable(struct mhdr *header);
#ifdef __cplusplus
}
#endif
#endif
