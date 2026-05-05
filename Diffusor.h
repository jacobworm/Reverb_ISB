#pragma once

#include "DelayLineBasic.h"
#include "Matrix_array.h"
#include "Constants.h"
#include "SdramPool.h"
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

//#define DIF_TEST

#ifdef DIF_TEST
#include <fstream>
#include <string>

constexpr int N_TEST = 48000;
#endif



template<typename SampleType>
class Diffusor
{
public:
    Diffusor()
    : Diffusor(48000.0f, 0, 0, std::array<float, NUM_DELAYLINES>{})
    {
    }

    Diffusor(double fs, size_t diff_num_, int max_delay_ms, std::array<float, NUM_DELAYLINES> delay_distribution_)
    : sampleRate(fs), diffusor_num(diff_num_),delay_distribution(delay_distribution_)
    {
        max_delay_samples = static_cast<int>(fs * max_delay_ms / 1000);        
        delayLines.reserve(NUM_DELAYLINES);
        for (int i = 0; i < NUM_DELAYLINES; ++i){
            float* buf = allocateDelaybuffer(); //Henter pointer til næste delaybuffer i delay_pool
            //delayLines.emplace_back(buf, ceil(((i+1) * 1.0f * max_delay_samples/NUM_DELAYLINES) + 2.0f));
            delayLines.emplace_back(buf, max_delay_samples + 2);
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
            temp_sample = sample[i];
            sample[i] = delayLines[i].read(delay_times[i]);
            delayLines[i].write(temp_sample);
        }
        // Her foretages matrix-multiplikation af HadShuffle på `sample`
        sample = matrix.multiplyShuffleHadVector(sample, diffusor_num);
        for(size_t i = 0; i < NUM_DELAYLINES; i++){
            sample[i] *= gainHadamardInv;
        } 
        #ifdef DIF_TEST
        if (testCounter < N_TEST) {
            testOutput[testCounter] = sample;
            testCounter++;
        }
        #endif
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
            delay_times[n] = delay_times[n] < 1 ? 1 : delay_times[n] >  static_cast<int>(max_delay_samples) ? static_cast<int>(max_delay_samples) :  delay_times[n];
        }        
    }    
    double sampleRate = 48000;
    size_t diffusor_num = 0;
    std::array<float, NUM_DELAYLINES> delay_distribution;
    float diffusor_skew = 100.0f;
    float diffusor_time_scaler = 1.0f;
    static constexpr float gainHadamardInv = 0.353553f; // 1.0f / sqrt(NUM_DELAYLINES)
    std::array<int, NUM_DELAYLINES> delay_times;
    int max_delay_samples;
    SampleType temp_sample;
    std::vector<DelayLineBasic<SampleType>> delayLines;
    Matrix_array<SampleType> matrix;

public:
    #ifdef DIF_TEST
    void exportTestOutputToFile() const {
        std::string filename = "diffusor_output_" + std::to_string(diffusor_num) + ".txt";
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Could not open file: " << filename << std::endl;
            return;
        }
        for (int i = 0; i < testCounter; ++i) {
            for (int j = 0; j < NUM_DELAYLINES; ++j) {
                file << testOutput[i][j];
                if (j < NUM_DELAYLINES - 1) file << " ";
            }
            file << "\n";
        }
        file.close();
        std::cout << "Exported " << testCounter << " samples to " << filename << std::endl;
    }
    #endif

private:
    #ifdef DIF_TEST
    int testCounter = 0;
    std::array<std::array<SampleType, NUM_DELAYLINES>, N_TEST> testOutput;
    #endif
};