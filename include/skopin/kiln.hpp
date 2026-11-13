#pragma once

#include "skopin/ware.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace skopin {

enum class Cone {
    Cold,
    Bisque,
    Glaze,
};

enum class Door {
    Closed,
    Open,
};

class KilnError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Горн. Снаружи нельзя нарушить правила присваиванием в поле:
// всё, что меняет ночь, проходит через методы.
// Публичные сигнатуры не меняйте. Поля и приватные методы — можно.
class Kiln {
public:
    Kiln();

    [[nodiscard]] Cone cone() const;
    [[nodiscard]] Door door() const;
    [[nodiscard]] int stokes() const;

    // Копия стопки снизу вверх. Индекс 0 поставили раньше всех,
    // он выйдет последним. Верх — back().
    [[nodiscard]] std::vector<Ware> stack() const;

    // Повторное открытие и повторное закрытие — ошибка, не пустое действие.
    // На поливном конусе дверцу не открыть.
    void open();
    void close();

    // Поднимает конус на один: холодный → утильный → поливной.
    // С поливного дальше некуда. Дверца должна быть закрыта.
    // Слишком толстое изделие или утиль на пути к поливе — KilnError,
    // конус и счётчик не меняются.
    void fire();

    // Опускает конус на один: поливной → утильный → холодный.
    // Одним вызовом через ступень не прыгает.
    // Полива, которая ещё не была на поливном конусе, остужаться не даёт.
    void cool();

    // Кладёт наверх свою копию. Исходный Ware после вызова можно менять.
    // Дверца должна быть открыта. На утильном конусе принимается только полива.
    void load(const Ware& ware);

    // Снимает верх стопки, если клеймо совпало.
    // Поливу с утильного конуса не снимают: только с холодного.
    void unload(const std::string& mark);

private:
    // Поля и приватные методы добавляйте здесь.
    // Публичные сигнатуры выше менять нельзя: тесты собираются против них.
    Cone cone_ = Cone::Cold;
    Door door_ = Door::Closed;
    int stokes_ = 0;
    std::vector<Ware> stack_;
    // Для каждого изделия помним, прошло ли оно поливной конус.
    // Новая горячая посадка не наследует обжиг предыдущих изделий.
    std::vector<bool> glazed_;
};

}  // namespace skopin
