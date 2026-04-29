///////////////////////////////////////////////////////////
//  Algorithm.h
//  Implementation of the virtual Class Algorithm
//  Created on:      10-march-2026 09:31:01
//  Original author: JSW
///////////////////////////////////////////////////////////

class Algorithm {
    public:
    virtual ~Algorithm() = default;
    virtual float Process (float input) = 0;
    virtual void Init (int samplerate) = 0;
    private:
};