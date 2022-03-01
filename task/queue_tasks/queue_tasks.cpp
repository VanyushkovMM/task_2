#include "queue_tasks.h"

#define ROOT 0
#define NO_SOURCE 0
#define WAIT_SOURCE 1
#define HAVE_SOURCE 2
#define SEND_SOURCE 3
#define SOURCE_DONE 4
#define MAX_MTRX_SIZE_OUT 10

void Matrix::print(bool all_mtrx_out) {
    if (mtrx.empty()) return;
    int max_mtrx_size = (all_mtrx_out ? 0x7FFFFFFF : MAX_MTRX_SIZE_OUT);
    int r = std::min(rows, max_mtrx_size);
    int c = std::min(cols, max_mtrx_size);
    int pos = 0;
    if (rows > r || cols > c) std::cout << "Size: " << rows << " x " << cols << '\n';
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++)
            std::cout << mtrx[pos + j] << ' ';
        if (rows > r) std::cout << "...";
        std::cout << '\n';
        pos += cols;
    }
    if (cols > c) std::cout << "...\n";
    std::cout << '\n';
}

Matrix matrix_mul(const Matrix& m1, const Matrix& m2) {
	Matrix res;
    if (m1.cols != m2.rows) throw "Error";
    res.rows = m1.rows;
    res.cols = m2.cols;
    res.mtrx.resize(res.rows * res.cols);
    int* a = const_cast<int*>(m1.mtrx.data());
    int* b, *b_head = const_cast<int*>(m2.mtrx.data());
    int* r = res.mtrx.data();
    for (int i = 0; i < res.rows; i++) {
        for (int j = 0; j < res.cols; r[j++] = 0) {}
        b = b_head;
        for (int k = 0; k < m1.cols; k++) {
            for (int j = 0; j < res.cols; j++) 
                r[j] += a[k] * b[j];
            b += res.cols;
        }
        a += m1.cols;
        r += res.cols;
    }
    return res;
}

void send_mtrx(const Matrix& m, int id) {
    MPI_Send(&m.rows, 1, MPI_INT, id, SEND_SOURCE, MPI_COMM_WORLD);
    MPI_Send(&m.cols, 1, MPI_INT, id, SEND_SOURCE, MPI_COMM_WORLD);
    MPI_Send(m.mtrx.data(), static_cast<int>(m.mtrx.size()), MPI_INT, id, SEND_SOURCE, MPI_COMM_WORLD);
}

Matrix recv_mtrx(int id) {
	Matrix m;
    MPI_Recv(&(m.rows), 1, MPI_INT, id, SEND_SOURCE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&(m.cols), 1, MPI_INT, id, SEND_SOURCE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    m.mtrx.resize(m.rows * m.cols);
    MPI_Recv(m.mtrx.data(), static_cast<int>(m.mtrx.size()), MPI_INT, id, SEND_SOURCE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return m;
}

void print_info(const char* text, int id = 0, int task = 0) {
    printf(text, id, task);
    printf("\n");
    fflush(stdout);
}

std::vector<Matrix> root(std::queue<std::pair<Matrix, Matrix>>* queue, bool info) {
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    std::vector<Matrix> res(queue->size());
    std::vector<int> rank_task(size);
    int i = 0;
    MPI_Status status;
    while (true) {
        if (size == 1) {
            if (info) print_info("ROOT end");
            break;
        }
        MPI_Recv(nullptr, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int id = status.MPI_SOURCE;
        if (status.MPI_TAG == SOURCE_DONE) {
            if (info) print_info("ROOT get result from rank %d TASK %d", id, rank_task[id]);
            res[rank_task[id]] = recv_mtrx(id);
            // res[i++] = recv_mtrx(id);
        } else {
            if (queue->empty()) {
                if (info) print_info("ROOT inform rank %d about end", id);
                MPI_Send(nullptr, 0, MPI_INT, id, NO_SOURCE, MPI_COMM_WORLD);
                size--;
            } else {
                if (info) print_info("ROOT set data to rank %d TASK %d", id, i);
                std::pair<Matrix, Matrix> data = queue->front();
                queue->pop();
                MPI_Send(nullptr, 0, MPI_INT, id, HAVE_SOURCE, MPI_COMM_WORLD);
                send_mtrx(data.first, id);
                send_mtrx(data.second, id);
                rank_task[id] = i++;
            }
        }
    }
    return res;
}

void worker(bool info) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;
    while (true) {
        MPI_Send(nullptr, 0, MPI_INT, ROOT, WAIT_SOURCE, MPI_COMM_WORLD);
        MPI_Recv(nullptr, 0, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == NO_SOURCE) {
            // if (info) print_info("Rank %d end", rank);
            break;
        }
        // if (info) print_info("Rank %d get data from root", rank);
        Matrix m1 = recv_mtrx(ROOT);
        Matrix m2 = recv_mtrx(ROOT);
        MPI_Send(nullptr, 0, MPI_INT, ROOT, SOURCE_DONE, MPI_COMM_WORLD);
        Matrix m = matrix_mul(m1, m2);
        // if (info) print_info("Rank %d set result to root", rank);
        send_mtrx(m, ROOT);
    }
}

std::vector<Matrix> queue_data(std::queue<std::pair<Matrix, Matrix>>* queue, bool info) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<Matrix> res;
    if (size == 1) {
        if (info) std::cout << "One thread:\n";
        res.resize(queue->size());
        for (int i = 0; !queue->empty(); i++) {
            std::pair<Matrix, Matrix> data = queue->front();
            queue->pop();
            res[i] = matrix_mul(data.first, data.second);
        }
    } else {
        if (rank == 0) {
            res = root(queue, info);
        } else {
            worker(info);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    return res;
}
