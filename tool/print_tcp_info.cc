#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <netinet/tcp.h>  //struct tcp_info
static const size_t TCP_CC_NAME_MAX = 16;
void error_handling(char *message);
void print_cc_type(int fd){
    char optval[TCP_CC_NAME_MAX];
    memset(optval,0,TCP_CC_NAME_MAX);
    int length=sizeof(optval);
    getsockopt(fd,IPPROTO_TCP, TCP_CONGESTION, (void*)optval,(socklen_t*)&length);
    printf("cctype %s\n",optval);
}
void print_tcp_info(int fd){
    struct tcp_info info;
    int length=sizeof(struct tcp_info);
     if(getsockopt(fd,IPPROTO_TCP,TCP_INFO,(void*)&info,(socklen_t*)&length)==0){
        printf("cwnd %u ss %u\n",info.tcpi_snd_cwnd,info.tcpi_snd_ssthresh);
     }
}
int set_congestion_type(int fd,char *cc){
    char optval[TCP_CC_NAME_MAX];
    memset(optval,0,TCP_CC_NAME_MAX);
    strncpy(optval,cc,TCP_CC_NAME_MAX);
    int length=strlen(optval)+1;
    int rc=setsockopt(fd,IPPROTO_TCP, TCP_CONGESTION, (void*)optval,length);
    if(rc!=0){
        printf("cc is not supprt\n");
    }
    return rc;
}
int main(int argc, char* argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char message[30];
    int str_len=0;
    int idx=0, read_len=0;
    const char *server_ip="127.0.0.1";
    uint16_t server_port=1234;
    char cc_type[]="cubic";
    
    sock=socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=inet_addr(server_ip);
    serv_addr.sin_port=htons(server_port);
    
    print_tcp_info(sock);
    set_congestion_type(sock,cc_type);
    print_cc_type(sock);
    /*if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) 
        error_handling("connect() error!");

    while(read_len=read(sock, &message[idx++], 1))
    {
        if(read_len==-1)
            error_handling("read() error!");

        str_len+=read_len;
    }

    printf("Message from server: %s \n", message);
    printf("Function read call count: %d \n", str_len);*/
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}