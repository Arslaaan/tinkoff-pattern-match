#pragma once

#include "gameModel.h"
#include "stepOrder.h"
#include "stepProfit.h"

namespace Action {

StepProfit getScoreAfterSwap(GameModel& gm, int currentRow, int currentCol,
                             int newRow, int newCol);

// check only bottom and right because its enough
bool checkSwapProfit(const GameModel& gm, StepOrder& stepOrder,
                     const MatrixPoint& point1, const MatrixPoint& point2);

std::vector<StepOrder> checkAllSwapsAndBoosterExplode(
    const GameModel& gm, const StepOrder& baseStep = {{}});

std::vector<StepOrder> checkAllSwapsAndBoosterExplodeLvl2(const GameModel& gm);

StepProfit hammer(GameModel& gm, int row, int col);

std::vector<StepOrder> checkAllHammer(const GameModel& gm);

std::vector<StepOrder> checkAllSupportHand(const GameModel& gm);
};  // namespace Action