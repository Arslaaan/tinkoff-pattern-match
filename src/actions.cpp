#include "actions.h"

StepProfit Action::getScoreAfterSwap(GameModel& gm, int currentRow,
                                     int currentCol, int newRow, int newCol) {
    if (gm.isBoosterAt(currentRow, currentCol) &&
        gm.isBoosterAt(newRow, newCol)) {
        std::set<std::string> types = {gm.getCell(currentRow, currentCol), gm.getCell(newRow, newCol)};
        gm.setCellDirect(currentRow, currentCol, "x");
        gm.setCellDirect(newRow, newCol, "x");
        StepProfit profit;
        if (types == DOUBLE_SUN) {
            gm.explodeDoubleSun(newRow, newCol, profit);
        } else if (types == DOUBLE_SNOW) {
            gm.explodeDoubleSnow(newRow, newCol, profit);
        } else if (types == SUN_AND_SNOW) {
            gm.explodeSnowAndSun(newRow, newCol, profit);
        } else if (types == std::set<std::string>{"~"} || types == std::set<std::string>{"|"} || types == DOUBLE_ROCKET) {
            gm.explodeDoubleRocket(newRow, newCol, profit);
        } else if (std::includes(ROCKET_AND_SNOW.begin(), ROCKET_AND_SNOW.end(), types.begin(), types.end())) {
            bool isVertical = *types.begin() == "|" || *std::next(types.begin()) == "|";
            gm.explodeRocketAndSnow(newRow, newCol, isVertical, profit);
        }
        return profit + gm.updateAndReturnProfit();  // ignore 2 boosters swap because unpredictable
    }
    if (gm.getCell(currentRow, currentCol) == "x" ||
        gm.getCell(newRow, newCol) == "x") {
        return {0, 0, 0, 0};
    }
    gm.swap(currentRow, currentCol, newRow, newCol);
    StepProfit profit;
    gm.explodeIfSingleBooster(currentRow, currentCol, profit,
                        gm.getCell(newRow, newCol));
    gm.explodeIfSingleBooster(newRow, newCol, profit,
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
void Action::calculateProfit(const GameModel& gm, StepOrder& stepOrder) {
    GameModel newGm = gm;
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
            newGm.explodeIfSingleBooster(point.row, point.col, profit);
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