#pragma once

#include <type_traits>
#include <vector>
#include <concepts>
#include <cstdint>
#include <string>
#include <iostream>

template<typename scalar>
class matrix
{
    // Compile-time check:
    // Sikrer at kun tilladte datatyper kan bruges som template parameter.
    static_assert(
        std::is_same_v<scalar, int32_t> ||
        std::is_same_v<scalar, float> ||
        std::is_same_v<scalar, double> ||
        std::is_same_v<scalar, uint32_t>,
        "Only int32_t, uint32_t, float, double allowed"
    );

public:
    // Default constructor
    matrix();

    // Konstruktor der sætter dimension n og initialiserer matricer
    // Forventer typisk at n er en potens af 2 (relevant for Hadamard)
    matrix(uint8_t n);

    // Destructor (default – ingen manuel resource management)
    ~matrix() = default;

    // Matrix-vektor multiplikation:
    // Beregner resultatVec = mat * vec - (n x 1) størrelse
    // - vec: reference til inputvektor (skal have længde n)
    // - mat: reference til input matrix
    // - Funktion validerer at vec.size() == n_
    std::vector<scalar> multiplyMatrixVector(const std::vector<scalar>& vec, const std::vector<std::vector<scalar>>& mat);

    // Konstruerer Hadamard matrix (n x n)
    // Kræver at n = 2^k, for vilkårlig k >= 0
    void createHadamardMatrix();
    // Udfyld shuffleHad matricer
    void createShuffleHadMatrix();

    // Getter for Hadamard matrix
    // Returnerer const reference for at undgå kopi
    const std::vector<std::vector<scalar>>& getMatrix(std::string matName) const;

    // Sætter dimension n
    // Returnerer true hvis n er gyldig, ellers false
    bool set_n(uint8_t n);

    // Getter for dimension
    uint8_t get_n();

    void printMatrix(std::vector<std::vector<scalar>> & matrix); // Ikke implementeret

private:
    // Intern repræsentation af matricer
    // 2D vector (row-major struktur)
    std::vector<std::vector<scalar>> hadamardMatrix_; // [row][col]
    
    std::vector<std::vector<std::vector<scalar>>> shuffleHad_; // [matNum][row][col]

    // Dimension af matricerne (n x n)
    uint8_t n_;

    // Flag der angiver om typen er floating point
    // (kan bruges til specialhåndtering af numerik)
    bool is_floating_;
};



// ================= IMPLEMENTATION =================


template<typename scalar>
matrix<scalar>::matrix()
{
    n_ = 8;
}

template<typename scalar>
matrix<scalar>::matrix(uint8_t n)
{
    if (!this->set_n(n))
    {
        throw std::invalid_argument("Invalid n for matrix. n == 2^k for k >= 0.\n");
    }
}

template<typename scalar>
bool matrix<scalar>::set_n(uint8_t n)
{
    uint8_t nDivider = n;

    if (n < 1)
    {
        return false;
    }
    else
    {
        while (nDivider % 2 == 0)
        {
            nDivider /= 2;
        }

        if (nDivider == 1)
        {
            n_ = n;  
            return true;
        }
        else
        {
            return false;
        }
    }
}

template<typename scalar>
uint8_t matrix<scalar>::get_n()
{
    return n_;
}

template<typename scalar>
void matrix<scalar>::createHadamardMatrix()
{
    hadamardMatrix_ = {{static_cast<scalar>(1)}};

    while (hadamardMatrix_.size() < n_)
    {
        size_t size = hadamardMatrix_.size();

        std::vector<std::vector<scalar>> temp(
            2 * size, std::vector<scalar>(2 * size)
        );

        for (size_t i = 0; i < size; i++)
        {
            for (size_t j = 0; j < size; j++)
            {
                scalar val = hadamardMatrix_[i][j];

                temp[i][j] = val;
                temp[i][j + size] = val;
                temp[i + size][j] = val;
                temp[i + size][j + size] = -val;
            }
        }

        hadamardMatrix_ = std::move(temp);
    }
}

