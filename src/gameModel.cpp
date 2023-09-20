#include "gameModel.h"

// #define DEBUG
#ifdef DEBUG
#define DEBUG_MSG(str)                 \
    do {                               \
        std::cout << str << std::endl; \
    } while (false)
#else
#define DEBUG_MSG(str) \
    do {               \
    } while (false)
#endif

void GameModel::notEmptyShiftToBottomInColumn(int col) {
    std::vector<std::string> newColumn(matrix.size(), "x");
    int k = matrix.size() - 1;
    for (int i = matrix.size() - 1; i >= 0; i--) {
        if (matrix[i][col] != "x") {
            newColumn[k] = matrix[i][col];
            k--;
        }
    }
    for (int i = 0; i < matrix.size(); ++i) {
        matrix[i][col] = newColumn[i];
    }
}

void GameModel::collectByTemplate(
    const MatrixPoint& point,
    const std::vector<std::vector<std::string>>& templateFigure,
    StepProfit& profit) {
    int firstNonEmpty;  // first row should contain x
    for (int j = 0; j < templateFigure.at(0).size(); ++j) {
        if (templateFigure[0][j] == "x") {
            firstNonEmpty = j;
            break;
        }
    }
    const std::string value = matrix[point.row][point.col + firstNonEmpty];
    if (value == "s" || value == "o" || value == "|" || value == "~") {
        // boosters cant collect
        return;
    }

    if (value != "x") {
        for (int i = 0; i < templateFigure.size(); ++i) {
            for (int j = 0; j < templateFigure.at(0).size(); ++j) {
                if (!MatrixPoint{point.row + i, point.col + j}.isCorrect()) {
                    return;
                }
                if (templateFigure[i][j] != "-") {
                    if (value != matrix[point.row + i][point.col + j]) {
                        return;
                    }
                }
            }
        }
        for (int i = 0; i < templateFigure.size(); ++i) {
            for (int j = 0; j < templateFigure.at(0).size(); ++j) {
                if (templateFigure[i][j] != "-") {
                    matrix[point.row + i][point.col + j] = templateFigure[i][j];
                    profit.score += 10;
                }
            }
        }
    }
}

void GameModel::enableDebug() {
    // todo debug compile time
        debug = true;
    }

void GameModel::disableDebug() {
        debug = true;
    }

void GameModel::collectByTemplates(
    const std::vector<std::vector<std::vector<std::string>>>& templates,
    StepProfit& profit, int& collectedCounter) {
    for (const auto& tpl : templates) {
        for (int i = 0; i <= ROW_SIZE - tpl.size(); ++i) {
            for (int j = 0; j <= COL_SIZE - tpl.at(0).size(); ++j) {
                StepProfit local;
                collectByTemplate({i, j}, tpl, local);
                if (local.exists()) {
                    profit = profit + local;
                    collectedCounter++;
                }
            }
        }
    }
}

StepProfit GameModel::collectSun() {
    StepProfit profit;
    collectByTemplates(FiguresTemplates::sunTemplates, profit, profit.sun);
    return profit;
}

StepProfit GameModel::collectRockets() {
    StepProfit profit;
    collectByTemplates(FiguresTemplates::rocketTemplates, profit,
                       profit.rocket);
    return profit;
}

StepProfit GameModel::collectSnows() {
    StepProfit profit;
    collectByTemplates(FiguresTemplates::snowTemplates, profit, profit.snow);
    return profit;
}

void GameModel::collectBoosters(StepProfit& stepProfit) {
    stepProfit = stepProfit + collectSun();
    stepProfit = stepProfit + collectRockets();
    stepProfit = stepProfit + collectSnows();
}

void GameModel::collectTriples(StepProfit& stepProfit) {
    int _c = 0;
    collectByTemplates(FiguresTemplates::commonTemplates, stepProfit, _c);
}

GameModel::GameModel() {
    matrix.resize(ROW_SIZE);
    for (auto& row : matrix) {
        row.resize(COL_SIZE);
    }
}

void GameModel::setCell(int row, int col, const std::string& type) {
    matrix[row][col] = FIGURE_MAPPER.at(type);
}

void GameModel::setCellDirect(int row, int col, const std::string& shortName) {
    matrix[row][col] = shortName;
}

const std::string& GameModel::getCell(int row, int col) const {
    return matrix.at(row).at(col);
}

StepProfit GameModel::updateAndReturnProfit() {
    StepProfit sum;
    while (1) {
        StepProfit profit;

        collectBoosters(profit);
        if (debug) {
            std::cout << *this << std::endl;
        }

        for (int j = 0; j < COL_SIZE; ++j) {
            notEmptyShiftToBottomInColumn(j);
        }
        if (debug) {
            std::cout << *this << std::endl;
        }

        if (!profit.exists()) {
            break;
        }

        sum = sum + profit;
    }

    while (1) {
        StepProfit profit;

        collectTriples(profit);
        if (debug) {
            std::cout << *this << std::endl;
        }

        for (int j = 0; j < COL_SIZE; ++j) {
            notEmptyShiftToBottomInColumn(j);
        }
        if (debug) {
            std::cout << *this << std::endl;
        }

        if (!profit.exists()) {
            break;
        }

        sum = sum + profit;
    }
    return sum;
}

