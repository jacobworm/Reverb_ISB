#pragma once

#include "DelayLineBasic.h"
#include "Matrix_array.h"
#include "HighShelving.h"
#include "LowShelving.h"
#include "Constants.h"
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

//#define FDN_TEST

#ifdef FDN_TEST
#include <fstream>
#include <string>

constexpr int N_TEST = 48000;
#endif

template<typename SampleType>
class FDN
{
public:
    FDN()
    : FDN(48000)
    {
    }

    FDN(int fs)
    : sampleRate(fs)
    {
        delayLines.reserve(NUM_DELAYLINES);
        lowShelvings.reserve(NUM_DELAYLINES);
        highShelvings.reserve(NUM_DELAYLINES);
        for (int i = 0; i < NUM_DELAYLINES; ++i){
            int max_delay_samples = static_cast<int>(std::ceil(fs * delay_ms_fdn_default[i] / 1000.0f));
            float* buf = allocateDelaybuffer(); //Henter pointer til næste delaybuffer i delay_pool
            //delayLines.emplace_back(buf, ceil(((i+1) * 1.0f * max_delay_samples/NUM_DELAYLINES) + 2.0f));
            delayLines.emplace_back(buf, max_delay_samples + 2);
            lowShelvings.emplace_back(sampleRate);
            highShelvings.emplace_back(sampleRate);
        }
        /*
        for (int i = 0; i < NUM_DELAYLINES; ++i){
            delayLines.emplace_back(static_cast<size_t>(std::ceil(fs * delay_ms_fdn_default[i] / 1000.0f))); //sætter bufferstørrelse til max delay tid svarende til FDN_time_scaler = 1
            delayLines[i].setInterpolationMode(InterpolationMode::None);
        } */
        updateDelayTimes();
        updateFeedbackGain();
    }

    // Parameter setters

    void setFDNTimeScaler(float FDN_time_scaler_){
        // sikring af FDN_time_scaler imellem 0.1 og 1
        FDN_time_scaler = FDN_time_scaler_ < 0.1f ? 0.1f : FDN_time_scaler_ > 1.0f ? 1.0f : FDN_time_scaler_;
        updateDelayTimes(); 
        updateFeedbackGain();       
    }
    void setRT60(float RT60_){
        RT60 = RT60_ < 100.0f ? 100.0f : RT60_ > 10000.0f ? 10000.0f : RT60_;
        updateFeedbackGain();
        // Efter RT60 justering skal fintre opdateres for at have korrekt gain.
        for(size_t i = 0; i < NUM_DELAYLINES; i++){
            lowShelvings[i].setFeedbackGain(feedback_gain);
            highShelvings[i].setFeedbackGain(feedback_gain);
        }
    }

    void setLoDecay(SampleType lo_decay){
        for(size_t i = 0; i < NUM_DELAYLINES; i++){
            lowShelvings[i].setLoDecay(lo_decay);
        }
    }

    void setLoFreq(SampleType freq){
        for(size_t i = 0; i < NUM_DELAYLINES; i++){
            lowShelvings[i].setFreq(freq);
        }
    }

    void setHiDecay(SampleType hi_decay){
        for(size_t i = 0; i < NUM_DELAYLINES; i++){
            highShelvings[i].setHiDecay(hi_decay);
        }
    }

    void setHiFreq(SampleType freq){
        for(size_t i = 0; i < NUM_DELAYLINES; i++){
            highShelvings[i].setFreq(freq);
        }
    }

    void process(std::array<SampleType, NUM_DELAYLINES>& sample)
    {
        // Læs fra delaylines
        for (int i = 0; i < NUM_DELAYLINES; ++i)
        {
            //temp_sample[i] = sample[i];
            read_sample[i] = delayLines[i].read(delay_times[i]);
        }
        
        // Her foretages matrix-multiplikation af HadShuffle på `sample`
        feedb_sample = matrix.multiplyHadamardVector(read_sample);
        for(size_t i = 0; i < NUM_DELAYLINES; i++){
            //. Gain kompensation for Hadamard matrix
            feedb_sample[i] *= gainHadamardInv;

            // Shelving-filtrering
            feedb_sample[i] = highShelvings[i].process(feedb_sample[i]);
            feedb_sample[i] = lowShelvings[i].process(feedb_sample[i]);
        }
        // write med feedback
        for (int i = 0; i < NUM_DELAYLINES; ++i)
        {
            delayLines[i].write(feedb_sample[i] * feedback_gain + sample[i]);
        }
        #ifdef DIF_TEST
        // Til test-eksport. Ikke implementeret.
        #endif
        sample = read_sample; // output af delaylines returneres.

    }

private:
    void updateDelayTimes()
    {
        for (int n = 0; n < NUM_DELAYLINES; ++n)
        {
            delay_times[n] = static_cast<int>(std::round(sampleRate * delay_ms_fdn_default[n]*FDN_time_scaler/1000));
        }
        updateFeedbackGain();        
    }
    void updateFeedbackGain(){
        float T_avg_fdn = FDN_time_scaler * FDN_avg_delay_ms * FDN_tuning;
        feedback_gain = std::pow(10.0f, -3.0f * T_avg_fdn / RT60);
    }

    std::array<SampleType, NUM_DELAYLINES> read_sample;
    std::array<SampleType, NUM_DELAYLINES> feedb_sample;
    double sampleRate = 48000;
    float FDN_time_scaler = 0.9f;
    float FDN_tuning = 0.82f;
    float FDN_avg_delay_ms = 168;
    SampleType feedback_gain = 0.85f; //0.68f;
    SampleType RT60 = 2500;
    std::array<int, 8> delay_ms_fdn_default = {80, 107, 126, 139,157, 167,186, 197};
    static constexpr float gainHadamardInv = 0.353553f; // 1.0f / sqrt(NUM_DELAYLINES)
    //std::array<float, NUM_DELAYLINES> delay_distribution;
    std::array<int, NUM_DELAYLINES> delay_times;
    //int max_delay_samples;
    std::vector<DelayLineBasic<SampleType>> delayLines;
    std::vector<HighShelving<SampleType>> highShelvings;
    std::vector<LowShelving<SampleType>> lowShelvings;
    Matrix_array<SampleType> matrix;

    // til testkode. Ikke implementeret.
public:
    #ifdef FDN_TEST
    
    #endif

private:
    #ifdef FDN_TEST
    int testCounter = 0;
    std::array<std::array<SampleType, NUM_DELAYLINES>, N_TEST> testOutput;
    #endif
};