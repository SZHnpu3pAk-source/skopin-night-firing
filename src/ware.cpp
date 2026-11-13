#include "skopin/ware.hpp"

namespace skopin {

std::optional<std::string> validateWare(const Ware& ware) {
    if (ware.mark.empty()) {
        return "empty mark";
    }
    for (char symbol : ware.mark) {
        if (symbol == ' ' || symbol == '\t' || symbol == '\n') {
            return "bad mark";
        }
    }
    if (ware.height_cm <= 0 || ware.height_cm > kKilnHeightCm) {
        return "bad height";
    }
    if (ware.thickness_mm <= 0) {
        return "bad thickness";
    }
    return std::nullopt;
}

bool fitsInKiln(const std::vector<Ware>& stack, const Ware& next) {
    long long height = next.height_cm;
    for (const Ware& ware : stack) {
        height += ware.height_cm;
        height += kShelfCm;
    }
    return height <= kKilnHeightCm;
}

}  // namespace skopin
