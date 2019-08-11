#include <memory.h>
#include "corvus.h"
#include "logging.h"
#include "alloc.h"
#define MIN_BUFSIZE   32
#ifndef TAILQ_RESET
#define TAILQ_RESET(var, field)           \
do {                                      \
    (var)->field.tqe_next = NULL;         \
    (var)->field.tqe_prev = NULL;         \
} while (0)
#endif

struct con_info * con_info_new(struct context *ctx);


void context_init(struct context*ctx,int bufsize)
{
    ctx->mbuf_offset=bufsize-sizeof(struct mbuf);
    ctx->mstats.free_buffers = 0;
    TAILQ_INIT(&ctx->free_mbufq);
    STAILQ_INIT(&ctx->free_con_infoq);

}
void *context_new(int bufsize){
    struct context *ctx=cv_malloc(sizeof(*ctx));
    memset(ctx,0,sizeof(struct context));
    context_init(ctx,bufsize);
    return ctx;
}
void context_destroy(struct context *ctx)
{
    cv_free(ctx);
}
int  context_free_content(struct context *ctx)
{
    if(ctx->mstats.conns)
    {
        LOG(ERROR,"connections are not totally free");
        return 1;
    }
    mbuf_destroy(ctx);

    struct buf_time  *bt;
    struct con_info *info;
    while(!STAILQ_EMPTY(&ctx->free_con_infoq))
    {
        info=STAILQ_FIRST(&ctx->free_con_infoq);
        STAILQ_REMOVE_HEAD(&ctx->free_con_infoq,next);
        cv_free(info);
    }
    return 0;
}
void print_stats(struct context *ctx)
{
    LOG(DEBUG,"total %lld connections in use\n",ctx->mstats.conns);
    LOG(DEBUG,"total  %lld free connections\n",ctx->mstats.free_conns);
    LOG(DEBUG,"total %lld mbufs in use\n",ctx->mstats.buffers);
    LOG(DEBUG,"total %lld free mbufs\n",ctx->mstats.free_buffers);
    LOG(DEBUG,"total %lld info in use\n",ctx->mstats.conn_info);
    LOG(DEBUG,"total %lld free info\n",ctx->mstats.free_conn_info);
}
struct con_info * con_info_new(struct context *ctx)
{
    struct con_info *info=NULL;

    if(!STAILQ_EMPTY(&ctx->free_con_infoq))
    {

        info=STAILQ_FIRST(&ctx->free_con_infoq);
        STAILQ_REMOVE_HEAD(&ctx->free_con_infoq,next);
        ctx->mstats.free_conn_info--;
    }else{
    info=(struct con_info*)cv_malloc(sizeof(struct con_info));
    }
    TAILQ_INIT(&info->data);
    info->ctx=ctx;
    info->current_mbuf=NULL;
    ctx->mstats.conn_info++;
    return info;

}
void conn_init(struct connection *con,struct context *ctx)
{
    memset(con,0,sizeof(struct connection));
    con->ctx=ctx;
}
void connection_buf_free(struct connection *con)
{
    struct mbuf *buf;
    struct con_info *info=con->info;
    if(info==NULL) return;
    while(!TAILQ_EMPTY(&info->data))
    {
        buf=TAILQ_FIRST(&info->data);
        TAILQ_REMOVE(&info->data,buf,next);
        mbuf_recycle(con->ctx,buf);
    }
}
struct connection* context_connection_new(struct context *ctx)
{
    struct connection *con=NULL;
    if(TAILQ_FIRST(&ctx->conns)!=NULL)
    {
       LOG(DEBUG,"connection get cache");
       TAILQ_REMOVE(&ctx->conns,con,next);
       ctx->mstats.free_conns--;
    }else{
        con=(struct connection*)cv_calloc(1,sizeof(struct connection));
    }
    conn_init(con,ctx);
    ctx->mstats.conns++;
    return con;
}
void context_conn_recyle(struct context*ctx,struct connection *con)
{
    if(con->info!=NULL)
    {
        ctx->mstats.conn_info--;
        connection_buf_free(con);
        struct con_info *info=con->info;
        STAILQ_INSERT_TAIL(&ctx->free_con_infoq,info,next);
        ctx->mstats.free_conn_info++;
    }
    ctx->mstats.conns--;
    if(con->next.tqe_next!=NULL||con->next.tqe_prev!=NULL)
    {
        TAILQ_REMOVE(&ctx->conns,con,next);
        TAILQ_RESET(con,next);
    }
    TAILQ_INSERT_HEAD(&ctx->conns,con,next);
    con->info=NULL;
    ctx->mstats.free_conns++;
}
void con_buf_write(struct connection *con,void*data,int len)
{

    if(len==0) return;
    struct mbuf *buf;
    int num_to_write=len;
    void *data_ptr=data;
    struct con_info *info=con->info;
    if(info==NULL)
        info=con_info_new(con->ctx);

    con->info=info;
    buf=info->current_mbuf;
    if(buf==NULL||(buf->end-buf->last)==0)
    {
        buf=mbuf_get(con->ctx);
        buf->queue=&info->data;
        TAILQ_INSERT_TAIL(&info->data,buf,next);
        info->current_mbuf=buf;
    }
    buf=info->current_mbuf;
  int write=0;
  while(num_to_write!=0)
  {
       int writable=mbuf_write_size(buf);//buf->end-buf->last;
       if(writable>num_to_write)
            {
                write=mbuf_write(buf,data_ptr,num_to_write);
                num_to_write-=write;

            }
        else{
                write=mbuf_write(buf,data_ptr,writable);
                 num_to_write-=write;
                if(num_to_write==0)
                    {return;}
                else{
                    buf=mbuf_get(con->ctx);
                    buf->queue=&info->data;
                    TAILQ_INSERT_TAIL(&info->data,buf,next);
                    info->current_mbuf=buf;
                }

        }
        data_ptr+=write;

  }
  con->data_size+=len;
}
int  con_buf_read(struct connection *con,void *dst,int len)
{
    int i=0;
    int read=0;
    struct con_info *info=con->info;
    while(mhdr_readable(&info->data))
    {
    int ret=0;
    struct mbuf *buf=TAILQ_FIRST(&info->data);
    ret=mbuf_read(buf,dst,len);
    len-=ret;
    read+=ret;
    dst+=ret;
    if(mbuf_read_size(buf)){
        break;
    }else{
        TAILQ_REMOVE(&info->data,buf,next);
        mbuf_recycle(con->ctx,buf);
    }
    }
    con->data_size-=read;
    return read;
}
int con_buf_peek(struct connection *con,void *dst,int len){
    struct mbuf *mbuf=NULL;
    struct con_info *info=con->info;
    int ret=0;
    if(info){
        TAILQ_FOREACH(mbuf,&info->data,next){
            int tmp=mbuf_read_size(mbuf);
            int read=tmp>len?len:tmp;
            mbuf_peek(mbuf,dst,read);
            ret+=read;
            dst+=read;
            len-=read;
            if(len==0){
                break;
            }
        }
    }
    return ret;
}
