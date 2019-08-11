#include <stdlib.h>
#include <string.h>
#include "alloc.h"
#include"mbuf.h"
#include"corvus.h"
#define RECYCLE_LENGTH 10
#define BUF_TIME_LIMIT 512
static struct mbuf *mbuf_create(struct context *ctx)
{
    struct mbuf *mbuf;
    uint8_t *buf;
    if (!TAILQ_EMPTY(&ctx->free_mbufq)) {
        mbuf = TAILQ_FIRST(&ctx->free_mbufq);
        TAILQ_REMOVE(&ctx->free_mbufq, mbuf, next);
        ctx->mstats.free_buffers--;
    } else {
        int bufsize=sizeof(struct mbuf)+ctx->mbuf_offset;
        buf = (uint8_t*)cv_malloc(bufsize);
        if (buf == NULL) {
            return NULL;
        }
        mbuf = (struct mbuf *)(buf + ctx->mbuf_offset);
    }
    return mbuf;
}

void mbuf_free(struct context *ctx, struct mbuf *mbuf)
{
    uint8_t *buf;
    buf = (uint8_t *)mbuf - ctx->mbuf_offset;
    cv_free(buf);
}
struct mbuf *mbuf_get(struct context *ctx)
{
    struct mbuf *mbuf;
    uint8_t *buf;

    mbuf = mbuf_create(ctx);
    if (mbuf == NULL) {
        return NULL;
    }

    buf = (uint8_t *)mbuf - ctx->mbuf_offset;
    mbuf->start = buf;
    mbuf->end = buf + ctx->mbuf_offset;

    mbuf->pos = mbuf->start;
    mbuf->last = mbuf->start;
    mbuf->queue = NULL;
    TAILQ_NEXT(mbuf, next) = NULL;
    ctx->mstats.buffers++;
    return mbuf;
}

void mbuf_recycle(struct context *ctx, struct mbuf *mbuf)
{
    ctx->mstats.buffers--;

    if (ctx->mstats.free_buffers > RECYCLE_LENGTH) {
        mbuf_free(ctx, mbuf);
        return;
    }

    TAILQ_NEXT(mbuf, next) = NULL;
    TAILQ_INSERT_HEAD(&ctx->free_mbufq, mbuf, next);

    ctx->mstats.free_buffers++;
}

int mbuf_read_size(struct mbuf *mbuf)
{
    return (int)(mbuf->last - mbuf->pos);
}

int mbuf_write_size(struct mbuf *mbuf)
{
    return (int)(mbuf->end - mbuf->last);
}
int mbuf_write(struct mbuf * mbuf,void *data,int len){
    if(mbuf->last==mbuf->end) return 0;
    int remain=mbuf_write_size(mbuf);
    int write=remain>len?len:remain;
    memcpy(mbuf->last,data,write);
    mbuf->last+=write;
    return write;
}
int mbuf_read(struct mbuf * mbuf,void*dst,int len){
    int read=mbuf_peek(mbuf,dst,len);
    mbuf->pos+=read;
    return read;
}
int mbuf_peek(struct mbuf * mbuf,void*dst,int len){
    int ret=0;
    int read=mbuf_read_size(mbuf);
    if(read){
        int copy=read>len?len:read;
        ret=copy;
        memcpy(dst,mbuf->pos,copy);
    }
    return ret;
}
void mbuf_destroy(struct context *ctx)
{
    struct mbuf *buf;
    while (!TAILQ_EMPTY(&ctx->free_mbufq)) {
        buf = TAILQ_FIRST(&ctx->free_mbufq);
        TAILQ_REMOVE(&ctx->free_mbufq, buf, next);
        mbuf_free(ctx, buf);
        ctx->mstats.free_buffers--;
    }
}
bool mhdr_readable(struct mhdr *header){
    bool ret=false;
    if(!TAILQ_EMPTY(header)){
        struct mbuf *buf=TAILQ_FIRST(header);
        if(mbuf_read_size(buf)){
            ret=true;
        }
    }
    return ret;
}
