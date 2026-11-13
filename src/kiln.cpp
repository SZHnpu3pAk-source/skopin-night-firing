#include "skopin/kiln.hpp"

#include <stdexcept>

namespace skopin {

Kiln::Kiln() = default;

Cone Kiln::cone() const {
    return cone_;
}

Door Kiln::door() const {
    return door_;
}

int Kiln::stokes() const {
    return stokes_;
}

std::vector<Ware> Kiln::stack() const {
    return stack_;
}

void Kiln::open() {
    // Повторное открытие и открытие на поливном конусе — KilnError.
    if (door_ == Door::Open || cone_ == Cone::Glaze) {
        throw KilnError("Cannot open the door");
    }
    door_ = Door::Open;
}

void Kiln::close() {
    if (door_ == Door::Closed) {
        throw KilnError("Door is already closed");
    }
    door_ = Door::Closed;
}

void Kiln::fire() {
    // Отказ не тратит такт и не поднимает конус наполовину.
    if (door_ != Door::Closed || cone_ == Cone::Glaze) {
        throw KilnError("Cannot raise the heat");
    }
    for (const Ware& ware : stack_) {
        if (ware.thickness_mm > kCrackThicknessMm) {
            throw KilnError("Ware is too thick");
        }
        if (cone_ == Cone::Bisque && ware.program == Program::Bisque) {
            throw KilnError("Bisque ware cannot reach glaze heat");
        }
    }
    if (cone_ == Cone::Cold) {
        cone_ = Cone::Bisque;
    } else {
        cone_ = Cone::Glaze;
        for (std::size_t index = 0; index < glazed_.size(); ++index) {
            glazed_[index] = true;
        }
    }
    ++stokes_;
}

void Kiln::cool() {
    // Ровно один конус вниз. Полива, не видевшая поливного огня, остыть не даёт.
    if (door_ != Door::Closed || cone_ == Cone::Cold) {
        throw KilnError("Cannot lower the heat");
    }
    if (cone_ == Cone::Bisque) {
        for (std::size_t index = 0; index < stack_.size(); ++index) {
            if (stack_[index].program == Program::Glaze && !glazed_[index]) {
                throw KilnError("Glaze ware has not been fired yet");
            }
        }
        cone_ = Cone::Cold;
    } else {
        cone_ = Cone::Bisque;
    }
    ++stokes_;
}

void Kiln::load(const Ware& ware) {
    // Внутри должна оказаться копия. Дверцу load сам не открывает.
    if (door_ != Door::Open) {
        throw KilnError("Loading needs an open door");
    }
    if (cone_ == Cone::Glaze ||
        (cone_ == Cone::Bisque && ware.program != Program::Glaze)) {
        throw KilnError("Ware cannot be loaded at this heat");
    }
    if (validateWare(ware).has_value()) {
        throw KilnError("Invalid ware");
    }
    for (const Ware& inside : stack_) {
        if (inside.mark == ware.mark) {
            throw KilnError("Duplicate mark");
        }
        if (inside.kind == WareKind::Sagger || ware.kind == WareKind::Sagger) {
            throw KilnError("A sagger must stand alone");
        }
    }
    if (!fitsInKiln(stack_, ware)) {
        throw KilnError("Not enough height");
    }
    stack_.push_back(ware);
    glazed_.push_back(false);
}

void Kiln::unload(const std::string& mark) {
    // Снимается только верх стопки. Поливу с утильного конуса не снимают.
    if (door_ != Door::Open || stack_.empty()) {
        throw KilnError("Unloading needs an open door and nonempty stack");
    }
    if (stack_.back().mark != mark) {
        throw KilnError("Only the top ware can be unloaded");
    }
    if (stack_.back().program == Program::Glaze && cone_ != Cone::Cold) {
        throw KilnError("Glaze ware must be cold");
    }
    stack_.pop_back();
    glazed_.pop_back();
}

}  // namespace skopin
