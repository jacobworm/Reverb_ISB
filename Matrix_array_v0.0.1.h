#pragma once

#include <array>
#include <type_traits>
#include <cstdint>
#include <string>
#include <stdexcept>
#include "core_cm7.h"
#include "daisy_pod.h"
#include "daisysp.h"
#include <daisy_core.h>

// Hadamard matrix Zero-init and allocated to DTCM ram.
DTCM_MEM_SECTION inline
std::array<std::array<float, 8>, 8> hadamardMatrix_{};

// shuffleHad matrix Zero-init and allocated to DTCM ram
DTCM_MEM_SECTION inline
std::array<std::array<std::array<float, 8>, 8>, 5> shuffleHad_{};


// Matrix-vektor multiplikation
template<typename scalar, size_t N = 8>
std::array<scalar, N> multiplyMatrixVector(
    const std::array<scalar, N>& vec,
    const std::array<std::array<scalar, N>, N>& mat)
{
    // Lav resultat vektor n_ lang med 0 i alle pladser
    std::array<scalar, N> resultVec{};
    
    // Dot produktet mellem række i matrix og vektor udregnes
    // Først itereres gennem rækkerne
    for (size_t i = 0; i < N; i++)
    {
        // For hver række itereres der gennem kolonnerne
        for (size_t j = 0; j < N; j++)
        {
            // Hver kolonne i en given række for matricen ganges på hver række i vektoren
            // og adderes til sidst i resultatvektoren
            resultVec[i] += mat[i][j] * vec[j];
        }
    }
    return resultVec;
}

    
// Matrix-vektor multiplikation, ShuffleHad
template<typename scalar, size_t N = 8>
std::array<scalar, N> multiplyShuffleHadVector(
    const std::array<scalar, N>& vec,
    size_t shuffleHad_Nr)
{
    //if (shuffleHad_Nr >= shuffleHad_.size()){
    //    throw std::out_of_range("shuffleHad_Nr out of range (0-4)");
    //    }
    return multiplyMatrixVector(vec, shuffleHad_[shuffleHad_Nr]);        
}


// Matrix-vektor multiplikation, Hadamard
template<typename scalar, size_t N = 8>
std::array<scalar, N> multiplyHadamardVector(const std::array<scalar, N>& vec)
{
    return multiplyMatrixVector(vec, hadamardMatrix_);
}


// Konstruerer Hadamard matrix m. Sylvester konstruktion (N x N)
void createHadamardMatrix(uint8_t N)
{
    hadamardMatrix_[0][0] = 1;

    size_t currentSize = 1;
    while (currentSize < N)
    {
        // Kopier den eksisterende matrix til øverste venstre kvadrant
        for (size_t i = 0; i < currentSize; i++)
        {
            for (size_t j = 0; j < currentSize; j++)
            {
                float val = hadamardMatrix_[i][j];
                
                // Øverste venstre (allerede der)
                // Øverste højre
                hadamardMatrix_[i][j + currentSize] = val;
                // Nederste venstre
                hadamardMatrix_[i + currentSize][j] = val;
                // Nederste højre (negeret)
                hadamardMatrix_[i + currentSize][j + currentSize] = -val;
            }
        }
        currentSize *= 2;
    }
}

// Init shuffleHad matrices. Is Zero-initialized to DTCM memory
void initShuffleHadMatrix()
{
    shuffleHad_ = {{
    // shuffleHad 1
    {{
        {{1, 1, 1, 1, 1, 1, 1, 1}},
        {{-1, 1, -1, 1, -1, 1, -1, 1}},
        {{-1, -1, 1, 1, -1, -1, 1, 1}},
        {{1, -1, -1, 1, 1, -1, -1, 1}},
        {{-1, 1, -1, 1, 1, -1, 1, -1}},
        {{1, 1, -1, -1, -1, -1, 1, 1}},
        {{-1, -1, 1, 1, 1, 1, -1, -1}},
        {{1, -1, -1, 1, -1, 1, 1, -1}}
    }},

    // shuffleHad 2
    {{
        {{-1, 1, -1, 1, -1, 1, -1, 1}},
        {{1, 1, -1, -1, -1, -1, 1, 1}},
        {{1, -1, -1, 1, 1, -1, -1, 1}},
        {{-1, 1, -1, 1, 1, -1, 1, -1}},
        {{-1, -1, -1, -1, -1, -1, -1, -1}},
        {{1, 1, -1, -1, 1, 1, -1, -1}},
        {{1, 1, 1, 1, -1, -1, -1, -1}},
        {{-1, 1, 1, -1, 1, -1, -1, 1}}
    }},

    // shuffleHad 3
    {{
        {{-1, 1, -1, 1, 1, -1, 1, -1}},
        {{1, -1, -1, 1, -1, 1, 1, -1}},
        {{1, 1, -1, -1, -1, -1, 1, 1}},
        {{-1, 1, -1, 1, -1, 1, -1, 1}},
        {{1, -1, -1, 1, 1, -1, -1, 1}},
        {{-1, -1, -1, -1, 1, 1, 1, 1}},
        {{1, 1, -1, -1, 1, 1, -1, -1}},
        {{-1, -1, -1, -1, -1, -1, -1, -1}}
    }},

    // shuffleHad 4
    {{
        {{-1, 1, 1, -1, -1, 1, 1, -1}},
        {{1, 1, 1, 1, -1, -1, -1, -1}},
        {{-1, -1, 1, 1, -1, -1, 1, 1}},
        {{1, 1, -1, -1, -1, -1, 1, 1}},
        {{1, -1, 1, -1, 1, -1, 1, -1}},
        {{-1, -1, -1, -1, -1, -1, -1, -1}},
        {{1, -1, -1, 1, -1, 1, 1, -1}},
        {{1, -1, 1, -1, -1, 1, -1, 1}}
    }},

    // shuffleHad 5
    {{
        {{1, 1, -1, -1, -1, -1, 1, 1}},
        {{-1, -1, -1, -1, -1, -1, -1, -1}},
        {{1, 1, 1, 1, -1, -1, -1, -1}},
        {{-1, -1, 1, 1, -1, -1, 1, 1}},
        {{1, -1, 1, -1, 1, -1, 1, -1}},
        {{-1, 1, 1, -1, 1, -1, -1, 1}},
        {{1, -1, 1, -1, -1, 1, -1, 1}},
        {{-1, 1, 1, -1, -1, 1, 1, -1}}
    }}
}};
}
    
// Getter for Hadamard matrix
template<typename scalar, size_t N = 8>
const std::array<std::array<scalar, N>, N>& getMatrix(const std::string& matName)
{
    if (matName == "hadamard")
        return hadamardMatrix_;
    else if (matName == "shuffle1")
        return shuffleHad_[0];
    else if (matName == "shuffle2")
        return shuffleHad_[1];
    else if (matName == "shuffle3")
        return shuffleHad_[2];
    else if (matName == "shuffle4")
        return shuffleHad_[3];
    else if (matName == "shuffle5")
        return shuffleHad_[4];
    else
        throw std::invalid_argument("Unknown matrix. Use: hadamard, shuffle<number1-5>");
}



