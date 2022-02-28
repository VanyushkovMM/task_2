#include <string>
#include <iostream>
#include <utility>
#include <queue>
#include <vector>
#include <mpi.h>

struct Matrix {
    std::vector<int> mtrx;
    int rows, cols;
    Matrix(const std::vector<int>& _mtrx = std::vector<int>(), int _rows = 0, int _cols = 0)
        : mtrx(_mtrx), rows(_rows), cols(_cols) {}

    void print(bool all_mtrx_out = false);
};

std::vector<Matrix> queue_data(std::queue<std::pair<Matrix, Matrix>>* queue, bool info = true);
