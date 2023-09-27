#include "stepOrder.h"

StepOrder::StepOrder(std::vector<std::string> actions_,
                     std::vector<MatrixPoint> points_, StepProfit profit_)
    : actions(actions_), points(points_), profit(profit_) {
    int cost = 0;
    for (int i = 0; i < actions.size(); ++i) {
        cost += BOOSTER_COST_MAPPER.at(actions[i]);
    }
    goldCost = cost;
}

double StepOrder::getScorePerGold() const { return allProfit(profit) / (1.0 * goldCost); }

void StepOrder::addStep(const std::string& action, const MatrixPoint& point,
                        const StepProfit& incomeProfit) {
    actions.push_back(action);
    points.push_back(point);
    profit = profit + incomeProfit;
    goldCost += BOOSTER_COST_MAPPER.at(action);
}

void StepOrder::addStepOrder(const StepOrder& order) {
    for (int i = 0; i < order.actions.size(); ++i) {
        actions.push_back(order.actions.at(i));
        points.push_back(order.points.at(i));
        goldCost += BOOSTER_COST_MAPPER.at(order.actions.at(i));
        // profit ignore because always zero todo logic separate profit and stepOrder
    }
}

void StepOrder::clear() {
    actions.clear();
    points.clear();
    goldCost = 0;
    profit = {0, 0, 0, 0};
}

bool StepOrder::empty() { return actions.empty(); }

bool StepOrder::operator>(const StepOrder& other) const {
    return getScorePerGold() > other.getScorePerGold();
}

std::ostream& operator<<(std::ostream& out, const StepOrder& stepOrder) {
    for (int i = 0; i < stepOrder.actions.size(); ++i) {
        out << stepOrder.actions[i] << " [" << stepOrder.points[i].row + 1
            << ", " << stepOrder.points[i].col + 1 << "] ";
    }
    out << stepOrder.profit
        << " s/g: " << StepOrder::allProfit(stepOrder.profit) / (1.0 * stepOrder.goldCost);
    return out;
}
