#pragma once
//#include <juce_audio_basics/juce_audio_basics.h>
// #include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <vector>

template<typename SampleType>
class DelayLineBasic
{
public:
    DelayLineBasic (int max_delay) 
    :   buffer(static_cast<size_t>(max_delay)),
        maxDelayInSamples(max_delay),         
        writePosition(0)
    {

    }

    void write(SampleType sample){
        buffer[writePosition]=sample;
        writePosition +=1;
        if (writePosition>= maxDelayInSamples) {
            writePosition -= maxDelayInSamples;
        }        
    }

    SampleType read(int delayInSamples){
        if (buffer.empty() || maxDelayInSamples == 0) {
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

        return buffer[static_cast<size_t>(readPositionInt)];
    }

    void clear() {
        std::fill(buffer.begin(), buffer.end(), SampleType(0));              // Clear AudioBuffer
        writePosition = 0;           // Reset write position  
    }
    void setBufferSize(int new_max_delay){  //Reallocation memory. Do not call during playback
        if(new_max_delay != maxDelayInSamples){
            maxDelayInSamples = new_max_delay;
            buffer.resize(static_cast<size_t>(new_max_delay));
            writePosition = 0;
            std::fill(buffer.begin(),buffer.end(), SampleType(0));
        }
    }
private:
    std::vector<SampleType> buffer;    
    int maxDelayInSamples;
    int writePosition;   
};