#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include <thread>
#include <iomanip>
#include <conio.h>

int readKeyNonBlocking() {
    if (_kbhit()) return _getch();  //смотрим клавиши в буфере и возвращаем ее код
    return -1;
}

// ==================== Светофор, заслонка и кнопка ====================
enum class LightColor { GREEN, YELLOW, RED };

std::string getColorString(LightColor c) {
    switch (c) {
    case LightColor::GREEN:  return "green";
    case LightColor::YELLOW: return "yellow";
    case LightColor::RED:    return "red";
    }
    return "";
}
// Класс светофора с заслонкой, сохраняющей состояние на жёлтом
class TrafficLight {
private:
    std::string name;
    LightColor color;
    bool flap;// текущее состояние заслонки
    bool buttonPending;

public:
    TrafficLight(const std::string& n, LightColor c)
        : name(n), color(c), buttonPending(false) {
        // Начальное состояние заслонки: открыта только на зелёном
        flap = (c == LightColor::GREEN);
    }
    // Обновление цвета с учётом логики заслонки
    void setColor(LightColor newColor) {
        // Если цвет не изменился — ничего не делаем
        if (newColor == color) return;
        // Меняем состояние заслонки только при переходе на ЗЕЛЁНЫЙ или КРАСНЫЙ
        if (newColor == LightColor::GREEN)      flap = true;
        else if (newColor == LightColor::RED)   flap = false;
        // YELLOW -> заслонка сохраняет предыдущее состояние
        color = newColor;
    }

    LightColor getColor() const { return color; }
    std::string getName() const { return name; }

    bool isFlapOpen() const { return flap; }
    std::string getFlapString() const { return flap ? "o" : "x"; }
//кнопки
    void pressButton() { buttonPending = true; }        //флаг нажатия кнопки
    void releaseButton() { buttonPending = false; }     //светофор загорелся, флаг снимаем
    bool isButtonPressed() const { return buttonPending; }

    std::string getButtonString(int queuePos) const {
        if (!buttonPending) return "-";
        if (queuePos > 0) return "+ (" + std::to_string(queuePos) + ")";
        return "+";
    }
};

// ==================== Состояния ====================
enum class SystemState {
    IDLE,
    GREEN_WAIT,
    YELLOW
};
// ==================== Вывод таблицы ====================
void printTable(const TrafficLight& l1, const TrafficLight& l2, const TrafficLight& l3,int pos1, int pos2, int pos3) {
    system("cls");

    std::cout << std::left
        << std::setw(10) << l1.getName()
        << std::setw(10) << getColorString(l1.getColor())
        << std::setw(5) << l1.getFlapString()
        << std::setw(5) << l1.getButtonString(pos1)
        << "\n";

    std::cout << std::left
        << std::setw(10) << l2.getName()
        << std::setw(10) << getColorString(l2.getColor())
        << std::setw(5) << l2.getFlapString()
        << std::setw(5) << l2.getButtonString(pos2)
        << "\n";

    std::cout << std::left
        << std::setw(10) << l3.getName()
        << std::setw(10) << getColorString(l3.getColor())
        << std::setw(5) << l3.getFlapString()
        << std::setw(5) << l3.getButtonString(pos3)
        << "\n";

    std::cout << "\nPress [s] [c] [g] to open the corresponding flap\n";
}
// ==================== main ====================
int main() {
    std::vector<TrafficLight> lights = {
        TrafficLight("sheep", LightColor::GREEN),
        TrafficLight("cows ", LightColor::RED),
        TrafficLight("goats", LightColor::RED)
    };

    SystemState state = SystemState::IDLE;
    int activeIndex = 0;
    int targetIndex = 0;
    int timer = 0;
    int greenElapsed = 0;
    std::deque<int> queue;

    const int GREEN_TIMER = 15;
    const int YELLOW_TIMER = 5;

    // ---------- позиция светофора в очереди ----------
    auto queuePosition = [&](int idx) -> int {
        for (size_t i = 0; i < queue.size(); ++i)
            if (queue[i] == idx) return static_cast<int>(i) + 1;
        return 0;   // не в очереди
        };
    printTable(lights[0], lights[1], lights[2], queuePosition(static_cast<int>(0)), queuePosition(static_cast<int>(1)), queuePosition(static_cast<int>(2)));

    while (1) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // ---------- Обработчик кнопок ----------
        int key;
        while ((key = readKeyNonBlocking()) != -1) {
            if ((key == 's' || key == 'S') || (key == 'c' || key == 'C') || (key == 'g' || key == 'G')) {
                int idx;
                if (key == 's' || key == 'S') idx = 0;
                else if (key == 'c' || key == 'C') idx = 1;
                else idx = 2;
                if (idx == activeIndex) continue;             // уже активен
                if (!queue.empty() && queue.back() == idx) continue; // не дублировать подряд
                queue.push_back(idx);
                lights[idx].pressButton();                    // <-- фиксируем нажатие
            }
        }

        // ---------- логика состояний ----------
        switch (state) {
        case SystemState::IDLE: {
            greenElapsed++;

            if (!queue.empty()) {
                targetIndex = queue.front();
                queue.pop_front();
                if (targetIndex == activeIndex) break;

                if (greenElapsed >= GREEN_TIMER) {
                    // 15 с прошло — сразу в общий жёлтый
                    lights[activeIndex].setColor(LightColor::YELLOW);   // З → Ж
                    lights[targetIndex].setColor(LightColor::YELLOW);   // К → Ж 
                    timer = YELLOW_TIMER;
                    state = SystemState::YELLOW;
                }
                else {
                    timer = GREEN_TIMER - greenElapsed;
                    state = SystemState::GREEN_WAIT;
                }
            }
            break;
        }

        case SystemState::GREEN_WAIT: {
            timer--;
            greenElapsed++;
            if (timer <= 0) {
                lights[activeIndex].setColor(LightColor::YELLOW);
                lights[targetIndex].setColor(LightColor::YELLOW);
                timer = YELLOW_TIMER;
                state = SystemState::YELLOW;
            }
            break;
        }

        case SystemState::YELLOW: {
            timer--;
            if (timer <= 0) {
                // Одновременно: уходящий → красный, приходящий → зелёный
                lights[activeIndex].setColor(LightColor::RED);
                lights[targetIndex].setColor(LightColor::GREEN);
                lights[targetIndex].releaseButton();
                activeIndex = targetIndex;
                greenElapsed = 0;
                state = SystemState::IDLE;
            }
            break;
        }
        }
        printTable(lights[0], lights[1], lights[2], queuePosition(static_cast<int>(0)), queuePosition(static_cast<int>(1)), queuePosition(static_cast<int>(2)));
    }
    return 0;
}