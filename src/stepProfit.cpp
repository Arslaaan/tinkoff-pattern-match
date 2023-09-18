#include "stepProfit.h"

bool StepProfit::exists() const {
    return score > 0 || sun > 0 || snow > 0 || rocket > 0;
}

StepProfit StepProfit::operator+(const StepProfit& profit) {
    return {score + profit.score, sun + profit.sun, snow + profit.snow,
            rocket + profit.rocket};
}

std::ostream& operator<<(std::ostream& out, const StepProfit& profit) {
    out << "score: " << profit.score;
    if (profit.sun != 0) {
        out << ", +sun: " << profit.sun << ",";
    }
    if (profit.snow != 0) {
        out << " +snow: " << profit.snow << ",";
    }
    if (profit.rocket != 0) {
        out << " +rocket: " << profit.rocket;
    }
    return out;
}
