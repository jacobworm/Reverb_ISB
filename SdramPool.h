// SdramPool.h
#pragma once
//#include "sdram.h" // giver DSY_SDRAM_BSS


constexpr int SAMPLE_RATE = 48000;
constexpr int MAX_DELAY_MS  = 480;
constexpr int MAX_SAMPLES   = (SAMPLE_RATE * MAX_DELAY_MS) / 1000 + 2; // 23042
constexpr int TOTAL_DELAYLINES = 58;
constexpr size_t SDRAM_POOL_BYTES = (size_t)TOTAL_DELAYLINES * MAX_SAMPLES * sizeof(float) + 256*1024; // + lidt extra

DSY_SDRAM_BSS float delay_pool[TOTAL_DELAYLINES][MAX_SAMPLES]; // uinitialiseret i SDRAM BSS
inline int buffer_counter = 0;
inline float* allocateDelaybuffer(){
    if (buffer_counter >= TOTAL_DELAYLINES){
        return nullptr;
    }
    return delay_pool[buffer_counter++];
}
inline void resetDelayBufferCounter() { buffer_counter = 0; }