#include "cf_platform.h"
#include <stdint.h>
#include <iostream>
char*serv_ip="127.0.0.1";
uint16_t serv_port=1234;
int main(){
    uint8_t send_buf[3000];
    su_socket fd;
    int ret=0;
    ret=su_tcp_create(&fd);
    if(ret<0){
        std::cout<<"ctreat failed"<<std::endl;
        return 0;
    }
    su_addr serv_addr;
    int addr_len=sizeof(serv_addr);
    su_set_addr(&serv_addr,serv_ip,serv_port);
    if(su_connect(fd,&serv_addr,addr_len)<0){
	su_socket_destroy(fd);
        std::cout<<"con failed"<<std::endl;
        return 0;
    }
    //
    uint32_t payload_len=2500;
    uint32_t total_len=sizeof(uint32_t)+payload_len;
    uint8_t *ptr=send_buf;
    uint32_t header=htonl(payload_len);
    memcpy(ptr,(void*)&header,sizeof(uint32_t));
    su_tcp_send(fd,send_buf,total_len);
    su_socket_destroy(fd);
    return 0;
}
