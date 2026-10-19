#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <iomanip>
#include <atomic>
#include <csignal>

// ==================== Светофор и заслонка ====================
enum class LightColor {
    GREEN,
    YELLOW,
    RED
};

std::string getColorString(LightColor color) {
    switch (color) {
    case LightColor::GREEN:  return "green";
    case LightColor::YELLOW: return "yellow";
    case LightColor::RED:    return "red";
    }
    return "";
}

// Класс светофора с заслонкой, сохраняющей состояние на жёлтом
class TrafficLight{
private:
    std::string name;
    LightColor currentColor;
    bool flap;          // текущее состояние заслонки
    LightColor prevColor;   // предыдущий цвет (для понимания, откуда пришли на жёлтый)

public:
    TrafficLight(const std::string& n, LightColor c)
        : name(n), currentColor(c), prevColor(c) {
        // Начальное состояние заслонки: открыта только на зелёном
        flap = (c == LightColor::GREEN);
    }

    // Обновление цвета с учётом логики заслонки
    void setColor(LightColor newColor) {
        // Если цвет не изменился — ничего не делаем
        if (newColor == currentColor) return;
        // Меняем состояние заслонки только при переходе на ЗЕЛЁНЫЙ или КРАСНЫЙ
        if (newColor == LightColor::GREEN) flap = true;
        else if (newColor == LightColor::RED) flap = false;
        // YELLOW -> заслонка сохраняет предыдущее состояние
        prevColor = currentColor;
        currentColor = newColor;
    }

    bool isFlapOpen() const { return flap; }
    std::string getFlapString() const { return flap ? "o" : "x"; }

    LightColor getCurrentColor() const { return currentColor; }
    std::string getName() const { return name; }
};

// ==================== Расписание по времени цикла ====================
// цикл 0..39
// [0..14]  - ЗЕЛЕНЫЙ
// [15..19] - ЖЕЛТЫЙ  (заслонка сохраняет состояние с зелёного -> ОТКРЫТА)
// [20..34] - КРАСНЫЙ (заслонка ЗАКРЫТА)
// [35..39] - ЖЕЛТЫЙ  (заслонка сохраняет состояние с красного -> ЗАКРЫТА)
LightColor getColorByPhase(int t) {
    if (t < 15)       return LightColor::GREEN;
    else if (t < 20)  return LightColor::YELLOW;
    else if (t < 35)  return LightColor::RED;
    else              return LightColor::YELLOW;
}

// ==================== Вывод таблицы ====================
void printTable(const TrafficLight& l1,
    const TrafficLight& l2) {
    system("cls");

    std::cout << std::left
        << std::setw(10) << l1.getName() 
        << std::setw(10) << getColorString(l1.getCurrentColor()) 
        << std::setw(5) << l1.getFlapString() 
        << "\n";

    std::cout << std::left
        << std::setw(10) << l2.getName() 
        << std::setw(10) << getColorString(l2.getCurrentColor()) 
        << std::setw(5) << l2.getFlapString() 
        << "\n";
}

// ==================== main ====================
int main() {
    const int CYCLE = 40;
    const int OFFSET = 20;

    int t1 = 0;         // светофор 1 стартует с зелёного
    int t2 = OFFSET;    // светофор 2 стартует с красного

    TrafficLight light1("sheep", getColorByPhase(t1));
    TrafficLight light2("cows ", getColorByPhase(t2));

    printTable(light1, light2);

    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        t1 = (t1 + 1) % CYCLE;
        t2 = (t2 + 1) % CYCLE;

        light1.setColor(getColorByPhase(t1));
        light2.setColor(getColorByPhase(t2));

        printTable(light1, light2);
    }

    return 0;
}