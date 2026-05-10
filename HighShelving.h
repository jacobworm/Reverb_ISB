#pragma once
//#include <juce_audio_basics/juce_audio_basics.h>
// #include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <vector>
#include "Constants.h"


template<typename SampleType>
class HighShelving
{
public:
HighShelving(double samplerate) : SampleRate(samplerate){
    // initialize coefficients with reasonable defaults
    calculateCoeffs(2000.0f, 1.0f, 0.5f);
}

void calculateCoeffs(float freq, float hi_decay, float feedback_gain){
    T = 1.0f / SampleRate;
    K = (1-(1-feedback_gain) / hi_decay) / feedback_gain;
    if (K < 0.0f){
        K = 0.0f;
    }
    omega = 2 * PI * freq;
    b0 = 2.0f * K + omega * T;
    b1 = (-2.0f) * K + omega * T;
    a0 = 2.0f + omega * T;
    a1 = (-2.0f) + omega * T;
    b0overa0 = b0 / a0;
    b1overa0 = b1 / a0;
    a1overa0 = a1 / a0;
}

SampleType process(SampleType input){
    // Direct form I-like update using previous input/output samples
    SampleType output = b0overa0 * input + b1overa0 * x_old - a1overa0 * y_old;
    // Update old samples
    x_old = input;
    y_old = output;
    return output;
}

private:
SampleType T = 1.0f/48000.0f;
SampleType K = 0.0f;
SampleType omega = 1200.0f;
SampleType b0 = 0.0f;
SampleType b1 = 0.0f;
SampleType a0 = 0.0f;
SampleType a1 = 0.0f;
SampleType b0overa0 = 0.0f;
SampleType b1overa0 = 0.0f;
SampleType a1overa0 = 0.0f;

double SampleRate = 48000;
SampleType y_old = 0.0f;
SampleType x_old = 0.0f;


};