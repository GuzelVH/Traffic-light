#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <iomanip>
#include <atomic>
#include <csignal>

// Определяем цвет по времени внутри цикла (0..39)
std::string getColorByPhase(int t) {
    if (t < 15)       return "GREEN";   
    else if (t < 20)  return "YELLOW";    
    else if (t < 35)  return "RED";   
    else              return "YELLOW";    
}

void printTable(int t1, int t2) {
    system("cls");

    std::cout << std::left << std::setw(10) << "sheep" 
        << std::setw(20) << getColorByPhase(t1) << "\n";
    std::cout << std::left << std::setw(10) << "cows" 
        << std::setw(20) << getColorByPhase(t2) << "\n";
}

int main() {

    const int CYCLE = 40;          // полный цикл светофора
    const int OFFSET = 20;         // сдвиг фаз между светофорами (полцикла)

    int t1 = 0;                    
    int t2 = OFFSET;               

    printTable(t1, t2);

    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        t1 = (t1 + 1) % CYCLE;
        t2 = (t2 + 1) % CYCLE;

        printTable(t1, t2);
    }

    return 0;
}