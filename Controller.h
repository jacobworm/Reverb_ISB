#include <string.h>
#include <math.h>
#include <array>
#include "core_cm7.h"
#include "daisy_pod.h"
#include "daisysp.h"
#include "ReverbEngine.h"
// Prioriteret liste af, hvor let det skal være at ændre
// 1. RT60, size, mix
// 2. LoDecay
// 3. HiDecay
// 4. LoFreq
// 5. HiFreq

using namespace daisy;
#define NUM_STATES 3 // Max number of states


class controller
{
public:
// Constructor
controller(DaisyPod *hwPod, ReverbEngine<float>* revEng)
: hwPod(hwPod), revEng(revEng){}

~controller(){}; // Std. destructor

void update()
    {

        // Encoder button toggles armed_ status
        if(hwPod->encoder.RisingEdge())
        {
            armed_ = !armed_;
        }
        
        // Control LED2 based off armed_ status
        if(armed_)
        {
            // Set LED2
            hwPod->led2.Set(1.0f, 1.0f, 1.0f);
        }
        else if(!armed_)
        {
            // Turn off LED2
            hwPod->led2.Set(0.0f, 0.0f, 0.0f);
        }


        // SW1 and SW2 changes state
        if(hwPod->button1.RisingEdge())
        {
            state_ += 1;
            if(state_ > NUM_STATES) state_ = 1; // If state is larger than NUM_STATES, wrap around
        }
        else if(hwPod->button2.RisingEdge())
        {
            state_ -= 1;
            if(state_ < 1) state_ = NUM_STATES; // If state is less than 1, wrap around to NUM_STATES
        }

        // Evaluate state
        switch (state_)
        {
        case 1: 
            // State 1 -> Set LED1 to red
            hwPod->led1.Set(1.0f, 0.0f, 0.0f);

            // Update values if armed
            if(armed_)
            {
                int8_t inc = hwPod->encoder.Increment();
                
                // Switch 1 and 2 in- and decrements and sets size
                if(inc > 0) // Increment size
                {
                    size_ += 2;
                    if(size_ > 100) size_ = 100;
                    else if(size_ < 0) size_ = 0;
                }
                else if(inc < 0) // Decrement size
                {
                    size_ -= 2;
                    if(size_ > 100) size_ = 100;
                    else if(size_ < 0) size_ = 0;
                }
                
                revEng->setSize(size_); // Set size

                // Pot1 changes RT60 between 100 and 10000
                RT60_ = 100 + (hwPod->knob1.Value() * (9900));
                revEng->setRT60(RT60_);

                // Pot2 changes mix between 0 and 100
                mix_ = hwPod->knob2.Value() * 100;
                revEng->setMix(mix_);
            }
            break;
        
        case 2:
            // State 2 -> Set LED1 to green
            hwPod->led1.Set(0.0f, 1.0f, 0.0f);

            if(armed_)
            {
                // Pot1 changes HiFreq between 500 and 10000
                HiFreq_ = 500 + (hwPod->knob1.Value() * (9500));
                revEng->setHiFreq(HiFreq_);

                // Pot2 changes LoFreq between 20 and 1000
                LoFreq_ = 20 + (hwPod->knob2.Value() * 980);
                revEng->setLoFreq(LoFreq_);
            }

            break;
        case 3:
            // State 3 -> Set LED1 to blue
            hwPod->led1.Set(0.0f, 0.0f, 1.0f);

            // Update values if armed
            if(armed_)
            {
                // Pot1 changes HiDecay between 0.1 and 
                HiDecay_ = (0.1 + (hwPod->knob1.Value() * 9.9));
                revEng->setHiDecay(HiDecay_);

                // Pot2 changes LoDecay between 0.1 and 10
                LoDecay_ = 0.1 + (hwPod->knob2.Value() * 9.9);
                revEng->setLoDecay(LoDecay_);
            }

            break;
        default:
            // If state is not 1, 2 or 3 -> Set state to 1
            state_ = 1;
            break;
        }

    };

// Getters
const bool getArmed(){ return armed_;};
const uint8_t getState(){ return state_;};
const uint8_t getSize(){ return size_;};
const float getHiFreq(){return HiFreq_;};
const float getLoFreq(){ return LoFreq_;};
const float getHiDecay(){ return HiDecay_;};
const float getLoDecay(){ return LoDecay_;};
const float getRT60() { return RT60_;};
const float getMix() { return mix_;};


private:
    // Variables to be affected by the user interface
    uint8_t state_ = 1;
    uint8_t size_;
    float HiFreq_;
    float LoFreq_;
    float HiDecay_;
    float LoDecay_;
    float RT60_;
    float mix_;
    int8_t inc_;
    bool armed_ = false;

    DaisyPod *hwPod;
    ReverbEngine<float> *revEng;


};