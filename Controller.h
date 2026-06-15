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
#define PCT_THRESHOLD 0.02 // Percent of max value used as threshold value in decimal

class controller
{
public:
// Constructor
controller(DaisyPod *hwPod, ReverbEngine<float>* revEng)
: hwPod(hwPod), revEng(revEng){}

~controller(){}; // Std. destructor

void update()
    {

        float RT60_temp = 0, mix_temp = 0;
        float HiFreq_temp = 0, LoFreq_temp = 0;
        float HiDecay_temp = 0, LoDecay_temp = 0;

        // Turn off LED2
        hwPod->led2.Set(0.0f, 0.0f, 0.0f);



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

            // Pot1 changes RT60 between 100 and 10000
            RT60_temp = 100 + (hwPod->knob1.Value() * (9900));

            // Pot2 changes mix between 0 and 100
            mix_temp = hwPod->knob2.Value() * 100;

            // Update values if knob value is within threshold value
            if(fabs(RT60_ - RT60_temp) < RT60_threshold)
            {
                // Update RT60_
                RT60_ = RT60_temp;
                revEng->setRT60(RT60_);

                // Set LED2
                hwPod->led2.Set(1.0f, 1.0f, 1.0f);

            }

            // Evaluate if encoder has been incremented
            inc_ = hwPod->encoder.Increment();

            // Switch 1 and 2 in- and decrements and sets size
            if(inc_ > 0) // Increment size
            {
                size_ += 2;
                if(size_ > 100) size_ = 100;
                else if(size_ < 0) size_ = 0;

                revEng->setSize(size_); // Set size

            }
            else if(inc_ < 0) // Decrement size
            {
                size_ -= 2;
                if(size_ < 0) size_ = 0;
                else if (size_ > 100) size_ = 100;
                
                revEng->setSize(size_); // Set size

            }


            if (fabs(mix_ - mix_temp) < mix_treshold)
            {
                // Update mix value
                mix_ = mix_temp;
                revEng->setMix(mix_);

                // Set LED2
                hwPod->led2.Set(1.0f, 1.0f, 1.0f);
            }

            break;
        case 2:

            // State 2 -> Set LED1 to green
            hwPod->led1.Set(0.0f, 1.0f, 0.0f);

            // Pot2 changes HiFreq between 500 and 10000
            HiFreq_temp = 500 + (hwPod->knob2.Value() * (9500));

            // Pot1 changes LoFreq between 20 and 1000
            LoFreq_temp = 20 + (hwPod->knob1.Value() * 980);


            // Evaluate if Pot2 is within threshold value
            if(fabs(HiFreq_ - HiFreq_temp) < HiFreq_threshold)
            {
                // Update values                
                HiFreq_ = HiFreq_temp;
                revEng->setHiFreq(HiFreq_);

                // Set LED2
                hwPod->led2.Set(1.0f, 1.0f, 1.0f);
            }

            // Evaluate if Pot1 is within threshold value
            if(fabs(LoFreq_ - LoFreq_temp) < LoFreq_threshold)
            {
                // Pot2 changes LoFreq between 20 and 1000
                LoFreq_ = LoFreq_temp;
                revEng->setLoFreq(LoFreq_);

                // Set LED2
                hwPod->led2.Set(1.0f, 1.0f, 1.0f);
            }

            break;
        case 3:
            // State 3 -> Set LED1 to blue
            hwPod->led1.Set(0.0f, 0.0f, 1.0f);

            // Pot2 changes HiDecay between 0.1 and 10
            HiDecay_temp = (0.1 + (hwPod->knob2.Value() * 9.9));

            // Pot1 changes LoDecay between 0.1 and 10 
            LoDecay_temp = 0.1 + (hwPod->knob1.Value() * 9.9);

            // Update values if within threshold
            if(fabs(HiDecay_ - HiDecay_temp) < decay_threshold)
            {
                // Pot2 changes HiDecay between 0.1 and 10
                HiDecay_ = HiDecay_temp;
                revEng->setHiDecay(HiDecay_);

                // Set LED2
                hwPod->led2.Set(1.0f, 1.0f, 1.0f);
            }


            // Update values if within threshold
            if(fabs(LoDecay_ - LoDecay_temp) < decay_threshold)
            {

                // Pot1 changes LoDecay between 0.1 and 10
                LoDecay_ = LoDecay_temp;
                revEng->setLoDecay(LoDecay_);

                // Set LED2
                hwPod->led2.Set(1.0f, 1.0f, 1.0f);
            }

            break;
        default:
            // If state is not 1, 2 or 3 -> Set state to 1
            state_ = 1;
            break;
        }

    };

// Getters
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
    int8_t size_ = 80;
    float HiFreq_ = 2000;
    float LoFreq_ = 300;
    float HiDecay_ = 0.4;
    float LoDecay_ = 2;
    float RT60_ = 2500;
    float mix_ = 50;
    int8_t inc_ = 0;
    float RT60_threshold = 10000 * PCT_THRESHOLD;
    float HiFreq_threshold = 10000 * PCT_THRESHOLD;
    float LoFreq_threshold = 1000 * PCT_THRESHOLD;
    float decay_threshold = 10 * PCT_THRESHOLD;
    float mix_treshold = 100 * PCT_THRESHOLD;


    DaisyPod *hwPod;
    ReverbEngine<float> *revEng;

};