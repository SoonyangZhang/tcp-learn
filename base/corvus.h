#ifndef __CORVUS_H
#define __CORVUS_H
//copy from https://github.com/eleme/corvus
#include"mbuf.h"
#include"queue.h"
#ifdef __cplusplus
extern "C" {
#endif
struct memory_stats {
    int64_t total_buffers;
    int64_t buffers;
    int64_t free_buffers;
    int64_t conns;
    int64_t conn_info;
    
    int64_t free_conns;
    int64_t free_conn_info;
};
struct con_info
{
    struct mbuf *current_mbuf;
    struct mhdr data;
    STAILQ_ENTRY(con_info)  next;
    struct context*ctx;
};
struct connection
{
    int data_size;
    struct con_info *info;
    TAILQ_ENTRY(connection) next;
    struct context * ctx;
};


TAILQ_HEAD(conn_tqh,connection);
STAILQ_HEAD(con_info_tqh,con_info);
struct context
{
    size_t mbuf_offset;
    struct conn_tqh conns;
    struct mhdr free_mbufq;
    struct con_info_tqh  free_con_infoq;
    struct memory_stats mstats;

};
void *context_new(int bufsize);
void context_destroy(struct context *ctx);
int  context_free_content(struct context *ctx);


struct connection* context_connection_new(struct context *ctx);
void context_conn_recyle(struct context*ctx,struct connection *con);

void con_buf_write(struct connection *con,void*data,int len);
int  con_buf_read(struct connection *con,void *dst,int len);
int con_buf_peek(struct connection *con,void *dst,int len);
void connection_buf_free(struct connection *con);

void print_stats(struct context *ctx);
#ifdef __cplusplus
}
#endif

#endif
