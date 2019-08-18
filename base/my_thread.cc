#include "my_thread.h"
#include <unistd.h>
namespace zsy{
ThreadRef GetCurrentThreadRef(){
    return pthread_self();
}
bool IsThreadRefEqual(const ThreadRef& a, const ThreadRef& b){
    return pthread_equal(a,b);
} 
ThreadManager *ThreadManager::Instance(){
    static ThreadManager  *const thread_manager
    =new ThreadManager();
    return thread_manager;
}
MyThread* ThreadManager::CurrentThread(){
     return static_cast<MyThread*>(pthread_getspecific(key_));
}
void ThreadManager::SetCurrentThread(MyThread *thread){
    pthread_setspecific(key_, thread);    
}
bool ThreadManager::IsMainThread(){
    return IsThreadRefEqual(GetCurrentThreadRef(),main_thread_ref_);
}
ThreadManager::ThreadManager():main_thread_ref_(GetCurrentThreadRef()){
     pthread_key_create(&key_, nullptr);
}
void *thread_fun(void *param){
    MyThread *thread=static_cast<MyThread*>(param);
    ThreadManager::Instance()->SetCurrentThread(thread);
    thread->Run();
}
//seems not safe
MyThread *MyThread::Current(){
    ThreadManager *manager=ThreadManager::Instance();
    MyThread *thread=manager->CurrentThread();
    /*if((!thread)&&manager->IsMainThread()){
        thread=new MyThread();
    }*/
    return thread;
}
bool MyThread::IsCurrent() const{
    return this==ThreadManager::Instance()->CurrentThread();
}
bool MyThread::Start(){
    int ret=0;
    ret=pthread_create(&pid_,nullptr,thread_fun,(void*)this);
    if(ret){
        return false;
    }
    return true;

}
void MyThread::Stop(){
	running_=false;
    pthread_join(pid_,nullptr);
}
void MyThread::Sleep(int ms){
    uint64_t us=ms*1000;
    usleep(us);
}
}
