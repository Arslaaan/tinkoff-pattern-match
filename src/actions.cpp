#include "actions.h"

StepProfit Action::getScoreAfterSwap(GameModel& gm, int currentRow,
                                     int currentCol, int newRow, int newCol,
                                     bool debug) {
    if (debug) {
        gm.enableDebug();
    }
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
    if (!profit.exists()) {
        return gm.updateAndReturnProfit();
    }
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
        stepOrder.addStep("[swap " + direction + "]", {point1.row, point1.col},
                          score);
        return true;
    }
    return false;
}
void Action::calculateProfit(const GameModel& gm, StepOrder& stepOrder,
                             bool debug) {
    GameModel newGm = gm;
    if (debug) {
        newGm.enableDebug();
    }
    StepProfit sum;
    for (int i = 0; i < stepOrder.actions.size(); i++) {
        const auto& typeAction = stepOrder.actions[i];
        const auto& point = stepOrder.points[i];
        StepProfit profit;
        if (typeAction == "[touch]") {
            if (!newGm.isBoosterAt(point.row, point.col)) {
                stepOrder.profit = {0, 0, 0, 0};
                return;
            }
            newGm.explodeIfBooster(point.row, point.col, profit);
            profit = profit + newGm.updateAndReturnProfit();
        } else if (typeAction == "[swap right]") {
            profit = getScoreAfterSwap(newGm, point.row, point.col, point.row,
                                       point.col + 1);
            if (!profit.exists()) {
                stepOrder.profit = {0, 0, 0, 0};
                return;
            }
        } else if (typeAction == "[swap bottom]") {
            profit = getScoreAfterSwap(newGm, point.row, point.col,
                                       point.row + 1, point.col);
            if (!profit.exists()) {
                stepOrder.profit = {0, 0, 0, 0};
                return;
            }
        } else if (typeAction == "[hammer]") {
            profit = hammer(newGm, point.row, point.col);
        } else if (typeAction == "[hand right]") {
            // swap without booster boom
            newGm.swap(point.row, point.col, point.row, point.col + 1);
            profit = newGm.updateAndReturnProfit();
        } else if (typeAction == "[hand bottom]") {
            // swap without booster boom
            newGm.swap(point.row, point.col, point.row + 1, point.col);
            profit = newGm.updateAndReturnProfit();
        } else if (typeAction == "[hboom]") {
            newGm.explodeRocket(i, 0, profit, false, "");
            profit = profit + newGm.updateAndReturnProfit();
        } else {
            throw new std::invalid_argument("no action: " + typeAction);
        }
        sum = sum + profit;
    }
    stepOrder.profit = sum;
}
StepProfit Action::hammer(GameModel& gm, int row, int col) {
    gm.setCellDirect(row, col, "x");
    return gm.updateAndReturnProfit();
}