#include "stepOrder.h"

void StepOrder::addStep(const std::string& action, const MatrixPoint& point,
                        const StepProfit& incomeProfit) {
    actions.push_back(action);
    points.push_back(point);
    profit = profit + incomeProfit;
}

void StepOrder::clear() {
    actions.clear();
    points.clear();
    profit = {0, 0, 0, 0};
}

bool StepOrder::empty() { return actions.empty(); }

bool StepOrder::operator>(const StepOrder& other) const {
    return profit.score + 55 * profit.rocket + 90 * profit.sun +
               35 * profit.snow >
           other.profit.score + 55 * other.profit.rocket +
               90 * other.profit.sun + 35 * other.profit.snow;
}

std::ostream& operator<<(std::ostream& out, const StepOrder& stepOrder) {
    for (int i = 0; i < stepOrder.actions.size(); ++i) {
        out << stepOrder.actions[i] << " [" << stepOrder.points[i].row + 1
            << ", " << stepOrder.points[i].col + 1 << "] ";
    }
    out << stepOrder.profit;
    return out;
}
