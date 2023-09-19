#pragma once

#include <string>
#include <vector>

#include "matrixPoint.h"
#include "stepProfit.h"
#include "constants.h"

struct StepOrder {
    std::vector<std::string> actions;
    std::vector<MatrixPoint> points;
    StepProfit profit;

    static int allProfit(const StepProfit& profit) {
        // const are average income score by explode booster
        return profit.score + 55 * profit.rocket + 90 * profit.sun +
               35 * profit.snow;
    }

    void addStep(const std::string& action, const MatrixPoint& point,
                 const StepProfit& incomeProfit);

    void clear();

    bool empty();

    bool operator>(const StepOrder& other) const;

    friend std::ostream& operator<<(std::ostream& out,
                                    const StepOrder& stepOrder);
};