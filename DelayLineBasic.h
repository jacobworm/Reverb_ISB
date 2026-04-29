#pragma once
//#include <juce_audio_basics/juce_audio_basics.h>
// #include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <vector>

template<typename SampleType>
class DelayLineBasic
{
public:
    DelayLineBasic (size_t max_delay) 
    :   buffer(static_cast<int>(max_delay)),
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
        int readPosition_int=static_cast<int>(std::round(writePosition-delayInSamples));
                if (readPosition_int < 0) {
                    readPosition_int += static_cast<int>(maxDelayInSamples);
                }
                SampleType output = buffer[readPosition_int];
                return output;
    }


    void clear() {
        std::fill(buffer.begin(), buffer.end(), SampleType(0));              // Clear AudioBuffer
        writePosition = 0;           // Reset write position  
    }
    void setBufferSize(size_t new_max_delay){  //Reallocation memory. Do not call during playback
        if(new_max_delay != maxDelayInSamples){
            maxDelayInSamples = new_max_delay;
            buffer.resize(new_max_delay);
            writePosition = 0;
            std::fill(buffer.begin(),buffer.end(), SampleType(0));
        }
    }
private:
    std::vector<SampleType> buffer;    
    size_t maxDelayInSamples;
    size_t writePosition;   
};