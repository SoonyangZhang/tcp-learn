#include <unistd.h>
#include <iostream>
#include "base_time.h"
using namespace zsy;
using namespace std;
int main(){
	uint32_t last=GetMilliSeconds();
	sleep(1);
	uint32_t now=GetMilliSeconds();
	uint32_t delta=now-last;
	std::cout<<"elapse "<<delta<<std::endl;
	return 0;
}