void GameModel::explodeIfBooster(int row, int col, StepProfit& profit,
                                 std::string swapCellType) {
    const std::string typeName = matrix[row][col];
    if (isBoosterAt(row, col)) {
        matrix[row][col] = "x";
        if (typeName == FIGURE_MAPPER.at("vrocket")) {
            explodeRocket(row, col, profit, true, swapCellType);
        } else if (typeName == FIGURE_MAPPER.at("grocket")) {
            explodeRocket(row, col, profit, false, swapCellType);
        } else if (typeName == FIGURE_MAPPER.at("sun")) {
            explodeSun(row, col, profit, swapCellType);
        } else if (typeName == FIGURE_MAPPER.at("snow")) {
            explodeSnow(row, col, profit, swapCellType);
        }
        if (profit.score > 0) {
            for (int j = 0; j < COL_SIZE; ++j) {
                notEmptyShiftToBottomInColumn(j);
            }
        }
    }
}

void GameModel::explodeCell(int row, int col, StepProfit& profit,
                            std::string swapCellType) {
    if (MatrixPoint{row, col}.isCorrect() && matrix[row][col] != "x") {
        explodeIfBooster(row, col, profit, swapCellType);
        if (!isBoosterAt(row, col)) {
            setCellDirect(row, col, "x");
            profit.score += 10;
        }
    }
}

void GameModel::explodeRocket(int row, int col, StepProfit& profit,
                              bool isVertical, std::string swapCellType) {
    if (isVertical) {
        for (int i = 0; i < ROW_SIZE; ++i) {
            explodeCell(i, col, profit, swapCellType);
        }
    } else {
        for (int j = 0; j < COL_SIZE; ++j) {
            explodeCell(row, j, profit, swapCellType);
        }
    }
    profit.rocket--;
}

void GameModel::explodeSnow(int row, int col, StepProfit& profit,
                            std::string swapCellType) {
    // todo operator +=
    explodeCell(row + 1, col, profit, swapCellType);
    explodeCell(row - 1, col, profit, swapCellType);
    explodeCell(row, col - 1, profit, swapCellType);
    explodeCell(row, col + 1, profit, swapCellType);
    profit.snow--;
}

void GameModel::explodeSun(int row, int col, StepProfit& profit,
                           std::string swapCellType) {
    std::map<std::string, int> counter;
    if (swapCellType != "") {
        for (int i = 0; i < ROW_SIZE; ++i) {
            for (int j = 0; j < COL_SIZE; ++j) {
                if (matrix[i][j] == swapCellType) {
                    matrix[i][j] = "x";
                    profit.score += 10;
                }
            }
        }
    } else {
        for (int i = 0; i < ROW_SIZE; ++i) {
            for (int j = 0; j < COL_SIZE; ++j) {
                if (!isBoosterAt(i, j) && matrix[i][j] != "x") {
                    counter[matrix[i][j]]++;
                }
            }
        }
        const std::string& frequent =
            std::max_element(std::begin(counter), std::end(counter),
                             [](const std::pair<std::string, int>& p1,
                                const std::pair<std::string, int>& p2) {
                                 return p1.second < p2.second;
                             })
                ->first;
        for (int i = 0; i < ROW_SIZE; ++i) {
            for (int j = 0; j < COL_SIZE; ++j) {
                if (frequent == matrix[i][j]) {
                    matrix[i][j] = "x";
                    profit.score += 10;
                }
            }
        }
    }
    profit.sun--;
}

bool GameModel::isBoosterAt(int row, int col) const {
    const std::string& shortName = matrix[row][col];
    return shortName == FIGURE_MAPPER.at("snow") ||
           shortName == FIGURE_MAPPER.at("sun") ||
           shortName == FIGURE_MAPPER.at("vrocket") ||
           shortName == FIGURE_MAPPER.at("grocket");
}

void GameModel::swap(int row_1, int col_1, int row_2, int col_2) {
    std::string type = matrix[row_1][col_1];
    setCellDirect(row_1, col_1, matrix[row_2][col_2]);
    setCellDirect(row_2, col_2, type);
}

std::ostream& operator<<(std::ostream& out, const GameModel& gm) {
    out << "-------------" << std::endl;
    for (int i = 0; i < gm.matrix.size(); ++i) {
        out << "|";
        for (int j = 0; j < gm.matrix[i].size(); ++j) {
            out << gm.matrix[i][j] << "|";
        }
        out << std::endl;
    }
    out << "-------------";
    return out;
}

GameModel& operator>>(std::istream& in, GameModel& gm) {
    for (int i = 0; i < gm.matrix.size(); ++i) {
        for (int j = 0; j < gm.matrix[i].size(); ++j) {
            in.ignore(1);
            std::string shortName(1, in.get());
            gm.setCellDirect(i, j, shortName);
        }
        in.ignore(1);
    }
    return gm;
}
