#include "actions.h"

StepProfit Action::getScoreAfterSwap(GameModel& gm, int currentRow,
                                     int currentCol, int newRow, int newCol) {
    if (gm.isBoosterAt(currentRow, currentCol) &&
        gm.isBoosterAt(newRow, newCol)) {
        return {0, 0, 0, 0};  // ignore 2 boosters swap because unpredictable
    }
    if (gm.getCell(currentRow, currentCol) == "x" ||
        gm.getCell(newRow, newCol) == "x") {
        return {0, 0, 0, 0};
    }
    gm.swap(currentRow, currentCol, newRow, newCol);
    StepProfit profit;
    gm.explodeIfBooster(currentRow, currentCol, profit,
                        gm.getCell(newRow, newCol));
    gm.explodeIfBooster(newRow, newCol, profit,
                        gm.getCell(currentRow, currentCol));
    return profit + gm.updateAndReturnProfit();
}
bool Action::checkSwapProfit(const GameModel& gm, StepOrder& stepOrder,
                             const MatrixPoint& point1,
                             const MatrixPoint& point2) {
    GameModel newGm = gm;
    const StepProfit& score = Action::getScoreAfterSwap(
        newGm, point1.row, point1.col, point2.row, point2.col);
    if (score.exists()) {
        std::string direction = "bottom";
        if (point2.row == point1.row) {
            direction = "right";
        }
        if (stepOrder.empty()) {
            stepOrder.clear();
        }
        stepOrder.addStep("[swap to " + direction + "]",
                          {point1.row, point1.col}, score);
        return true;
    }
    return false;
}

std::vector<StepOrder> Action::checkAllSwapsAndBoosterExplode(
    const GameModel& gm, const StepOrder& baseStep) {
    std::vector<StepOrder> steps;
    for (int i = 0; i < ROW_SIZE - 1; ++i) {
        for (int j = 0; j < COL_SIZE - 1; ++j) {
            StepOrder base1 = baseStep;
            if (checkSwapProfit(gm, base1, {i, j}, {i, j + 1})) {
                steps.push_back(base1);
            }
            StepOrder base2 = baseStep;
            if (checkSwapProfit(gm, base2, {i, j}, {i + 1, j})) {
                steps.push_back(base2);
            }
            StepOrder base3 = baseStep;
            GameModel newGm = gm;
            StepProfit profit;
            newGm.explodeIfBooster(i, j, profit);
            profit = profit + newGm.updateAndReturnProfit();
            if (profit.exists()) {
                base3.addStep("[touch]", {i, j}, profit);
                steps.push_back(base3);
            }
        }
    }
    sort(steps.begin(), steps.end(), std::greater());
    // 3 best results
    return {steps.begin(), steps.begin() + std::min((size_t)3, steps.size())};
}
std::vector<StepOrder> Action::checkAllSwapsAndBoosterExplodeLvl2(
    const GameModel& gm) {
    std::vector<StepOrder> steps;
    for (int i = 0; i < ROW_SIZE - 1; ++i) {
        for (int j = 0; j < COL_SIZE - 1; ++j) {
            GameModel newGm = gm;
            auto profit = getScoreAfterSwap(newGm, i, j, i, j + 1);
            StepOrder rightSwap = {{"[swap right]"}, {{i, j}}, profit};
            if (profit.exists()) {
                const auto& swaps1 =
                    checkAllSwapsAndBoosterExplode(newGm, rightSwap);
                steps.insert(steps.end(), swaps1.begin(), swaps1.end());
            }

            newGm = gm;
            profit = getScoreAfterSwap(newGm, i, j, i + 1, j);
            StepOrder bottomSwap = {{"[swap botttom]"}, {{i, j}}, profit};
            if (profit.exists()) {
                const auto& swaps2 =
                    checkAllSwapsAndBoosterExplode(newGm, bottomSwap);
                steps.insert(steps.end(), swaps2.begin(), swaps2.end());
            }

            newGm = gm;
            profit = {0, 0, 0, 0};
            newGm.explodeIfBooster(i, j, profit);
            profit = profit + newGm.updateAndReturnProfit();
            if (profit.exists()) {
                StepOrder touch = {{"[touch]"}, {{i, j}}, profit};
                const auto& swaps3 =
                    checkAllSwapsAndBoosterExplode(newGm, touch);
                steps.insert(steps.end(), swaps3.begin(), swaps3.end());
            }
        }
    }
    sort(steps.begin(), steps.end(), std::greater());
    // 3 best results
    return {steps.begin(), steps.begin() + std::min((size_t)3, steps.size())};
}
StepProfit Action::hammer(GameModel& gm, int row, int col) {
    gm.setCellDirect(row, col, "x");
    return gm.updateAndReturnProfit();
}
std::vector<StepOrder> Action::checkAllHammer(const GameModel& gm) {
    // is it gain 10 score ?
    std::vector<StepOrder> steps;
    for (int i = 0; i < ROW_SIZE; ++i) {
        for (int j = 0; j < COL_SIZE; ++j) {
            GameModel newGm = gm;
            newGm.setCellDirect(i, j, "x");
            auto profit = newGm.updateAndReturnProfit();
            StepOrder hammerStep = {{"[hammer]"}, {{i, j}}, profit};
            const std::vector<StepOrder>& swaps =
                checkAllSwapsAndBoosterExplode(newGm, hammerStep);
            steps.insert(steps.end(), swaps.begin(), swaps.end());
        }
    }
    sort(steps.begin(), steps.end(), std::greater());
    // 3 best results
    return {steps.begin(), steps.begin() + std::min((size_t)3, steps.size())};
}
std::vector<StepOrder> Action::checkAllSupportHand(const GameModel& gm) {
    std::vector<StepOrder> steps;
    for (int i = 0; i < ROW_SIZE - 1; ++i) {
        for (int j = 0; j < COL_SIZE - 1; ++j) {
            GameModel newGm = gm;
            newGm.swap(i, j, i, j + 1);
            auto profit = newGm.updateAndReturnProfit();
            StepOrder supportHandStep1 = {{"[hand right]"}, {{i, j}}, profit};
            const auto& swaps1 =
                checkAllSwapsAndBoosterExplode(newGm, supportHandStep1);
            steps.insert(steps.end(), swaps1.begin(), swaps1.end());

            newGm = gm;
            newGm.swap(i, j, i + 1, j);
            profit = newGm.updateAndReturnProfit();
            StepOrder supportHandStep2 = {{"[hand bottom]"}, {{i, j}}, profit};
            const auto& swaps2 =
                checkAllSwapsAndBoosterExplode(newGm, supportHandStep2);
            steps.insert(steps.end(), swaps2.begin(), swaps2.end());
        }
    }
    sort(steps.begin(), steps.end(), std::greater());
    // 3 best results
    return {steps.begin(), steps.begin() + std::min((size_t)3, steps.size())};
};