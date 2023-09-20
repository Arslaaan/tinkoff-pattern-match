#pragma once
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "constants.h"
#include "matrixPoint.h"
#include "stepProfit.h"

class GameModel {
    std::vector<std::vector<std::string>> matrix;
    bool debug = false;

    void notEmptyShiftToBottomInColumn(int col);

    void collectByTemplate(
        const MatrixPoint& point,
        const std::vector<std::vector<std::string>>& templateFigure,
        StepProfit& profit);

    void collectByTemplates(
        const std::vector<std::vector<std::vector<std::string>>>& templates,
        StepProfit& profit, int& collectedCounter);

    StepProfit collectSun();

    StepProfit collectRockets();

    StepProfit collectSnows();

    void collectBoosters(StepProfit& stepProfit);

    void collectTriples(StepProfit& stepProfit);

   public:
    static const std::map<std::string, std::string> mapper;
    GameModel();

    void setCell(int row, int col, const std::string& type);

    void setCellDirect(int row, int col, const std::string& shortName);

    const std::string& getCell(int row, int col) const;

    StepProfit updateAndReturnProfit();

    void explodeIfBooster(int row, int col, StepProfit& profit,
                          std::string swapCellType = "");

    void explodeCell(int row, int col, StepProfit& profit,
                     std::string swapCellType);

    void explodeRocket(int row, int col, StepProfit& profit, bool isVertical,
                       std::string swapCellType);

    void explodeSnow(int row, int col, StepProfit& profit,
                     std::string swapCellType);

    void explodeSun(int row, int col, StepProfit& profit,
                    std::string swapCellType);

    bool isBoosterAt(int row, int col) const;

    void swap(int row_1, int col_1, int row_2, int col_2);

    friend std::ostream& operator<<(std::ostream& out, const GameModel& gm);

    friend GameModel& operator>>(std::istream& in, GameModel& gm);

    void enableDebug();

    void disableDebug();
};