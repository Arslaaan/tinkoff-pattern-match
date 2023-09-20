#pragma once

#include <string>
#include <vector>

#include "constants.h"
#include "matrixPoint.h"
#include "stepProfit.h"

struct StepOrder {
    std::vector<std::string> actions;
    std::vector<MatrixPoint> points;
    StepProfit profit;
    int goldCost;

    StepOrder(std::vector<std::string> actions_,
              std::vector<MatrixPoint> points_, StepProfit profit_);

    double getScorePerGold() const;

    void addStep(const std::string& action, const MatrixPoint& point,
                 const StepProfit& incomeProfit);

    void addStepOrder(const StepOrder& order);

    void clear();

    bool empty();

    bool operator>(const StepOrder& other) const;

    friend std::ostream& operator<<(std::ostream& out,
                                    const StepOrder& stepOrder);

    static int allProfit(const StepProfit& profit) {
        // const are average income score by explode booster
        return profit.score + 55 * profit.rocket + 90 * profit.sun +
               35 * profit.snow;
    }
};