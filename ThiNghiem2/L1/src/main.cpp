#ifndef B1_USE_INTERRUPT
#define B1_USE_INTERRUPT 0
#endif

// Bài 1 giữ hai bản code riêng, còn PlatformIO chọn một bản theo environment.
#if B1_USE_INTERRUPT
#include "../Interrupt/src/main.cpp"
#else
#include "../Polling/src/main.cpp"
#endif
