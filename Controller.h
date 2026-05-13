#include <string.h>
#include <math.h>
#include <array>
#include "core_cm7.h"
#include "daisy_pod.h"
#include "daisysp.h"
// Prioriteret liste af, hvor let det skal være at ændre
// 1. RT60, size, mix
// 2. LoDecay
// 3. HiDecay
// 4. LoFreq
// 5. HiFreq


class controller
{
public:
    controller(){}; //Std. constructor
    ~controller(){}; // Std. destructor

    
    



private:
    // Variables to be affected by the user interface
    uint8_t state;
    uint8_t size;
    uint8_t RT60;
    uint8_t mix;
    uint16_t HiFreq;
    uint16_t lowFreq;
    uint8_t HiDecay;
    uint8_t LoDecay;



};