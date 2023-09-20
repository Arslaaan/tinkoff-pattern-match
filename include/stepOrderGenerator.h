#pragma once

#include <cmath>

#include "gameModel.h"
#include "stepOrder.h"

class StepOrderGenerator {
    std::vector<StepOrder> basicActions;
    size_t startSize;

   public:
    StepOrderGenerator();

    void fillUp(const GameModel& gm);

    std::vector<StepOrder> generate(int deep);
};