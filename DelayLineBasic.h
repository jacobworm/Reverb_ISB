#pragma once
//#include <juce_audio_basics/juce_audio_basics.h>
// #include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <vector>

template<typename SampleType>
class DelayLineBasic
{
public:
    DelayLineBasic (SampleType* externalBuffer, int max_delay) 
    :   buffer_ptr(externalBuffer),
        maxDelayInSamples(max_delay),         
        writePosition(0)
    {
        if (buffer_ptr == nullptr || maxDelayInSamples <= 0)
        {
            fallbackSample = SampleType(0);
            buffer_ptr = &fallbackSample;
            maxDelayInSamples = 1;
        }
        std::fill(buffer_ptr, buffer_ptr + maxDelayInSamples, SampleType(0));
    }

    void write(SampleType sample){
        buffer_ptr[writePosition]=sample;
        writePosition ++;
        if (writePosition>= maxDelayInSamples) {
            writePosition -= maxDelayInSamples;
        }        
    }

    SampleType read(int delayInSamples){
        if (buffer_ptr == nullptr || maxDelayInSamples == 0) {
            return SampleType(0);
        }

        int readPositionInt = writePosition - delayInSamples;

        if (readPositionInt < 0) {
            while (readPositionInt < 0) {
                readPositionInt += maxDelayInSamples;
            }
        } else if (readPositionInt >= maxDelayInSamples) {
            while (readPositionInt >= maxDelayInSamples) {
                readPositionInt -= maxDelayInSamples;
            }
        }

        return buffer_ptr[readPositionInt];
    }

private:
    SampleType* buffer_ptr;    
    SampleType fallbackSample = SampleType(0);
    int maxDelayInSamples;
    int writePosition;   
};