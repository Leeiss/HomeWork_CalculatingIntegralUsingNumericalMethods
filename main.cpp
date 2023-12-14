#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex> // для предотвращения одновременного потока к общим данным 
#include <iomanip>
#include <limits>

std::mutex result_mutex;
double result = 0.0;

double integrate(double start, double end, int steps) {
    double step_size = (end - start) / steps;
    double sum = 0.0;

    for (int i = 0; i < steps; ++i) {
        double x = start + (i + 0.5) * step_size;

        if (std::abs(x) > 1e-10) {
            sum += (std::cos(x) - 1) / x;
        }
    }

    return sum * step_size;
}

// Функция, выполняемая в каждом потоке
void integrateThread(double start, double end, int steps) {
    double partial_result = integrate(start, end, steps);

    // Защищаем доступ к общей переменной result с использованием мьютекса
    std::lock_guard<std::mutex> lock(result_mutex);

    if (!std::isnan(partial_result)) {
        result += partial_result;
    }
}

int main() {
    double x;
    int N;

    std::cout << "Граница: ";
    std::cin >> x;
    std::cout << "Шаги: ";
    std::cin >> N;

    auto start_time = std::chrono::steady_clock::now();

    const int num_threads[] = { 1, 2, 4, 8, 16, 32 };
    std::vector<std::thread> threads;

    for (int num_thread : num_threads) {
        result = 0.0;

        for (int i = 0; i < num_thread; ++i) {
            double thread_start = i * x / num_thread;
            double thread_end = (i + 1) * x / num_thread;
            threads.emplace_back(integrateThread, thread_start, thread_end, N / num_thread);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_time = end_time - start_time;

        std::cout << "Результат с " << num_thread << " потоками: " << std::setprecision(10) << result << std::endl;
        std::cout << "Время выполнения: " << elapsed_time.count() << " секунд" << std::endl;

        threads.clear();
    }

    return 0;
}