template<typename scalar>
void matrix<scalar>::createShuffleHadMatrix()
{

    // Laver 5 matricer, som er shuffle * had
    // De 5 matricer er udregnet i MathCad inden og indtastes blot manuelt i denne funktion
    // Denne metode anbefales ikke af hensyn til ens sindstilstand efter de 320 tegn er indtastet

    // Resize vektorer til 5 (antal matricer) og matrice størrelse (n x n).
    shuffleHad_.resize(5, std::vector<std::vector<scalar>>(n_, std::vector<scalar>(n_)));

    // shuffleHad 1
    // Række 1
    shuffleHad_[0][0][0] = 1;
    shuffleHad_[0][0][1] = -1;
    shuffleHad_[0][0][2] = -1;
    shuffleHad_[0][0][3] = 1;
    shuffleHad_[0][0][4] = -1;
    shuffleHad_[0][0][5] = 1;
    shuffleHad_[0][0][6] = -1;
    shuffleHad_[0][0][7] = 1;

    // Række 2
    shuffleHad_[0][1][0] = 1;
    shuffleHad_[0][1][1] = 1;
    shuffleHad_[0][1][2] = -1;
    shuffleHad_[0][1][3] = -1;
    shuffleHad_[0][1][4] = -1;
    shuffleHad_[0][1][5] = -1;
    shuffleHad_[0][1][6] = -1;
    shuffleHad_[0][1][7] = -1;

    // Række 3
    shuffleHad_[0][2][0] = 1;
    shuffleHad_[0][2][1] = -1;
    shuffleHad_[0][2][2] = 1;
    shuffleHad_[0][2][3] = -1;
    shuffleHad_[0][2][4] = -1;
    shuffleHad_[0][2][5] = 1;
    shuffleHad_[0][2][6] = 1;
    shuffleHad_[0][2][7] = -1;

    // Række 4
    shuffleHad_[0][3][0] = 1;
    shuffleHad_[0][3][1] = 1;
    shuffleHad_[0][3][2] = 1;
    shuffleHad_[0][3][3] = 1;
    shuffleHad_[0][3][4] = -1;
    shuffleHad_[0][3][5] = -1;
    shuffleHad_[0][3][6] = 1;
    shuffleHad_[0][3][7] = 1;

    // Række 5
    shuffleHad_[0][4][0] = 1;
    shuffleHad_[0][4][1] = -1;
    shuffleHad_[0][4][2] = -1;
    shuffleHad_[0][4][3] = 1;
    shuffleHad_[0][4][4] = 1;
    shuffleHad_[0][4][5] = -1;
    shuffleHad_[0][4][6] = 1;
    shuffleHad_[0][4][7] = -1;

    // Række 6
    shuffleHad_[0][5][0] = 1;
    shuffleHad_[0][5][1] = 1;
    shuffleHad_[0][5][2] = -1;
    shuffleHad_[0][5][3] = -1;
    shuffleHad_[0][5][4] = 1;
    shuffleHad_[0][5][5] = 1;
    shuffleHad_[0][5][6] = 1;
    shuffleHad_[0][5][7] = 1;

    // Række 7
    shuffleHad_[0][6][0] = 1;
    shuffleHad_[0][6][1] = -1;
    shuffleHad_[0][6][2] = 1;
    shuffleHad_[0][6][3] = -1;
    shuffleHad_[0][6][4] = 1;
    shuffleHad_[0][6][5] = -1;
    shuffleHad_[0][6][6] = -1;
    shuffleHad_[0][6][7] = 1;

    // Række 8
    shuffleHad_[0][7][0] = 1;
    shuffleHad_[0][7][1] = 1;
    shuffleHad_[0][7][2] = 1;
    shuffleHad_[0][7][3] = 1;
    shuffleHad_[0][7][4] = 1;
    shuffleHad_[0][7][5] = 1;
    shuffleHad_[0][7][6] = -1;
    shuffleHad_[0][7][7] = -1;



    // shuffleHad 2
    // Række 1
    shuffleHad_[1][0][0] = -1;
    shuffleHad_[1][0][1] = -1;
    shuffleHad_[1][0][2] = 1;
    shuffleHad_[1][0][3] = 1;
    shuffleHad_[1][0][4] = 1;
    shuffleHad_[1][0][5] = -1;
    shuffleHad_[1][0][6] = 1;
    shuffleHad_[1][0][7] = -1;

    // Række 2
    shuffleHad_[1][1][0] = -1;
    shuffleHad_[1][1][1] = -1;
    shuffleHad_[1][1][2] = -1;
    shuffleHad_[1][1][3] = 1;
    shuffleHad_[1][1][4] = 1;
    shuffleHad_[1][1][5] = 1;
    shuffleHad_[1][1][6] = -1;
    shuffleHad_[1][1][7] = 1;

    // Række 3
    shuffleHad_[1][2][0] = -1;
    shuffleHad_[1][2][1] = -1;
    shuffleHad_[1][2][2] = 1;
    shuffleHad_[1][2][3] = -1;
    shuffleHad_[1][2][4] = -1;
    shuffleHad_[1][2][5] = 1;
    shuffleHad_[1][2][6] = 1;
    shuffleHad_[1][2][7] = 1;

    // Række 4
    shuffleHad_[1][3][0] = -1;
    shuffleHad_[1][3][1] = -1;
    shuffleHad_[1][3][2] = -1;
    shuffleHad_[1][3][3] = -1;
    shuffleHad_[1][3][4] = -1;
    shuffleHad_[1][3][5] = -1;
    shuffleHad_[1][3][6] = -1;
    shuffleHad_[1][3][7] = -1;

    // Række 5
    shuffleHad_[1][4][0] = 1;
    shuffleHad_[1][4][1] = -1;
    shuffleHad_[1][4][2] = -1;
    shuffleHad_[1][4][3] = 1;
    shuffleHad_[1][4][4] = -1;
    shuffleHad_[1][4][5] = -1;
    shuffleHad_[1][4][6] = 1;
    shuffleHad_[1][4][7] = 1;

    // Række 6
    shuffleHad_[1][5][0] = 1;
    shuffleHad_[1][5][1] = -1;
    shuffleHad_[1][5][2] = 1;
    shuffleHad_[1][5][3] = 1;
    shuffleHad_[1][5][4] = -1;
    shuffleHad_[1][5][5] = 1;
    shuffleHad_[1][5][6] = -1;
    shuffleHad_[1][5][7] = -1;

    // Række 7
    shuffleHad_[1][6][0] = 1;
    shuffleHad_[1][6][1] = -1;
    shuffleHad_[1][6][2] = -1;
    shuffleHad_[1][6][3] = -1;
    shuffleHad_[1][6][4] = 1;
    shuffleHad_[1][6][5] = 1;
    shuffleHad_[1][6][6] = 1;
    shuffleHad_[1][6][7] = -1;

    // Række 8
    shuffleHad_[1][7][0] = 1;
    shuffleHad_[1][7][1] = -1;
    shuffleHad_[1][7][2] = 1;
    shuffleHad_[1][7][3] = -1;
    shuffleHad_[1][7][4] = 1;
    shuffleHad_[1][7][5] = -1;
    shuffleHad_[1][7][6] = -1;
    shuffleHad_[1][7][7] = 1;



    // shuffleHad 3
    // Række 1
    shuffleHad_[2][0][0] = -1;
    shuffleHad_[2][0][1] = -1;
    shuffleHad_[2][0][2] = 1;
    shuffleHad_[2][0][3] = 1;
    shuffleHad_[2][0][4] = -1;
    shuffleHad_[2][0][5] = -1;
    shuffleHad_[2][0][6] = 1;
    shuffleHad_[2][0][7] = 1;

    // Række 2
    shuffleHad_[2][1][0] = 1;
    shuffleHad_[2][1][1] = 1;
    shuffleHad_[2][1][2] = 1;
    shuffleHad_[2][1][3] = 1;
    shuffleHad_[2][1][4] = 1;
    shuffleHad_[2][1][5] = -1;
    shuffleHad_[2][1][6] = 1;
    shuffleHad_[2][1][7] = -1;

    // Række 3
    shuffleHad_[2][2][0] = 1;
    shuffleHad_[2][2][1] = 1;
    shuffleHad_[2][2][2] = -1;
    shuffleHad_[2][2][3] = 1;
    shuffleHad_[2][2][4] = -1;
    shuffleHad_[2][2][5] = -1;
    shuffleHad_[2][2][6] = -1;
    shuffleHad_[2][2][7] = 1;

    // Række 4
    shuffleHad_[2][3][0] = 1;
    shuffleHad_[2][3][1] = 1;
    shuffleHad_[2][3][2] = -1;
    shuffleHad_[2][3][3] = 1;
    shuffleHad_[2][3][4] = 1;
    shuffleHad_[2][3][5] = -1;
    shuffleHad_[2][3][6] = -1;
    shuffleHad_[2][3][7] = -1;

    // Række 5
    shuffleHad_[2][4][0] = 1;
    shuffleHad_[2][4][1] = -1;
    shuffleHad_[2][4][2] = -1;
    shuffleHad_[2][4][3] = -1;
    shuffleHad_[2][4][4] = 1;
    shuffleHad_[2][4][5] = -1;
    shuffleHad_[2][4][6] = 1;
    shuffleHad_[2][4][7] = 1;

    // Række 6
    shuffleHad_[2][5][0] = -1;
    shuffleHad_[2][5][1] = -1;
    shuffleHad_[2][5][2] = -1;
    shuffleHad_[2][5][3] = -1;
    shuffleHad_[2][5][4] = -1;
    shuffleHad_[2][5][5] = -1;
    shuffleHad_[2][5][6] = -1;
    shuffleHad_[2][5][7] = -1;

    // Række 7
    shuffleHad_[2][6][0] = -1;
    shuffleHad_[2][6][1] = -1;
    shuffleHad_[2][6][2] = -1;
    shuffleHad_[2][6][3] = -1;
    shuffleHad_[2][6][4] = -1;
    shuffleHad_[2][6][5] = -1;
    shuffleHad_[2][6][6] = -1;
    shuffleHad_[2][6][7] = 1;

    // Række 8
    shuffleHad_[2][7][0] = 1;
    shuffleHad_[2][7][1] = -1;
    shuffleHad_[2][7][2] = 1;
    shuffleHad_[2][7][3] = -1;
    shuffleHad_[2][7][4] = -1;
    shuffleHad_[2][7][5] = -1;
    shuffleHad_[2][7][6] = -1;
    shuffleHad_[2][7][7] = -1;



    // shuffleHad 4
    // Række 1
    shuffleHad_[3][0][0] = -1;
    shuffleHad_[3][0][1] = 1;
    shuffleHad_[3][0][2] = -1;
    shuffleHad_[3][0][3] = -1;
    shuffleHad_[3][0][4] = 1;
    shuffleHad_[3][0][5] = 1;
    shuffleHad_[3][0][6] = 1;
    shuffleHad_[3][0][7] = 1;

    // Række 2
    shuffleHad_[3][1][0] = 1;
    shuffleHad_[3][1][1] = 1;
    shuffleHad_[3][1][2] = -1;
    shuffleHad_[3][1][3] = -1;
    shuffleHad_[3][1][4] = -1;
    shuffleHad_[3][1][5] = -1;
    shuffleHad_[3][1][6] = -1;
    shuffleHad_[3][1][7] = 1;

    // Række 3
    shuffleHad_[3][2][0] = -1;
    shuffleHad_[3][2][1] = 1;
    shuffleHad_[3][2][2] = 1;
    shuffleHad_[3][2][3] = -1;
    shuffleHad_[3][2][4] = 1;
    shuffleHad_[3][2][5] = -1;
    shuffleHad_[3][2][6] = -1;
    shuffleHad_[3][2][7] = -1;

    // Række 4
    shuffleHad_[3][3][0] = 1;
    shuffleHad_[3][3][1] = 1;
    shuffleHad_[3][3][2] = 1;
    shuffleHad_[3][3][3] = -1;
    shuffleHad_[3][3][4] = -1;
    shuffleHad_[3][3][5] = 1;
    shuffleHad_[3][3][6] = 1;
    shuffleHad_[3][3][7] = -1;

    // Række 5
    shuffleHad_[3][4][0] = 1;
    shuffleHad_[3][4][1] = -1;
    shuffleHad_[3][4][2] = -1;
    shuffleHad_[3][4][3] = -1;
    shuffleHad_[3][4][4] = 1;
    shuffleHad_[3][4][5] = 1;
    shuffleHad_[3][4][6] = 1;
    shuffleHad_[3][4][7] = -1;

    // Række 6
    shuffleHad_[3][5][0] = -1;
    shuffleHad_[3][5][1] = -1;
    shuffleHad_[3][5][2] = -1;
    shuffleHad_[3][5][3] = -1;
    shuffleHad_[3][5][4] = -1;
    shuffleHad_[3][5][5] = 1;
    shuffleHad_[3][5][6] = -1;
    shuffleHad_[3][5][7] = -1;

    // Række 7
    shuffleHad_[3][6][0] = 1;
    shuffleHad_[3][6][1] = -1;
    shuffleHad_[3][6][2] = 1;
    shuffleHad_[3][6][3] = -1;
    shuffleHad_[3][6][4] = 1;
    shuffleHad_[3][6][5] = 1;
    shuffleHad_[3][6][6] = -1;
    shuffleHad_[3][6][7] = 1;

    // Række 8
    shuffleHad_[3][7][0] = -1;
    shuffleHad_[3][7][1] = -1;
    shuffleHad_[3][7][2] = 1;
    shuffleHad_[3][7][3] = -1;
    shuffleHad_[3][7][4] = -1;
    shuffleHad_[3][7][5] = -1;
    shuffleHad_[3][7][6] = 1;
    shuffleHad_[3][7][7] = 1;



    // shuffleHad 5
    // Række 1
    shuffleHad_[4][0][0] = -1;
    shuffleHad_[4][0][1] = 1;
    shuffleHad_[4][0][2] = -1;
    shuffleHad_[4][0][3] = -1;
    shuffleHad_[4][0][4] = 1;
    shuffleHad_[4][0][5] = 1;
    shuffleHad_[4][0][6] = 1;
    shuffleHad_[4][0][7] = -1;

    // Række 2
    shuffleHad_[4][1][0] = 1;
    shuffleHad_[4][1][1] = 1;
    shuffleHad_[4][1][2] = 1;
    shuffleHad_[4][1][3] = 1;
    shuffleHad_[4][1][4] = 1;
    shuffleHad_[4][1][5] = 1;
    shuffleHad_[4][1][6] = 1;
    shuffleHad_[4][1][7] = 1;

    // Række 3
    shuffleHad_[4][2][0] = -1;
    shuffleHad_[4][2][1] = 1;
    shuffleHad_[4][2][2] = 1;
    shuffleHad_[4][2][3] = 1;
    shuffleHad_[4][2][4] = -1;
    shuffleHad_[4][2][5] = -1;
    shuffleHad_[4][2][6] = 1;
    shuffleHad_[4][2][7] = -1;

    // Række 4
    shuffleHad_[4][3][0] = 1;
    shuffleHad_[4][3][1] = 1;
    shuffleHad_[4][3][2] = -1;
    shuffleHad_[4][3][3] = -1;
    shuffleHad_[4][3][4] = -1;
    shuffleHad_[4][3][5] = -1;
    shuffleHad_[4][3][6] = 1;
    shuffleHad_[4][3][7] = 1;

    // Række 5
    shuffleHad_[4][4][0] = -1;
    shuffleHad_[4][4][1] = -1;
    shuffleHad_[4][4][2] = -1;
    shuffleHad_[4][4][3] = 1;
    shuffleHad_[4][4][4] = 1;
    shuffleHad_[4][4][5] = -1;
    shuffleHad_[4][4][6] = 1;
    shuffleHad_[4][4][7] = 1;

    // Række 6
    shuffleHad_[4][5][0] = 1;
    shuffleHad_[4][5][1] = -1;
    shuffleHad_[4][5][2] = 1;
    shuffleHad_[4][5][3] = -1;
    shuffleHad_[4][5][4] = 1;
    shuffleHad_[4][5][5] = -1;
    shuffleHad_[4][5][6] = 1;
    shuffleHad_[4][5][7] = -1;

    // Række 7
    shuffleHad_[4][6][0] = -1;
    shuffleHad_[4][6][1] = -1;
    shuffleHad_[4][6][2] = 1;
    shuffleHad_[4][6][3] = -1;
    shuffleHad_[4][6][4] = -1;
    shuffleHad_[4][6][5] = 1;
    shuffleHad_[4][6][6] = 1;
    shuffleHad_[4][6][7] = 1;

    // Række 8
    shuffleHad_[4][7][0] = 1;
    shuffleHad_[4][7][1] = -1;
    shuffleHad_[4][7][2] = -1;
    shuffleHad_[4][7][3] = 1;
    shuffleHad_[4][7][4] = -1;
    shuffleHad_[4][7][5] = 1;
    shuffleHad_[4][7][6] = 1;
    shuffleHad_[4][7][7] = -1;

}

