#include <iostream>
#include <signal.h>
#include <stdint.h>
#include "cf_platform.h"
#include <pthread.h>
#include <sys/epoll.h>
#include <map>
#include <memory>
#include "corvus.h"
bool m_running=true;
void signal_exit_handler(int sig)
{
	m_running=false;
}
const char *serv_ip="127.0.0.1";
uint16_t port=1234;
//refer from https://blog.csdn.net/hnlyyk/article/details/48974749
class ImageDelegate{
public:
    virtual void DataAvailable(uint32_t)=0;
    virtual ~ImageDelegate(){}
};
const int mbuf_size=1024;
class TcpClient{
public:
    TcpClient(su_socket fd,struct context *context):TcpClient(fd,context,nullptr){}
    TcpClient(su_socket fd, struct context *context,ImageDelegate *d):fd_(fd),context_(context),delegate_(d){
        
        con_=context_connection_new(context_);
    }
    ~TcpClient(){
        if(context_){
            if(con_){
                context_conn_recyle(context_,con_);
            }
        }
        
    }
    void OnNewData(uint8_t *data,uint32_t len){
        con_buf_write(con_,data,len);
        int data_size=con_->data_size;
        if(data_size>4){
            uint32_t payload_len=0;
            con_buf_peek(con_,(void*)&payload_len,sizeof(uint32_t));
            payload_len=ntohl(payload_len);
            uint32_t available=payload_len+sizeof(uint32_t);
            std::cout<<" len "<<payload_len<<std::endl;
            if(data_size>=available){
                NotifyDataAvailable(available);
            }
        }
    }
    void Close(){
        
    }
    void NotifyDataAvailable(uint32_t size){
        std::cout<<" available "<<size<<std::endl;
        if(delegate_){
            delegate_->DataAvailable(size);
        }
    }
private:
    su_socket fd_;
    struct context *context_{nullptr};
    ImageDelegate *delegate_{nullptr};
    struct connection *con_{nullptr};
};
class TcpServer{
public:
    TcpServer(const char*ip,uint16_t port){
        int ret=0;
        ret=su_tcp_listen_create(ip,port,&listenfd_);
        if(ret<0){
            std::cout<<"tcp server fd failed"<<std::endl;
            abort();
        }
        context_=(struct context*)context_new(mbuf_size);
        su_socket_noblocking(listenfd_);
        epfd_ = epoll_create(256);
        struct epoll_event ev;
        ev.data.fd = listenfd_;
        ev.events = EPOLLIN | EPOLLET;
        epoll_ctl(epfd_, EPOLL_CTL_ADD, listenfd_, &ev);
        
    }
    ~TcpServer(){
        while(!clients_.empty()){
            TcpClient *client=nullptr;
            auto it=clients_.begin();
            client=it->second;
            clients_.erase(it);
            delete client;
        }
        su_socket_destroy(listenfd_);
        if(context_){
            context_free_content(context_);
            context_destroy(context_);            
        }
    }
    void Stop(){
        running_=false;
    }
    void Process(){
        while(running_){
            int nfd=0;
            int i=0;
            struct epoll_event events[20];
            nfd=epoll_wait(epfd_, events, 20, 500);
            for(i=0;i<nfd;i++){
                if(events[i].data.fd ==listenfd_){
                    su_addr addr;
                    int addr_len=sizeof(addr);
                    su_socket con_fd=su_accept(listenfd_,&addr,&addr_len);
                    std::cout<<"new con"<<std::endl;
                    su_socket_noblocking(con_fd);
                    TcpClient *client=new TcpClient(con_fd,context_);
                    clients_.insert(std::make_pair(con_fd,client));
                    struct epoll_event ev;
                    ev.data.fd=con_fd;
                    ev.events=EPOLLIN|EPOLLRDHUP|EPOLLET;      //EPOLLLT EPOLLET that's matters 
                    //https://blog.csdn.net/qcghdy/article/details/22791077
                    //https://github.com/yedf/handy/blob/master/raw-examples/epoll-et.cc
                    epoll_ctl(epfd_, EPOLL_CTL_ADD, con_fd, &ev);
                }else{
                    su_socket con_fd=events[i].data.fd;
                    if(events[i].events&EPOLLIN){
                        int n=0;
                        uint8_t buf[1500];
                        n=su_tcp_recv(con_fd,buf,1500);
                        if(n<=0){
                            std::cout<<"con close in epoll in"<<std::endl;
                            su_socket_destroy(con_fd);
                            struct epoll_event ev;
                            epoll_ctl(epfd_, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                        }
                        auto it=clients_.find(con_fd);
                        if(n>0){
                        std::cout<<"in "<<n<<std::endl;
                            if(it!=clients_.end()){
                                it->second->OnNewData(buf,n);
                            }
                        }
			//data may not read all so, continue read, in ET mode
			for(;;){
				n=su_tcp_recv(con_fd,buf,1500);	
				if(n>0){
				std::cout<<"in "<<n<<std::endl;
                            	if(it!=clients_.end()){
                                	it->second->OnNewData(buf,n);
                            	}
				  std::cout<<" total "<<n<<std::endl;
				}else{
					std::cout<<" break "<<std::endl;
					break;
				}
			}
    }
            if(events[i].events&EPOLLRDHUP){
            std::cout<<"con close in hup"<<std::endl;
            su_socket_destroy(con_fd);
            struct epoll_event ev;
            epoll_ctl(epfd_, EPOLL_CTL_DEL, events[i].data.fd, &ev);                
            }
        }
            }
        }
    }
private:
    bool running_{true};
    struct context *context_{nullptr};
    su_socket listenfd_;
    int epfd_;
    std::map<int,TcpClient*> clients_;
};
void *another_thread(void *param){
    TcpServer *server=static_cast<TcpServer*>(param);
    server->Process();
}
int main()
{
	signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGTSTP, signal_exit_handler);
    TcpServer server(serv_ip,port);
    int ret=0;
    pthread_t pid;
    ret=pthread_create(&pid,nullptr,another_thread,&server);
    if(ret){
        std::cout<<"thread failed"<<std::endl;
    }
    while(m_running){
        
    }
    server.Stop();
    pthread_join(pid,nullptr);
}

