#pragma once

#include <string>
#include <vector>

#include "matrixPoint.h"
#include "stepProfit.h"

struct StepOrder {
    std::vector<std::string> actions;
    std::vector<MatrixPoint> points;
    StepProfit profit;

    void addStep(const std::string& action, const MatrixPoint& point,
                 const StepProfit& incomeProfit);

    void clear();

    bool empty();

    bool operator>(const StepOrder& other) const;

    friend std::ostream& operator<<(std::ostream& out,
                                    const StepOrder& stepOrder);
};