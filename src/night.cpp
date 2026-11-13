#include "skopin/night.hpp"

#include "skopin/kiln.hpp"

#include <algorithm>

namespace skopin {

NightReport runNightFire(const std::vector<Ware>& queue) {
    // 1. Слева направо вычеркнуть duplicate, затем invalid, затем thick.
    //    Вычеркнутые не стоят в очереди и не рвут садку.
    // 2. Пока кто-то ждёт, собрать одну садку по правилам README.
    //    На стопку сажать с конца к началу: первое взятое снимается первым.
    // 3. Провести её объектом Kiln. После утиля, за которым сразу полива,
    //    горн не остужать. stokes взять из kiln.stokes() в конце.
    NightReport report;
    Kiln kiln;
    std::vector<std::string> seen;
    std::vector<Ware> waiting;

    for (const Ware& ware : queue) {
        if (std::find(seen.begin(), seen.end(), ware.mark) != seen.end()) {
            report.refused.push_back({ware.mark, "duplicate"});
            continue;
        }
        // Клеймо занимаем даже у изделия, которое затем вычеркнем.
        seen.push_back(ware.mark);
        if (validateWare(ware).has_value()) {
            report.refused.push_back({ware.mark, "invalid"});
        } else if (ware.thickness_mm > kCrackThicknessMm) {
            report.refused.push_back({ware.mark, "thick"});
        } else {
            waiting.push_back(ware);
        }
    }

    while (!waiting.empty()) {
        std::vector<Ware> batch{waiting.front()};
        const Program program = batch.front().program;
        waiting.erase(waiting.begin());

        if (batch.front().kind != WareKind::Sagger) {
            bool skipped = false;
            std::size_t index = 0;
            while (index < waiting.size()) {
                const Ware& next = waiting[index];
                if (next.program != program || next.kind == WareKind::Sagger) {
                    break;
                }
                if ((!skipped || next.kind == WareKind::Rush) &&
                    fitsInKiln(batch, next)) {
                    batch.push_back(next);
                    waiting.erase(waiting.begin() + index);
                } else {
                    skipped = true;
                    ++index;
                }
            }
        }

        if (program == Program::Bisque) {
            if (kiln.door() == Door::Open) {
                kiln.close();
            }
            if (kiln.cone() == Cone::Bisque) {
                kiln.cool();
            }
        }
        if (kiln.door() == Door::Closed) {
            kiln.open();
        }
        // Первое в партии должно оказаться сверху стопки.
        for (std::size_t index = batch.size(); index > 0; --index) {
            kiln.load(batch[index - 1]);
        }
        kiln.close();

        if (program == Program::Bisque) {
            kiln.fire();
        } else {
            if (kiln.cone() == Cone::Cold) {
                kiln.fire();
            }
            kiln.fire();
            kiln.cool();
            kiln.cool();
        }

        kiln.open();
        for (const Ware& ware : batch) {
            kiln.unload(ware.mark);
            report.passed.push_back(ware.mark);
        }

        if (program == Program::Bisque) {
            // Смотрим на первую оставшуюся запись, а не на исходную очередь.
            const bool glaze_next = !waiting.empty() &&
                                    waiting.front().program == Program::Glaze;
            if (!glaze_next) {
                kiln.close();
                kiln.cool();
            }
        } else {
            kiln.close();
        }
    }
    report.stokes = kiln.stokes();
    return report;
}

}  // namespace skopin
