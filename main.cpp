#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <iomanip>
#include <limits>

std::mutex result_mutex;
double result = 0.0;

// Функция, вычисляющая интеграл методом прямоугольников на заданном интервале
double integrate(double start, double end, int steps) {
    double step_size = (end - start) / steps;
    double sum = 0.0;

    for (int i = 0; i < steps; ++i) {
        double x = start + (i + 0.5) * step_size;

        // Добавим проверку на близость x к 0
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
    
    // Проверяем, содержит ли результат NaN
    if (!std::isnan(partial_result)) {
        result += partial_result;
    }
}

int main() {
    double x;
    int N;
    
    // Ввод данных
    std::wcout << L"Граница: ";
    std::wcin >> x;
    std::wcout << L"Шаги: ";
    std::wcin >> N;

    // Время начала выполнения программы
    auto start_time = std::chrono::steady_clock::now();

    // Количество потоков
    const int num_threads[] = {1, 2, 4, 8, 16, 32};
    std::vector<std::thread> threads;

    // Подсчет интеграла на разном количестве потоков
    for (int num_thread : num_threads) {
        result = 0.0;

        // Создаем потоки
        for (int i = 0; i < num_thread; ++i) {
            double thread_start = i * x / num_thread;
            double thread_end = (i + 1) * x / num_thread;
            threads.emplace_back(integrateThread, thread_start, thread_end, N / num_thread);
        }

        // Ожидаем завершения всех потоков
        for (auto &thread : threads) {
            thread.join();
        }

        // Вывод результата и времени выполнения
        auto end_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_time = end_time - start_time;

        // Проверяем, содержит ли результат NaN
        if (!std::isnan(result)) {
            std::wcout << L"Результат с " << num_thread << L" потоками: " << std::setprecision(10) << result << std::endl;
        } else {
            std::wcout << L"Результат с " << num_thread << L" потоками: Расчет содержит NaN." << std::endl;
        }
        
        std::wcout << L"Время выполнения: " << elapsed_time.count() << L" секунд" << std::endl;

        // Очищаем вектор потоков перед следующей итерацией
        threads.clear();
    }

    return 0;
}
