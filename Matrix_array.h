// #pragma once

// #include <array>
// #include <type_traits>
// #include <cstdint>
// #include <string>
// #include <stdexcept>

// template<typename scalar, size_t N = 8>
// class Matrix_array
// {
//     /*
//     static_assert(
//         std::is_same_v<scalar, int32_t> ||
//         std::is_same_v<scalar, float> ||
//         std::is_same_v<scalar, double> ||
//         std::is_same_v<scalar, uint32_t>,
//         "Only int32_t, uint32_t, float, double allowed"
//     );*/

// public:
//     // Default constructor
//     Matrix_array(){
//         createHadamardMatrix();
//     };

//     // Matrix-vektor multiplikation
//     std::array<scalar, N> multiplyMatrixVector(
//         const std::array<scalar, N>& vec,
//         const std::array<std::array<scalar, N>, N>& mat) const
//     {
//         // Lav resultat vektor n_ lang med 0 i alle pladser
//         std::array<scalar, N> resultVec{};
        
//         // Dot produktet mellem række i matrix og vektor udregnes
//         // Først itereres gennem rækkerne
//         for (size_t i = 0; i < N; i++)
//         {
//             // For hver række itereres der gennem kolonnerne
//             for (size_t j = 0; j < N; j++)
//             {
//                 // Hver kolonne i en given række for matricen ganges på hver række i vektoren
//                 // og adderes til sidst i resultatvektoren
//                 resultVec[i] += mat[i][j] * vec[j];
//             }
//         }
//         return resultVec;
//     }
//     // Matrix-vektor multiplikation, ShuffleHad
//     std::array<scalar, N> multiplyShuffleHadVector(
//         const std::array<scalar, N>& vec,
//         size_t shuffleHad_Nr) const
//     {
//         //if (shuffleHad_Nr >= shuffleHad_.size()){
//         //    throw std::out_of_range("shuffleHad_Nr out of range (0-4)");
//         //    }
//         return multiplyMatrixVector(vec, shuffleHad_[shuffleHad_Nr]);        
//     }
//     // Matrix-vektor multiplikation, Hadamard
//     std::array<scalar, N> multiplyHadamardVector(
//         const std::array<scalar, N>& vec) const
//     {
//         return multiplyMatrixVector(vec, hadamardMatrix_);
//     }
//     // Konstruerer Hadamard matrix (N x N)
//     void createHadamardMatrix()
//     {
//         hadamardMatrix_[0][0] = scalar(1);

//         size_t currentSize = 1;
//         while (currentSize < N)
//         {
//             // Kopier den eksisterende matrix til øverste venstre kvadrant
//             for (size_t i = 0; i < currentSize; i++)
//             {
//                 for (size_t j = 0; j < currentSize; j++)
//                 {
//                     scalar val = hadamardMatrix_[i][j];
                    
//                     // Øverste venstre (allerede der)
//                     // Øverste højre
//                     hadamardMatrix_[i][j + currentSize] = val;
//                     // Nederste venstre
//                     hadamardMatrix_[i + currentSize][j] = val;
//                     // Nederste højre (negeret)
//                     hadamardMatrix_[i + currentSize][j + currentSize] = -val;
//                 }
//             }
//             currentSize *= 2;
//         }
//     }

//     // Getter for Hadamard matrix
//     const std::array<std::array<scalar, N>, N>& getMatrix(const std::string& matName) const
//     {
//         if (matName == "hadamard")
//             return hadamardMatrix_;
//         else if (matName == "shuffle1")
//             return shuffleHad_[0];
//         else if (matName == "shuffle2")
//             return shuffleHad_[1];
//         else if (matName == "shuffle3")
//             return shuffleHad_[2];
//         else if (matName == "shuffle4")
//             return shuffleHad_[3];
//         else if (matName == "shuffle5")
//             return shuffleHad_[4];
//         else
//             throw std::invalid_argument("Unknown matrix. Use: hadamard, shuffle1-5");
//     }

// private:
//     static constexpr size_t SIZE = N;
//     std::array<std::array<scalar, N>, N> hadamardMatrix_{};
//     std::array<std::array<std::array<scalar, N>, N>, 5> shuffleHad_{{
//         // shuffleHad 1
//         {{
//             {{1, 1, 1, 1, 1, 1, 1, 1}},
//             {{-1, 1, -1, 1, -1, 1, -1, 1}},
//             {{-1, -1, 1, 1, -1, -1, 1, 1}},
//             {{1, -1, -1, 1, 1, -1, -1, 1}},
//             {{-1, 1, -1, 1, 1, -1, 1, -1}},
//             {{1, 1, -1, -1, -1, -1, 1, 1}},
//             {{-1, -1, 1, 1, 1, 1, -1, -1}},
//             {{1, -1, -1, 1, -1, 1, 1, -1}}
//         }},

//         // shuffleHad 2
//         {{
//             {{-1, 1, -1, 1, -1, 1, -1, 1}},
//             {{1, 1, -1, -1, -1, -1, 1, 1}},
//             {{1, -1, -1, 1, 1, -1, -1, 1}},
//             {{-1, 1, -1, 1, 1, -1, 1, -1}},
//             {{-1, -1, -1, -1, -1, -1, -1, -1}},
//             {{1, 1, -1, -1, 1, 1, -1, -1}},
//             {{1, 1, 1, 1, -1, -1, -1, -1}},
//             {{-1, 1, 1, -1, 1, -1, -1, 1}}
//         }},

//         // shuffleHad 3
//         {{
//             {{-1, 1, -1, 1, 1, -1, 1, -1}},
//             {{1, -1, -1, 1, -1, 1, 1, -1}},
//             {{1, 1, -1, -1, -1, -1, 1, 1}},
//             {{-1, 1, -1, 1, -1, 1, -1, 1}},
//             {{1, -1, -1, 1, 1, -1, -1, 1}},
//             {{-1, -1, -1, -1, 1, 1, 1, 1}},
//             {{1, 1, -1, -1, 1, 1, -1, -1}},
//             {{-1, -1, -1, -1, -1, -1, -1, -1}}
//         }},

//         // shuffleHad 4
//         {{
//             {{-1, 1, 1, -1, -1, 1, 1, -1}},
//             {{1, 1, 1, 1, -1, -1, -1, -1}},
//             {{-1, -1, 1, 1, -1, -1, 1, 1}},
//             {{1, 1, -1, -1, -1, -1, 1, 1}},
//             {{1, -1, 1, -1, 1, -1, 1, -1}},
//             {{-1, -1, -1, -1, -1, -1, -1, -1}},
//             {{1, -1, -1, 1, -1, 1, 1, -1}},
//             {{1, -1, 1, -1, -1, 1, -1, 1}}
//         }},

//         // shuffleHad 5
//         {{
//             {{1, 1, -1, -1, -1, -1, 1, 1}},
//             {{-1, -1, -1, -1, -1, -1, -1, -1}},
//             {{1, 1, 1, 1, -1, -1, -1, -1}},
//             {{-1, -1, 1, 1, -1, -1, 1, 1}},
//             {{1, -1, 1, -1, 1, -1, 1, -1}},
//             {{-1, 1, 1, -1, 1, -1, -1, 1}},
//             {{1, -1, 1, -1, -1, 1, -1, 1}},
//             {{-1, 1, 1, -1, -1, 1, 1, -1}}
//         }}
//     }};
// };