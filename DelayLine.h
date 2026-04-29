#pragma once
//#include <juce_audio_basics/juce_audio_basics.h>
// #include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <vector>
enum class InterpolationMode 
{
    None = 0,
    Linear = 1,
    Lagrange2 = 2,
    Lagrange3 = 3,
    AllPass = 4
};

template<typename SampleType>
class DelayLine
{
public:
    DelayLine (size_t max_delay) 
    :   buffer(static_cast<int>(max_delay)),
        maxDelayInSamples(max_delay),         
        writePosition(0),
        AllPassState(SampleType(0))
    {

    }

    void write(SampleType sample){
        buffer[writePosition]=sample;
        writePosition +=1;
        if (writePosition>= maxDelayInSamples) {
            writePosition -= maxDelayInSamples;
        }        
    }

    SampleType read(float delayInSamples){
        
        switch (interpolationMode){
            case InterpolationMode::None: {
                int readPosition_int=static_cast<int>(std::round(writePosition-delayInSamples));
                if (readPosition_int < 0) {
                    readPosition_int += static_cast<int>(maxDelayInSamples);
                }
                SampleType output = buffer[readPosition_int];
                return output;
                break;
            }
            case InterpolationMode::Linear: {
                float readPosition = writePosition - delayInSamples;
                int readPosition_floor = static_cast<int>(std::floor(readPosition));
                float delayFract = readPosition - readPosition_floor;

                int readPosition_floorPlusOne = readPosition_floor + 1;
                if (readPosition_floorPlusOne < 0) {
                    readPosition_floorPlusOne += static_cast<int>(maxDelayInSamples);
                }
                if (readPosition_floorPlusOne >= static_cast<int>(maxDelayInSamples)) {
                    readPosition_floorPlusOne -= static_cast<int>(maxDelayInSamples);
                }
                if (readPosition_floor < 0) {
                    readPosition_floor += static_cast<int>(maxDelayInSamples);
                }
                
                SampleType output = buffer[readPosition_floor]*(1-delayFract)+buffer[readPosition_floorPlusOne]*(delayFract);
                return output;
                break;
            }
            case InterpolationMode::Lagrange2: {
                // Implementer Lagrange interpolation
                float readPosition = writePosition - delayInSamples;
                int read_0 = static_cast<int>(std::floor(readPosition));
                float delayFract = readPosition - read_0;
                if (read_0 < 0) {
                    read_0 += static_cast<int>(maxDelayInSamples);
                }
                int read_m1 = read_0 - 1;
                if (read_m1 < 0) {
                    read_m1 += static_cast<int>(maxDelayInSamples);
                }
                int read_p1 = read_0 + 1;
                if (read_p1 >= static_cast<int>(maxDelayInSamples)) {
                    read_p1 -= static_cast<int>(maxDelayInSamples);
                }
                /*int read_p2 = read_0 + 2;
                if (read_p2 > static_cast<int>(maxDelayInSamples)) {
                    read_p2 -= static_cast<int>(maxDelayInSamples);
                } */
                SampleType a = static_cast<SampleType> (delayFract);
                SampleType output = ((buffer[read_m1]-2*buffer[read_0]+buffer[read_p1])*a*a+(buffer[read_p1]-buffer[read_m1])*a+2*buffer[read_0]) * SampleType(0.5);
                

                return output;
                break;
            }
            case InterpolationMode::Lagrange3: {
                // Implementer Lagrange interpolation
                float readPosition = writePosition - delayInSamples;
                if (readPosition < 0) {
                    readPosition += static_cast<float>(maxDelayInSamples);
                }
                int read_0 = static_cast<int>(std::floor(readPosition));
                float delayFract = readPosition - read_0;
                int read_m1 = read_0 - 1;
                if (read_m1 < 0) {
                    read_m1 += static_cast<int>(maxDelayInSamples);
                }
                int read_p1 = read_0 + 1;
                if (read_p1 >= static_cast<int>(maxDelayInSamples)) {
                    read_p1 -= static_cast<int>(maxDelayInSamples);
                }
                int read_p2 = read_0 + 2;
                if (read_p2 >= static_cast<int>(maxDelayInSamples)) {
                    read_p2 -= static_cast<int>(maxDelayInSamples);
                }
                SampleType a = static_cast<SampleType>(delayFract);
                SampleType x_1 = buffer[read_m1];
                SampleType x_2 = buffer[read_0];
                SampleType x_3 = buffer[read_p1];
                SampleType x_4 = buffer[read_p2];
                SampleType output =
                ((((3 * (x_2 - x_3) + x_4 - x_1) *
                        a // Omskrevet efter horner's method, færre memory-opslag
                    + 3 * (x_3 + x_1 - 2 * x_2)) *
                        a +
                    (6 * x_3 - 3 * x_2 - 2 * x_1 - x_4)) *
                a) *
                    SampleType(1.0/6.0) +
                x_2;

            return output;
            break;
    }
            case InterpolationMode::AllPass: {
                // Implementer AllPass interpolation
                return SampleType(0);
                break;
            }
            default:
                return SampleType(0);
        }
        
    }


    void clear() {
        std::fill(buffer.begin(), buffer.end(), SampleType(0));              // Clear AudioBuffer
        writePosition = 0;           // Reset write position  
        // Clear AllPassState if needed:
        AllPassState = SampleType(0);
    }
    void setBufferSize(size_t new_max_delay){  //Reallocation memory. Do not call during playback
        if(new_max_delay != maxDelayInSamples){
            maxDelayInSamples = new_max_delay;
            buffer.resize(new_max_delay);
            writePosition = 0;
            std::fill(buffer.begin(),buffer.end(), SampleType(0));
        }
    }
    void setInterpolationMode(InterpolationMode mode){
        interpolationMode = mode;
    }
    /* void prepare(size_t max_delay, InterpolationMode mode){
        maxDelayInSamples = max_delay;
        buffer.resize(max_delay);
        interpolationMode = mode;
    } */
private:
    /* void setMaxDelay(size_t max_delay){
        maxDelayInSamples = max_delay;
    } */

    std::vector<SampleType> buffer;
    SampleType AllPassState;  // For AllPass interpolation feedback    
    size_t maxDelayInSamples;
    size_t writePosition;
//    SampleType currentDelay = 0.0;
    InterpolationMode interpolationMode = InterpolationMode::Lagrange3;    
};