template<typename scalar>
const std::vector<std::vector<scalar>>& matrix<scalar>::getMatrix(std::string matName) const
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
        throw std::invalid_argument(std::string("Unknown matrix. matName must be all lower caps. \"hardmard\" or \"shuffleHad<x>\"\n"));
}

template<typename scalar>
std::vector<scalar> matrix<scalar>::multiplyMatrixVector(const std::vector<scalar>& vec, const std::vector<std::vector<scalar>>& mat)
{
    
    // Evaluér om vector og matrix er n lang. Ellers kast en exception
    if (vec.size() != n_)
    {
        throw std::invalid_argument(std::string("Vector length must be equal to n, ") +
             std::to_string(n_) + ", but is " + std::to_string(vec.size()));
    }
    else if (mat.size() <= 0) // Evaluér større end 0
    {
        throw std::invalid_argument(std::string("Matrix length must be greater than 0\n"));
    }
    else if ((mat.size() != n_) || (mat[0].size() != n_)) // Evaluér længde på rækker og kolonner
    {
        throw std::invalid_argument(std::string("Matrix length must be equal to n, ") +
            std::to_string(n_) + ", but is " + std::to_string(mat.size()));
    }

    // Lav resultat vektor n_ lang med 0 i alle pladser
    std::vector<scalar> resultVec(n_, scalar(0));
        
    // Dot produktet mellem række i matrix og vektor udregnes
    // Først itereres gennem rækkerne
    for (size_t i = 0; i < n_; i++)
    {

        // For hver række itereres der gennem kolonnerne
        for (size_t j = 0; j < n_; j++)
        {
            // Hver kolonne i en given række for matricen ganges på hver række i vektoren
            // og adderes til sidst i resultatvektoren
            resultVec[i] += mat[i][j] * vec[j];
        }

    }

    return resultVec;
}
