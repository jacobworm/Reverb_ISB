#pragma once

#include "Diffusor.h"
#include "FDN.h"
#include "Constants.h"

//constexpr int NUM_DELAYLINES = 8;
constexpr int DIFFUSER_STEP_TO_EARLY = 1; //OBS: 0-indekseret
template <typename SampleType>
class ReverbEngine{

public:
/*
    ReverbEngine()
    : diffusor_delay_distribution{{
        {{ 0.70f, 0.32f, 0.45f, 0.20f, 0.84f, 0.67f, 0.18f, 0.23f }},
        {{ 0.22f, 0.77f, 0.45f, 0.67f, 0.84f, 0.14f, 0.97f, 0.80f }},
        {{ 0.72f, 0.35f, 0.12f, 0.58f, 0.12f, 0.25f, 0.93f, 0.24f }},
        {{ 0.43f, 0.35f, 0.84f, 0.96f, 0.67f, 0.88f, 0.31f, 0.42f }},
        {{ 0.70f, 0.44f, 0.87f, 0.39f, 0.63f, 0.73f, 0.74f, 0.70f }}
    }}, 
    diffusor_max_delay_ms{30, 60, 120, 240, 480}, 
    sampleRate(48000.0f),
    diffusors{{
    Diffusor<SampleType>(static_cast<int>(sampleRate), 0, diffusor_max_delay_ms[0], diffusor_delay_distribution[0]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 1, diffusor_max_delay_ms[1], diffusor_delay_distribution[1]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 2, diffusor_max_delay_ms[2], diffusor_delay_distribution[2]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 3, diffusor_max_delay_ms[3], diffusor_delay_distribution[3]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 4, diffusor_max_delay_ms[4], diffusor_delay_distribution[4])
    }} 
    {
    }
    */
   
   //Constructor for 4 step diffusor (DEBUG)
    ReverbEngine()
    : diffusor_delay_distribution{{
        {{ 0.70f, 0.32f, 0.45f, 0.20f, 0.84f, 0.67f, 0.18f, 0.23f }},
        {{ 0.22f, 0.77f, 0.45f, 0.67f, 0.84f, 0.14f, 0.97f, 0.80f }},
        {{ 0.72f, 0.35f, 0.12f, 0.58f, 0.12f, 0.25f, 0.93f, 0.24f }},
        {{ 0.43f, 0.35f, 0.84f, 0.96f, 0.67f, 0.88f, 0.31f, 0.42f }}
    }}, 
    diffusor_max_delay_ms{30, 60, 120, 240}, 
    sampleRate(48000.0f),
    diffusors{{
    Diffusor<SampleType>(static_cast<int>(sampleRate), 0, diffusor_max_delay_ms[0], diffusor_delay_distribution[0]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 1, diffusor_max_delay_ms[1], diffusor_delay_distribution[1]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 2, diffusor_max_delay_ms[2], diffusor_delay_distribution[2]),
        Diffusor<SampleType>(static_cast<int>(sampleRate), 3, diffusor_max_delay_ms[3], diffusor_delay_distribution[3])
    }} 
    {
    }
    
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
            

        fdn.process(sample_8ch);


        std::array<SampleType,2> output {{0.0f, 0.0f}};
        //Temp summeret output til test af diffusorer:::::::::::
        /*count = 0;
        for (int i = 0; i<NUM_DELAYLINES;i++){
            output[count]+=sample_8ch[i];
            count++;
            if (count > 1){
                count = 0;
            }
        }*/
       // Implementering af mix, early level og late level
        output[0]=input[0]* direct_level + wet_level * late_level * sample_8ch[0];
        output[1]=input[1]* direct_level + wet_level * late_level * sample_8ch[1];

        return output;
    }
    // USER PARAMETERS::::::::::::::::::::::::::::::::
    void setRT60(float rt60_ms){ //Range 100 - 10000
        fdn.setRT60(rt60_ms);
    }

    void setSize(float size){ // Range 1 - 100
        // Kald til kontrol af FDN time scaler, Diff time scaler, Diff_skew, early_size
        for(int i = 0; i < NUM_STEPS; i++){
            float skew_ = 0.01282f * size + 0.4876f;
            float diff_time_scaler_ = 0.0001643f * size * size - 0.0008811 * size + 0.01027f + 0.041f;
            float fdn_time_scaler_ = 0.00009609f * size * size + 0.00362 * size + 0.09626;
            diffusors[i].setParameters(skew_, diff_time_scaler_);
            fdn.setFDNTimeScaler(fdn_time_scaler_);
        }
    }

    void setLoDecay(SampleType lo_decay){ // Range:0.1 - 10
        fdn.setLoDecay(lo_decay);
    }

    void setLoFreq(SampleType freq){ // Range: 20 - 1000
        fdn.setLoFreq(freq);
    }
    
    void setHiDecay(SampleType hi_decay){ // Range:0.1 - 10
        fdn.setHiDecay(hi_decay);
    }

    void setHiFreq(SampleType freq){ // Range: 500 - 10000
        fdn.setLoFreq(freq);
    }

    void setMix(float value_pct){ // Range: 0 - 100;
        direct_level = (200.0f - 2 * value_pct)/100.0f > 1 ? 1 : (200.0f - 2 * value_pct)/100.0f; //Directlevel 100% indtil mix=50%. Derefter aftager direct level mod 0.
        wet_level = 2 * value_pct / 100;
    }
    // END OF USER PARAMETERS::::::::::::::::::::::::::::::::::::::::

    #ifdef DIF_TEST
    void exportAllTestOutputs() const {
        for (size_t i = 0; i < NUM_STEPS; ++i) {
            diffusors[i].exportTestOutputToFile();
        }
    }
    #endif

private:
    SampleType late_level = 1.0f;
    SampleType direct_level = 1.0f;
    SampleType wet_level = 0.5;
    // Diffusor-related private attributes
    std::array<std::array<float, NUM_DELAYLINES>,NUM_STEPS> diffusor_delay_distribution;
    std::array<size_t, NUM_STEPS> diffusor_max_delay_ms; 
    double sampleRate = 48000;   
    std::array<Diffusor<SampleType>, NUM_STEPS> diffusors;
    std::array<SampleType,NUM_DELAYLINES> sample_8ch = {0.0f, 0.0f};
    std::array<SampleType,NUM_DELAYLINES> sample_8ch_early = {0.0f, 0.0f};

    // FDN-related private attributes
    FDN<float> fdn;
};