#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include"corvus.h"
#define DEFAULT_BUFSIZE  64
int main()
{
    struct context *mycontext;
    mycontext=context_new(DEFAULT_BUFSIZE);
    //context_init(mycontext,DEFAULT_BUFSIZE);
    struct connection *con=context_connection_new(mycontext);
    char str[]="weqfrrgfghdfhdhhhgjhkkdfsdggfagdweqfrrgfghdfhdhhhgjhkkdfsdggfagddfvcxbbxzbarffr";
    char str1[]="12345678899967655554433345667789fsatrsraatryufsagfsgfagfdshsdhshgfsfdsffgfdfdgfs";
    int write_size=strlen(str)+strlen(str1);
    printf("write_size %d\n",write_size);
    con_buf_write(con,str,strlen(str));
    con_buf_write(con,str1,strlen(str1));
    uint8_t buf[80];
    int read=0;
    read=con_buf_read(con,buf,80);
    printf("1 read %d\n",read);
    read=con_buf_read(con,buf,80);
    printf("2 read %d\n",read);
    context_conn_recyle(mycontext,con);
    print_stats(mycontext);
    context_free_content(mycontext);
    context_destroy(mycontext);
    return 0;
}
