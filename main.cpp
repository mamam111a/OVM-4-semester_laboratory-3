#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <functional>
#include <random>
using namespace std;

typedef vector<vector<double>> Matrix;
void GenerateMatrix(Matrix& mat, int size) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, 10.0);

    mat.resize(size, vector<double>(size));
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            mat[i][j] = dis(gen);
}

void DGEMM(const Matrix& A, const Matrix& B, Matrix& C, int sizeA, int sizeB) {
    for (int i = 0; i < sizeA; i++)
        for (int j = 0; j < sizeB; j++)
            for (int k = 0; k < sizeA; k++)
                C[i][j] += A[i][k] * B[k][j];
}

void DGEMM_opt_1(const Matrix& A, const Matrix& B, Matrix& C, int sizeA, int sizeB) {
    Matrix B_T(sizeB, vector<double>(sizeA));
    for (int i = 0; i < sizeA; i++)
        for (int j = 0; j < sizeB; j++)
            B_T[j][i] = B[i][j]; 

    for (int i = 0; i < sizeA; i++)
        for (int j = 0; j < sizeB; j++) {
            double sum = 0.0;
            for (int k = 0; k < sizeA; k++)
                sum += A[i][k] * B_T[j][k]; 
            C[i][j] = sum;
        }
}
void DGEMM_opt_2(const Matrix& A, const Matrix& B, Matrix& C, int sizeA, int sizeB, int blockSize) {
    for (int i = 0; i < sizeA; i += blockSize) {
        for (int j = 0; j < sizeB; j += blockSize) {
            for (int k = 0; k < sizeA; k += blockSize) {
                for (int ii = i; ii < min(i + blockSize, sizeA); ii++)
                    for (int jj = j; jj < min(j + blockSize, sizeB); jj++)
                        for (int kk = k; kk < min(k + blockSize, sizeA); kk++)
                            C[ii][jj] += A[ii][kk] * B[kk][jj];
            }
        }
    }
}

void RunANDMeter(function<void()> func, const string& label, ofstream& outFile, int sizeA, int sizeB) {
    auto start = chrono::high_resolution_clock::now();
    func();  
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    outFile << label << " Время: " << duration.count() << " секунд для " << sizeA << "x" << sizeB << endl;
    cout << label << " Время: " << duration.count() << " секунд для " << sizeA << "x" << sizeB << endl;
}

int main() {
    ofstream outFile("time.csv");
    int sizeA, sizeB;

    cout << "Введите размерности двух квадратных матриц ==>> ";
    cin >> sizeA >> sizeB;

    Matrix A, B, C;
    GenerateMatrix(A, sizeA);
    GenerateMatrix(B, sizeB);
    C.resize(sizeA, vector<double>(sizeB, 0.0)); 

    RunANDMeter([&]() { DGEMM(A, B, C, sizeA, sizeB); }, "DGEMM", outFile, sizeA, sizeB);
    RunANDMeter([&]() { DGEMM_opt_1(A, B, C, sizeA, sizeB); }, "DGEMM_opt_1", outFile, sizeA, sizeB);

    int blockSize = 64;
    RunANDMeter([&]() { DGEMM_opt_2(A, B, C, sizeA, sizeB, blockSize); }, "DGEMM_opt_2", outFile, sizeA, sizeB);
    outFile << endl;
    
    outFile.close();

    return 0;
}
