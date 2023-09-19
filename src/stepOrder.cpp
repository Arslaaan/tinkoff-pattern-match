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
    return allProfit(profit) >
           allProfit(other.profit);
}

std::ostream& operator<<(std::ostream& out, const StepOrder& stepOrder) {
    int cost = 0;
    for (int i = 0; i < stepOrder.actions.size(); ++i) {
        out << stepOrder.actions[i] << " [" << stepOrder.points[i].row + 1
            << ", " << stepOrder.points[i].col + 1 << "] ";
        cost += BOOSTER_COST_MAPPER.at(stepOrder.actions[i]);
    }
    out << stepOrder.profit << " s/p: "
        << StepOrder::allProfit(stepOrder.profit) / (1.0) * cost;
    return out;
}
