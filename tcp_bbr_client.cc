/*
https://hug.app/t/topic/14
echo "net.core.default_qdisc=fq" >> /etc/sysctl.conf
echo "net.ipv4.tcp_congestion_control=bbr" >> /etc/sysctl.conf
sysctl -p
reboot
sysctl net.ipv4.tcp_available_congestion_control
 */

#include <stdint.h>
#include <iostream>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>  //struct tcp_info
#include <signal.h>

#include <string>
#include <fstream>
#include "anet.h"
#include "my_thread.h"
#include "base_time.h"
class TcpClient:public zsy::MyThread{
public:
	TcpClient(int fd):fd_(fd){}
	~TcpClient(){
		if(f_out_.is_open()){
			f_out_.close();
		}
	}
	void SetLogName(std::string &s){
		f_out_.open(s.c_str(), std::fstream::out);
	}
	void StartThread(){
		running_=true;
		start_time_=zsy::GetMilliSeconds();
		Start();
	}
	void StopThread(){
		running_=false;
		Stop();
	}
	void Run() override{
		while(running_){
			int32_t now=zsy::GetMilliSeconds();
			if(now>last_record_){
				GetTcpInfo(now);
				last_record_=now+interval_;
			}
		}
	}
//https://github.com/torvalds/linux/blob/master/include/uapi/linux/tcp.h
	void GetTcpInfo(int32_t now){
		if(!f_out_.is_open()){
			return;
		}
		char line [512];
		memset(line,0,512);
		int abs_time=now-start_time_;
	    struct tcp_info info;
	    int length=sizeof(struct tcp_info);
	    if(getsockopt(fd_,IPPROTO_TCP,TCP_INFO,(void*)&info,(socklen_t*)&length)==0){
	        //printf("cwnd %u ss %u\n",info.tcpi_snd_cwnd,info.tcpi_snd_ssthresh);
	    	 sprintf(line, "%u %u %u",abs_time,info.tcpi_rtt,
	    			 info.tcpi_snd_cwnd/*,info.tcpi_delivery_rate*/);
		std::cout<<info.tcpi_snd_cwnd<<std::endl;
	    	 f_out_<<line<<std::endl;
	    	 //interval_=info.tcpi_rtt/1000;
	    }
	}
private:
	bool running_{true};
	int fd_;
	int last_record_{0};
	int start_time_{0};
	int interval_{100};
	std::fstream f_out_;
};
char g_err_string[1024]={0};
static const size_t TCP_CC_NAME_MAX = 16;
int set_congestion_type(int fd,char *cc){
    char optval[TCP_CC_NAME_MAX];
    memset(optval,0,TCP_CC_NAME_MAX);
    strncpy(optval,cc,TCP_CC_NAME_MAX);
    int length=strlen(optval)+1;
    int rc=setsockopt(fd,IPPROTO_TCP, TCP_CONGESTION, (void*)optval,length);
    return rc;
}
int run_duration=300;// in seconds
bool m_running=true;
void signal_exit_handler(int sig)
{
	m_running=false;
}
int main(){
	signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGTSTP, signal_exit_handler);
	char*serv_ip="10.0.4.2";
	uint16_t serv_port=1234;
    int fd=0;
    int ret=0;
    char cc_type[]="bbr";
    int send_times=0;
    char send_data[1400]={0};
    fd=anetCreateSocket(g_err_string,AF_INET);
    if(fd<0){
        std::cout<<"fd create failed"<<std::endl;
        return 0;
    }
    ret=set_congestion_type(fd,cc_type);
    if(ret!=0){
    	std::cout<<"cc type is not support"<<std::endl;
    	close(fd);
    	return 0;
    }
    ret=anetTcpConnectWithFd(g_err_string,fd,serv_ip,serv_port,ANET_CONNECT_NONE);
    if(ret<0){
    	std::cout<<"connect error"<<std::endl;
    }
    /*ret=anetNonBlock(g_err_string,fd);
    if(ret<0){
    	std::cout<<"set non block failed"<<std::endl;
    	close(fd);
    	return 0;
    }*/
    int now=zsy::GetMilliSeconds();
    int stop=now+run_duration*1000;
    TcpClient client(fd);
    std::string name("trace.txt");
    client.SetLogName(name);
    client.StartThread();
    while(m_running){
    	now=zsy::GetMilliSeconds();
    	anetWrite(fd,send_data,sizeof(send_data));
    	if(now>stop){
    		break;
    	}
    }
    client.StopThread();
    close(fd);
    return 0;
}
