#pragma once

#include "DelayLineBasic.h"
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

constexpr int NUM_DELAYLINES = 8;

template<typename SampleType>
class Diffusor
{
public:
    Diffusor(int fs, int diff_num_, size_t max_delay_ms, std::array<float, NUM_DELAYLINES> delay_distribution_)
    : diffusor_num(diff_num_),sampleRate(fs), delay_distribution(delay_distribution_)
    {
        max_delay_samples = fs * max_delay_ms / 1000;
        delayLines.reserve(NUM_DELAYLINES);
        for (int i = 0; i < NUM_DELAYLINES; ++i){
            delayLines.emplace_back(max_delay_samples);
        }
        updateDelayTimes();
    }

    void setParameters(float skew_, float diffusor_time_scaler_){
        // sikring af at skew ikke bliver lavere end 0.5
        diffusor_skew = skew_ < 0.5f ? 0.5f : skew_;
        // sikring af diffusor_time_scaler imellem 0.0167 og 1
        diffusor_time_scaler = diffusor_time_scaler_ < 0.0167 ? 0.0167 : diffusor_time_scaler_ > 1.0f ? 1.0f : diffusor_time_scaler_;
        updateDelayTimes();
        
    }

    void process(std::array<SampleType, NUM_DELAYLINES>& sample)
    {
        //std::array<SampleType, NUM_DELAYLINES> out{};
        for (int i = 0; i < NUM_DELAYLINES; ++i)
        {
            delayLines[i].write(sample[i]);
            sample[i] = delayLines[i].read(delay_times[i]);
        }
        // Her kan du foretage matrix-multiplikation af HadShuffle på `out`
            
    }

private:
    float calculateSkew(int n, int num_delays, float diff_skew){
        float skew_fact = (std::pow(2, (n/diff_skew))-1)/(std::pow(2,(num_delays/diff_skew))-1);
        return skew_fact;
    }
    void updateDelayTimes()
    {
        float delay = max_delay_samples * diffusor_time_scaler;
        for (int n = 0; n < NUM_DELAYLINES; ++n)
        {
            // Inddeler intervallet fra 0 til 1 i NUM_DELAYLINES eksponentielt inddelte intervaller. Placerer delaytider pseudo-random i disse slots med fordelingerne i delay_distribution
            float diff_delay_skew = calculateSkew(n,NUM_DELAYLINES,diffusor_skew)+(calculateSkew(n+1,NUM_DELAYLINES,diffusor_skew)-calculateSkew(n,NUM_DELAYLINES,diffusor_skew)) * delay_distribution[n];
            delay_times[n] = static_cast<int>(std::ceil(diff_delay_skew * delay));
            delay_times[n] = std::clamp(delay_times[n], 0, static_cast<int>(max_delay_samples));
        }        
    }
    double sampleRate = 48000;
    float diffusor_skew = 5.0f;
    float diffusor_time_scaler = 1.0f;
    int diffusor_num;
    std::array<float, NUM_DELAYLINES> delay_distribution;
    std::array<int, NUM_DELAYLINES> delay_times;
    size_t max_delay_samples;
    std::vector<DelayLineBasic<SampleType>> delayLines;
};