#pragma once
#include <stdbool.h>
#include <stdint.h>
bool base_sleep(int milliseconds);
namespace zsy{
class Clock{
public:
    virtual uint32_t Now()const=0;
};
void SetGlobalClock(Clock *clock);
uint32_t GetMilliSeconds();
}