#include <random>
#include "queue_tasks.h"

std::vector<int> gen_random_vec(int rows, int cols, int mod = 10) {
    std::random_device dev;
    std::mt19937 rng(dev());

    int size = rows * cols;
    std::vector<int> v(size);
    for (int i = 0; i < size; i++) v[i] = rng() % mod;
    return v;
}

int main(int argc, char const *argv[]) {
    MPI_Init(nullptr, nullptr);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::queue<std::pair<Matrix, Matrix>> queue;
    if (rank == 0) {
        int size;
        std::cout << "Enter tasks count: ";
        std::cin >> size;
        for (int i = 0; i < size; i++) {
            int rows, cols_rows, cols;
            std::cout << "Enter count rows, cols_rows and cols to matrixs: ";
            std::cin >> rows >> cols_rows >> cols;
            Matrix a(gen_random_vec(rows, cols_rows), rows, cols_rows);
            Matrix b(gen_random_vec(cols_rows, cols), cols_rows, cols);
            queue.push(std::pair<Matrix, Matrix>(a, b));
        }
    }
    std::vector<Matrix> res = queue_data(&queue);
    if (rank == 0) {
        std::cout << "Results:\n";
        int size = static_cast<int>(res.size());
        for (int i = 0; i < size; i++) {
            std::cout << "result " << i + 1 << ":\n";
            res[i].print();
        }
    }
    MPI_Finalize();
    return 0;
}
