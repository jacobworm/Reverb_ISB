#pragma once

#include "Diffusor.h"
#include "FDN.h"
#include "Constants.h"

//constexpr int NUM_DELAYLINES = 8;
constexpr int DIFFUSER_STEP_TO_EARLY = 1; //OBS: 0-indekseret
template <typename SampleType>
class ReverbEngine{

public:
    ReverbEngine()
    : diffusor_delay_distribution{{
        {{ 0.70f, 0.32f, 0.45f, 0.20f, 0.84f, 0.67f, 0.18f, 0.23f }},
        {{ 0.22f, 0.77f, 0.45f, 0.67f, 0.84f, 0.14f, 0.97f, 0.80f }},
        {{ 0.72f, 0.35f, 0.12f, 0.58f, 0.12f, 0.25f, 0.93f, 0.24f }},
        {{ 0.43f, 0.35f, 0.84f, 0.96f, 0.67f, 0.88f, 0.31f, 0.42f }},
        {{ 0.70f, 0.44f, 0.87f, 0.39f, 0.63f, 0.73f, 0.74f, 0.70f }}
    }}, diffusor_max_delay_ms{30, 60, 120, 240, 480}, sampleRate(static_cast<int>(48000)),
    diffusors{{
    Diffusor<SampleType>(static_cast<int>(sampleRate), 0, diffusor_max_delay_ms[0], diffusor_delay_distribution[0]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 1, diffusor_max_delay_ms[1], diffusor_delay_distribution[1]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 2, diffusor_max_delay_ms[2], diffusor_delay_distribution[2]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 3, diffusor_max_delay_ms[3], diffusor_delay_distribution[3]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 4, diffusor_max_delay_ms[4], diffusor_delay_distribution[4])
    }} 
    {
    }
    
   /*
   //Constructor for 4 step diffusor (DEBUG)
    ReverbEngine()
    : diffusor_delay_distribution{{
        {{ 0.70f, 0.32f, 0.45f, 0.20f, 0.84f, 0.67f, 0.18f, 0.23f }},
        {{ 0.22f, 0.77f, 0.45f, 0.67f, 0.84f, 0.14f, 0.97f, 0.80f }},
        {{ 0.72f, 0.35f, 0.12f, 0.58f, 0.12f, 0.25f, 0.93f, 0.24f }},
        {{ 0.43f, 0.35f, 0.84f, 0.96f, 0.67f, 0.88f, 0.31f, 0.42f }}
    }}, diffusor_max_delay_ms{30}, sampleRate(static_cast<int>(48000)),
    diffusors{{
    Diffusor<SampleType>(static_cast<int>(sampleRate), 0, diffusor_max_delay_ms[0], diffusor_delay_distribution[0]),
    Diffusor<SampleType>(static_cast<int>(sampleRate), 1, diffusor_max_delay_ms[1], diffusor_delay_distribution[1]),
    Diffusor<SampleType>(static_cast<int>(sampleRate), 2, diffusor_max_delay_ms[2], diffusor_delay_distribution[2]),
    Diffusor<SampleType>(static_cast<int>(sampleRate), 3, diffusor_max_delay_ms[3], diffusor_delay_distribution[3])
    }} 
    {
    }
    */
    std::array<SampleType,2> process(std::array<SampleType,2> input){
        // distribuerer input til 8 channels, L R L R L R L R
        int count = 0;
        for (int i = 0; i<NUM_DELAYLINES;i++){
            sample_8ch[i]=input[count];
            count++;
            if (count > 1){
                count = 0;
            }
        }

        // Processerer igennem diffusorer.
        for (int i = 0; i<NUM_STEPS;i++){
            diffusors[i].process(sample_8ch);
            if(i == DIFFUSER_STEP_TO_EARLY){
                sample_8ch_early = sample_8ch; // Tapning af signal til early reflection
            }
        }


        std::array<SampleType,2> output {{0.0f, 0.0f}};
        //Temp summeret output til test af diffusorer:::::::::::
        count = 0;
        for (int i = 0; i<NUM_DELAYLINES;i++){
            output[count]+=sample_8ch[i];
            count++;
            if (count > 1){
                count = 0;
            }
        }
        return output;
    }

    #ifdef DIF_TEST
    void exportAllTestOutputs() const {
        for (size_t i = 0; i < NUM_STEPS; ++i) {
            diffusors[i].exportTestOutputToFile();
        }
    }
    #endif

private:
    int sampleRate;
    // Diffusor-related private attributes
    std::array<std::array<float, NUM_DELAYLINES>,NUM_STEPS> diffusor_delay_distribution;
    std::array<size_t, NUM_STEPS> diffusor_max_delay_ms;    
    std::array<Diffusor<SampleType>, NUM_STEPS> diffusors;
    std::array<SampleType,NUM_DELAYLINES> sample_8ch = {0.0f, 0.0f};
    std::array<SampleType,NUM_DELAYLINES> sample_8ch_early = {0.0f, 0.0f};

    // FDN-related private attributes
    //FDN<float> fdn;
};