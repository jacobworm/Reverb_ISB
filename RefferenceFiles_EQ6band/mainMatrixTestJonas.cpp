#include <iostream>
#include "matrix_v1.h"

// n skal være 8 i denne implementation, fordi had*diff matricer ikke bygges dynamisk, men er skrevet manuelt
// ind i størrelse 8x8.
#define n 8

int main()
{
    // Alt i try er beskyttet. Hvis der sker en runtime fejl stopper den og går til catch uden at programmet crasher
    try
    {

        matrix<float> H(n);          // Constructoren laver objektet, sætter og validerer n
        H.createHadamardMatrix();    // Bygger matricen
        H.createShuffleHadMatrix();  // Bygger shuffle * had matricerne

        const auto& had = H.getMatrix("hadamard"); // Henter Hadamard matricen til udskrivning
        const auto& shuf1 = H.getMatrix("shuffle2"); // Henter shuffle1 til multiplication


        // Udskriver matricen
        std::cout << "Hadamard matrix " << n << "x" << n <<"\n";
        for (const auto& row : had)
        {
            for (auto val : row)
                std::cout << val << " ";
            std::cout << "\n";
        }

        std::vector<float> vec = {1, 1, 0.3, 2, -3, 0.5, 0, 8};
        std::vector<float> resVec = H.multiplyMatrixVector(vec, shuf1);


        // Print resultat vektor
        std::cout << "Resultat vektor \n";
        for (int i = 0; i < n; i++)
        {
            std::cout << resVec[i] << "\n";
        }
    }
    catch (const std::exception& e) // Catch thrown exceptions
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }



    return 0;
}