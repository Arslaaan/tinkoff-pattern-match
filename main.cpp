#include <iostream>
#include <map>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <set>
#include <vector>

#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;
using namespace std;

const int ROW_SIZE = 7;
const int COL_SIZE = 6;

// #define DEBUG
#define CALIBRATE 0
#define IPHONE 0

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

struct MatrixPoint {
    int row;
    int col;

    bool isCorrect() const {
        return row >= 0 && row < ROW_SIZE && col >= 0 && col < COL_SIZE;
    }
};

struct StepProfit {
    int score = 0;  // score from basic objects
    int sun = 0;    // amount of suns
    int snow = 0;
    int rocket = 0;

    bool exists() const {
        return score > 0 || sun > 0 || snow > 0 || rocket > 0;
    }

    friend ostream& operator<<(ostream& out, const StepProfit& profit) {
        out << "score: " << profit.score;
        if (profit.sun != 0) {
            out << ", +sun: " << profit.sun << ",";
        }
        if (profit.snow != 0) {
            out << " +snow: " << profit.snow << ",";
        }
        if (profit.rocket != 0) {
            out << " +rocket: " << profit.rocket;
        }
        return out;
    }

    StepProfit operator+(const StepProfit& profit) {
        return {score + profit.score, sun + profit.sun, snow + profit.snow,
                rocket + profit.rocket};
    }
};

namespace FiguresTemplates {
static const vector<vector<vector<string>>>& rocketTemplates = {
    {{"~", "x", "x", "x"}},
    {{"x"}, {"x"}, {"x"}, {"|"}},
};
static const vector<vector<vector<string>>>& sunTemplates = {
    {{"-", "x", "-"}, {"-", "x", "-"}, {"x", "o", "x"}},
    {{"-", "-", "x"}, {"-", "-", "x"}, {"x", "x", "o"}},
    {{"x", "-", "-"}, {"x", "-", "-"}, {"o", "x", "x"}},
    {{"x", "o", "x"}, {"-", "x", "-"}, {"-", "x", "-"}},
    {{"o", "x", "x"}, {"x", "-", "-"}, {"x", "-", "-"}},
    {{"x", "x", "o"}, {"-", "-", "x"}, {"-", "-", "x"}},
    {{"x", "-", "-"}, {"o", "x", "x"}, {"x", "-", "-"}},
    {{"-", "-", "x"}, {"x", "x", "o"}, {"-", "-", "x"}},
    {{"o", "x", "x", "x", "x"}},
    {{"x"}, {"x"}, {"x"}, {"x"}, {"o"}},
};
static const vector<vector<vector<string>>>& snowTemplates = {
    {{"x", "x"}, {"s", "x"}},
};
static const vector<vector<vector<string>>>& commonTemplates = {
    {{"x", "x", "x"}},
    {{"x"}, {"x"}, {"x"}},
};
}  // namespace FiguresTemplates

class GameModel {
    vector<vector<string>> matrix;

    string typeToShortName(const string& type) { return mapper.at(type); }

    void notEmptyShiftToBottomInColumn(int col) {
        vector<string> newColumn(matrix.size(), "x");
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

    void collectByTemplate(const MatrixPoint& point,
                           const vector<vector<string>>& templateFigure,
                           StepProfit& profit) {
        int firstNonEmpty;  // first row should contain x
        for (int j = 0; j < templateFigure.at(0).size(); ++j) {
            if (templateFigure[0][j] == "x") {
                firstNonEmpty = j;
                break;
            }
        }
        const string value = matrix[point.row][point.col + firstNonEmpty];
        if (value != "x") {
            for (int i = 0; i < templateFigure.size(); ++i) {
                for (int j = 0; j < templateFigure.at(0).size(); ++j) {
                    if (!MatrixPoint{point.row + i, point.col + j}
                             .isCorrect()) {
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
                        matrix[point.row + i][point.col + j] =
                            templateFigure[i][j];
                        profit.score += 10;
                    }
                }
            }
        }
    }

    void collectByTemplates(const vector<vector<vector<string>>>& templates,
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

    StepProfit collectSun() {
        StepProfit profit;
        collectByTemplates(FiguresTemplates::sunTemplates, profit, profit.sun);
        return profit;
    }

    StepProfit collectRockets() {
        StepProfit profit;
        collectByTemplates(FiguresTemplates::rocketTemplates, profit,
                           profit.rocket);
        return profit;
    }

    StepProfit collectSnows() {
        StepProfit profit;
        collectByTemplates(FiguresTemplates::snowTemplates, profit,
                           profit.snow);
        return profit;
    }

    void collectBoosters(StepProfit& stepProfit) {
        stepProfit = stepProfit + collectSun();
        stepProfit = stepProfit + collectRockets();
        stepProfit = stepProfit + collectSnows();
    }

    void collectTriples(StepProfit& stepProfit) {
        int _c = 0;
        collectByTemplates(FiguresTemplates::commonTemplates, stepProfit, _c);
    }

   public:
    static const map<string, string> mapper;
    GameModel() {
        matrix.resize(ROW_SIZE);
        for (auto& row : matrix) {
            row.resize(COL_SIZE);
        }
    }

    void setCell(int row, int col, const string& type) {
        matrix[row][col] = typeToShortName(type);
    }

    void setCellDirect(int row, int col, const string& shortName) {
        matrix[row][col] = shortName;
    }

    const string& getCell(int row, int col) const { return matrix[row][col]; }

    StepProfit updateAndReturnProfit() {
        StepProfit sum;
        while (1) {
            StepProfit profit;

            collectBoosters(profit);
            DEBUG_MSG(*this);

            for (int j = 0; j < COL_SIZE; ++j) {
                notEmptyShiftToBottomInColumn(j);
            }
            DEBUG_MSG(*this);

            if (!profit.exists()) {
                break;
            }

            sum = sum + profit;
        }

        while (1) {
            StepProfit profit;

            collectTriples(profit);
            DEBUG_MSG(*this);

            for (int j = 0; j < COL_SIZE; ++j) {
                notEmptyShiftToBottomInColumn(j);
            }
            DEBUG_MSG(*this);

            if (!profit.exists()) {
                break;
            }

            sum = sum + profit;
        }
        return sum;
    }

    void explodeIfBooster(int row, int col, StepProfit& profit,
                          string swapCellType = "") {
        const string typeName = matrix[row][col];
        if (isBoosterAt(row, col)) {
            matrix[row][col] = "x";
            if (typeName == mapper.at("vrocket")) {
                explodeRocket(row, col, profit, true, swapCellType);
            } else if (typeName == mapper.at("grocket")) {
                explodeRocket(row, col, profit, false, swapCellType);
            } else if (typeName == mapper.at("sun")) {
                explodeSun(row, col, profit, swapCellType);
            } else if (typeName == mapper.at("snow")) {
                explodeSnow(row, col, profit, swapCellType);
            }
            if (profit.score > 0) {
                for (int j = 0; j < COL_SIZE; ++j) {
                    notEmptyShiftToBottomInColumn(j);
                }
            }
        }
        DEBUG_MSG(*this);
    }

    void explodeCell(int row, int col, StepProfit& profit,
                     string swapCellType) {
        if (MatrixPoint{row, col}.isCorrect() && matrix[row][col] != "x") {
            explodeIfBooster(row, col, profit, swapCellType);
            if (!isBoosterAt(row, col)) {
                setCellDirect(row, col, "x");
                profit.score += 10;
            }
        }
    }

    void explodeRocket(int row, int col, StepProfit& profit, bool isVertical,
                       string swapCellType) {
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

    void explodeSnow(int row, int col, StepProfit& profit,
                     string swapCellType) {
        // todo operator +=
        explodeCell(row + 1, col, profit, swapCellType);
        explodeCell(row - 1, col, profit, swapCellType);
        explodeCell(row, col - 1, profit, swapCellType);
        explodeCell(row, col + 1, profit, swapCellType);
        profit.snow--;
        DEBUG_MSG(*this);
    }

    void explodeSun(int row, int col, StepProfit& profit, string swapCellType) {
        map<string, int> counter;
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
            const string& frequent =
                std::max_element(std::begin(counter), std::end(counter),
                                 [](const pair<string, int>& p1,
                                    const pair<string, int>& p2) {
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

    bool isBoosterAt(int row, int col) const {
        const string& shortName = matrix[row][col];
        // todo refactor call at
        return shortName == mapper.at("snow") ||
               shortName == mapper.at("sun") ||
               shortName == mapper.at("vrocket") ||
               shortName == mapper.at("grocket");
    }

    void swap(int row_1, int col_1, int row_2, int col_2) {
        string type = matrix[row_1][col_1];
        setCellDirect(row_1, col_1, matrix[row_2][col_2]);
        setCellDirect(row_2, col_2, type);
    }

    friend ostream& operator<<(ostream& out, const GameModel& gm) {
        out << "-------------" << endl;
        for (int i = 0; i < gm.matrix.size(); ++i) {
            out << "|";
            for (int j = 0; j < gm.matrix[i].size(); ++j) {
                out << gm.matrix[i][j] << "|";
            }
            out << endl;
        }
        out << "-------------";
        return out;
    }

    friend GameModel& operator>>(istream& in, GameModel& gm) {
        for (int i = 0; i < gm.matrix.size(); ++i) {
            for (int j = 0; j < gm.matrix[i].size(); ++j) {
                in.ignore(1);
                string shortName(1, in.get());
                gm.setCellDirect(i, j, shortName);
            }
            in.ignore(1);
        }
        return gm;
    }
};

class GameObjectImages {
    map<string, Mat> gameObjects;

   public:
    GameObjectImages() {
        vector<string> gameObjectNames = {
            "briefcase", "gold",    "pig",     "pocket", "sandclock",
            "sun",       "vrocket", "grocket", "snow"};  // todo base on mapper
        for (const auto& name : gameObjectNames) {
            Mat gameObject = imread("../" + name + ".png", IMREAD_COLOR);
            if (gameObject.empty()) {
                cout << "Error load " + name << endl;
            }
            Mat gtpl;
            cvtColor(gameObject, gtpl, COLOR_BGR2GRAY);
            Canny(gtpl, gtpl, 110, 330);
            gameObjects[name] = move(gameObject);
        }
    }

    const map<string, Mat>& getImages() const { return gameObjects; }
};

class ScreenReader {
    Mat gameScreen;
    Mat gref;
    const int error = 25;  // pixels

    pair<int, int> detectRowAndCol(int x, int y) {
        // todo can be optimized
        int row = -1, col = -1;
        if (inRange(x, 274)) {
            col = 0;
        } else if (inRange(x, 406)) {
            col = 1;
        } else if (inRange(x, 539)) {
            col = 2;
        } else if (inRange(x, 670)) {
            col = 3;
        } else if (inRange(x, 804)) {
            col = 4;
        } else {
            col = 5;
        }

        if (inRange(y, 885)) {
            row = 0;
        } else if (inRange(y, 1018)) {
            row = 1;
        } else if (inRange(y, 1150)) {
            row = 2;
        } else if (inRange(y, 1281)) {
            row = 3;
        } else if (inRange(y, 1414)) {
            row = 4;
        } else if (inRange(y, 1548)) {
            row = 5;
        } else if (inRange(y, 1681)) {
            row = 6;
        }
        return {row, col};
    }

    pair<int, int> detectRowAndColForIphone(int x, int y) {
        // todo can be optimized
        int row = -1, col = -1;
        if (inRange(x, 250)) {
            col = 0;
        } else if (inRange(x, 391)) {
            col = 1;
        } else if (inRange(x, 529)) {
            col = 2;
        } else if (inRange(x, 668)) {
            col = 3;
        } else if (inRange(x, 810)) {
            col = 4;
        } else {
            col = 5;
        }

        if (inRange(y, 835)) {
            row = 0;
        } else if (inRange(y, 975)) {
            row = 1;
        } else if (inRange(y, 1115)) {
            row = 2;
        } else if (inRange(y, 1255)) {
            row = 3;
        } else if (inRange(y, 1395)) {
            row = 4;
        } else if (inRange(y, 1538)) {
            row = 5;
        } else if (inRange(y, 1676)) {
            row = 6;
        }
        return {row, col};
    }

    inline bool inRange(int value, int prop) {
        return value >= prop - error && value <= prop + error;
    }

   public:
    ScreenReader() {
        gameScreen = imread("../screen.png", IMREAD_COLOR);
        if (gameScreen.empty()) {
            cout << "Error load screen.png" << endl;
        }
        cvtColor(gameScreen, gref, COLOR_BGR2GRAY);
        Canny(gref, gref, 110, 330);
    }

    ScreenReader(string screenshotName) {
        gameScreen = imread("../" + screenshotName, IMREAD_COLOR);
        if (gameScreen.empty()) {
            cout << "Error load screen.png" << endl;
        }
        cvtColor(gameScreen, gref, COLOR_BGR2GRAY);
        Canny(gref, gref, 110, 330);
    }

    GameModel buildModel(const GameObjectImages& gameObjectImages) {
        GameModel gm;
        for (const auto& gameObject : gameObjectImages.getImages()) {
            Mat res_32f(gameScreen.rows - gameObject.second.rows + 1,
                        gameScreen.cols - gameObject.second.cols + 1, CV_32FC1);
            matchTemplate(gameScreen, gameObject.second, res_32f,
                          TM_CCOEFF_NORMED);

            Mat res;
            res_32f.convertTo(res, CV_8U, 255.0);

            int size =
                ((gameObject.second.cols + gameObject.second.rows) / 4) * 2 +
                1;  // force size to be odd
            adaptiveThreshold(res, res, 255, ADAPTIVE_THRESH_MEAN_C,
                              THRESH_BINARY, size, -160);

            double minVal, maxVal = 1;
            Point minLoc, maxLoc;
            while (maxVal > 0) {
                minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
                if (maxVal > 0) {
                    floodFill(res, maxLoc, 0);
                    pair<int, int> position;
                    if (IPHONE) {
                        position = detectRowAndColForIphone(
                            maxLoc.x + gameObject.second.cols,
                            maxLoc.y + gameObject.second.rows);
                    } else {
                        position =
                            detectRowAndCol(maxLoc.x + gameObject.second.cols,
                                            maxLoc.y + gameObject.second.rows);
                    }
                    gm.setCell(position.first, position.second,
                               gameObject.first);
                }
            }
        }
        return gm;
    }
};

int showMatches(const string& screenshotName, const string& objectImageName) {
    // Load images
    Mat gameScreen = imread("../" + screenshotName, IMREAD_COLOR);
    Mat smallImage = imread("../" + objectImageName, IMREAD_COLOR);

    if (gameScreen.empty() || smallImage.empty()) {
        cout << "Error loading images!" << endl;
    }

    Mat gref, gtpl;
    cvtColor(gameScreen, gref, COLOR_BGR2GRAY);
    cvtColor(smallImage, gtpl, COLOR_BGR2GRAY);

    const int low_canny = 110;
    Canny(gref, gref, low_canny, low_canny * 3);
    Canny(gtpl, gtpl, low_canny, low_canny * 3);
    Mat res_32f(gameScreen.rows - smallImage.rows + 1,
                gameScreen.cols - smallImage.cols + 1, CV_32FC1);

    matchTemplate(gameScreen, smallImage, res_32f, TM_CCOEFF_NORMED);

    Mat res;
    res_32f.convertTo(res, CV_8U, 255.0);

    int size = ((smallImage.cols + smallImage.rows) / 4) * 2 +
               1;  // force size to be odd
    adaptiveThreshold(res, res, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,
                      size, -160);

    int resultCounter = 0;
    while (1) {
        double minVal, maxVal;
        Point minLoc, maxLoc;
        minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

        if (maxVal > 0) {
            resultCounter++;
            cout << objectImageName << " "
                 << Point(maxLoc.x + smallImage.cols,
                          maxLoc.y + smallImage.rows)
                 << endl;
            rectangle(
                gameScreen, maxLoc,
                Point(maxLoc.x + smallImage.cols, maxLoc.y + smallImage.rows),
                Scalar(0, 255, 0), 2);
            floodFill(res, maxLoc, 0);
        } else
            break;
    }

    namedWindow("result", WINDOW_NORMAL);
    imshow("result", gameScreen);
    resizeWindow("result", 800, 800);
    waitKey(0);
    return resultCounter;
}

const map<string, string> GameModel::mapper = {
    {"briefcase", "b"}, {"gold", "g"}, {"pig", "p"},     {"pocket", "e"},
    {"sandclock", "8"}, {"sun", "o"},  {"vrocket", "|"}, {"grocket", "~"},
    {"snow", "s"},      {"empty", "x"}};

namespace Action {
struct StepOrder {
    vector<string> actions;
    vector<MatrixPoint> points;
    StepProfit profit;

    void addStep(const string& action, const MatrixPoint& point,
                 const StepProfit& incomeProfit) {
        actions.push_back(action);
        points.push_back(point);
        profit = profit + incomeProfit;
    }

    void clear() {
        actions.clear();
        points.clear();
        profit = {0, 0, 0, 0};
    }

    bool empty() { return actions.empty(); }

    bool operator>(const StepOrder& other) const {
        return profit.score + 55 * profit.rocket + 90 * profit.sun +
                   35 * profit.snow >
               other.profit.score + 55 * other.profit.rocket +
                   90 * other.profit.sun + 35 * other.profit.snow;
    }

    friend ostream& operator<<(ostream& out, const StepOrder& stepOrder) {
        for (int i = 0; i < stepOrder.actions.size(); ++i) {
            out << stepOrder.actions[i] << " [" << stepOrder.points[i].row + 1
                << ", " << stepOrder.points[i].col + 1 << "] ";
        }
        out << stepOrder.profit;
        return out;
    }
};

StepProfit getScoreAfterSwap(GameModel& gm, int currentRow, int currentCol,
                             int newRow, int newCol) {
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
};

// check only bottom and right because its enough
bool checkSwapProfit(const GameModel& gm, StepOrder& stepOrder,
                     const MatrixPoint& point1, const MatrixPoint& point2) {
    GameModel newGm = gm;
    const StepProfit& score = Action::getScoreAfterSwap(
        newGm, point1.row, point1.col, point2.row, point2.col);
    if (score.exists()) {
        string direction = "bottom";
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

vector<StepOrder> checkAllSwapsAndBoosterExplode(const GameModel& gm,
                                                 const StepOrder& baseStep = {
                                                     {}}) {
    vector<StepOrder> steps;
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
    sort(steps.begin(), steps.end(), greater());
    // 3 best results
    return {steps.begin(), steps.begin() + min((size_t)3, steps.size())};
};

vector<StepOrder> checkAllSwapsAndBoosterExplodeLvl2(const GameModel& gm) {
    vector<StepOrder> steps;
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
    sort(steps.begin(), steps.end(), greater());
    // 3 best results
    return {steps.begin(), steps.begin() + min((size_t)3, steps.size())};
};

StepProfit hammer(GameModel& gm, int row, int col) {
    gm.setCellDirect(row, col, "x");
    return gm.updateAndReturnProfit();
}

vector<StepOrder> checkAllHammer(const GameModel& gm) {
    // is it gain 10 score ?
    vector<StepOrder> steps;
    for (int i = 0; i < ROW_SIZE; ++i) {
        for (int j = 0; j < COL_SIZE; ++j) {
            GameModel newGm = gm;
            newGm.setCellDirect(i, j, "x");
            auto profit = newGm.updateAndReturnProfit();
            StepOrder hammerStep = {{"[hammer]"}, {{i, j}}, profit};
            const vector<StepOrder>& swaps =
                checkAllSwapsAndBoosterExplode(newGm, hammerStep);
            steps.insert(steps.end(), swaps.begin(), swaps.end());
        }
    }
    sort(steps.begin(), steps.end(), greater());
    // 3 best results
    return {steps.begin(), steps.begin() + min((size_t)3, steps.size())};
};

vector<StepOrder> checkAllSupportHand(const GameModel& gm) {
    vector<StepOrder> steps;
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
    sort(steps.begin(), steps.end(), greater());
    // 3 best results
    return {steps.begin(), steps.begin() + min((size_t)3, steps.size())};
};
};  // namespace Action

void printSteps(const vector<Action::StepOrder>& steps) {
    for (const auto& order : steps) {
        cout << order << endl;
    }
}

void testShow() {
    showMatches("screen-test.png", "pig.png");
    showMatches("screen-test.png", "pocket.png");
    showMatches("screen-test.png", "gold.png");
    showMatches("screen-test.png", "briefcase.png");
    showMatches("screen-test.png", "sandclock.png");
    showMatches("screen-test.png", "sun.png");
    showMatches("screen-test.png", "snow.png");
    showMatches("screen-test.png", "grocket.png");
    showMatches("screen-test.png", "vrocket.png");
}

void test1() {
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|b|e|e|b|8|b|";
    example << "|g|8|b|8|e|g|";
    example << "|8|b|p|p|b|p|";
    example << "|8|e|g|p|8|g|";
    example << "|e|8|p|8|g|8|";
    example << "|8|g|8|b|e|b|";
    example << "|e|8|o|8|8|g|";
    example >> gm;
    cout << gm << endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 3, 2, 4, 2);
    cout << scores << endl;
    assert(scores.score == 80);
    assert(scores.snow == 1);
    assert(scores.sun == 0);
    assert(scores.rocket == 1);
}

void test2() {
    // todo i dont know, all seems corect
    // error in [hand right] [5, 2] [swap to bottom] [5, 3] score: 110 +snow: 1,
    // +rocket: 1
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    // example << "|b|e|8|b|e|p|";
    // example << "|b|e|e|p|g|b|";
    // example << "|p|g|8|g|8|g|";
    // example << "|g|b|e|b|p|b|";
    // example << "|p|g|8|e|p|g|";
    // example << "|g|8|e|g|e|g|";
    // example << "|e|8|p|e|g|8|";
    // example >> gm;
    // cout << gm << endl;

    // gm.swap(4, 1, 4, 2);
    // auto profit1 = gm.updateAndReturnProfit();
    // gm.swap(4, 2, 5, 2);
    // auto profit2 = gm.updateAndReturnProfit();
    // cout << profit1 + profit2 << endl;
}

void test3() {
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|p|b|g|8|e|p|";
    example << "|e|e|g|8|e|e|";
    example << "|g|g|8|g|~|8|";
    example << "|e|p|g|e|g|b|";
    example << "|p|8|g|g|8|p|";
    example << "|b|8|8|b|p|g|";
    example << "|p|b|g|8|b|b|";
    example >> gm;
    cout << gm << endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 2, 2, 2, 3);
    cout << scores << endl;
    assert(scores.score == 80);
    assert(scores.snow == 0);
    assert(scores.sun == 1);
    assert(scores.rocket == 0);
}

void test4() {
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|e|p|b|e|g|g|";
    example << "|e|8|8|g|e|p|";
    example << "|g|b|~|e|g|8|";
    example << "|p|g|8|p|p|b|";
    example << "|e|||b|e|8|8|";
    example << "|g|p|e|e|p|e|";
    example << "|8|e|b|8|b|b|";
    example >> gm;
    cout << gm << endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 1, 3, 1, 4);
    cout << scores << endl;
    assert(scores.score == 60);
    assert(scores.snow == 0);
    assert(scores.sun == 0);
    assert(scores.rocket == 0);
}

void test5() {
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|p|b|e|b|p|p|";
    example << "|e|e|p|e|p|b|";
    example << "|e|p|b|8|g|g|";
    example << "|g|p|b|s|g|p|";
    example << "|e|b|~|b|p|8|";
    example << "|g|||b|e|p|e|";
    example << "|8|p|b|8|b|b|";
    example >> gm;
    cout << gm << endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 4, 1, 4, 2);
    cout << scores << endl;
    assert(scores.score == 90);
    assert(scores.sun == 0);
    assert(scores.snow == 0);
    assert(scores.rocket == 0);
}

void test6() {
    // 4 6 to left should be sun
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|e|e|b|8|b|e|";
    example << "|b|b|g|g|b|g|";
    example << "|g|o|p|8|e|b|";
    example << "|g|p|e|e|g|e|";
    example << "|8|8|p|8|e|p|";
    example << "|g|b|8|8|b|e|";
    example << "|b|s|b|p|8|g|";
    example >> gm;
    cout << gm << endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 3, 4, 3, 5);
    cout << scores << endl;
    assert(scores.score == 110);
    assert(scores.sun == 1);
    assert(scores.snow == 0);
    assert(scores.rocket == 0);
}

void test7() {
    //  [swap to right] [5, 2] score: 120 +rocket: -1
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|g|e|e|b|g|e|";
    example << "|8|b|p|8|p|8|";
    example << "|b|p|g|g|p|b|";
    example << "|p|p|e|g|e|e|";
    example << "|g|||p|b|g|p|";
    example << "|p|b|~|e|e|b|";
    example << "|e|b|p|e|g|8|";
    example >> gm;
    cout << gm << endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 4, 1, 4, 2);
    cout << scores << endl;
    assert(scores.rocket == -2);
}

void test8() {
    // test new method for collect boosters
    GameObjectImages gameObjectImages;
    {
        GameModel gm;
        stringstream example;
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|x|g|g|g|g|";
        example >> gm;
        cout << gm << endl;

        StepProfit scores = gm.updateAndReturnProfit();
        cout << scores << endl;
        assert(scores.sun == 0);
        assert(scores.snow == 0);
        assert(scores.rocket == 2);
    }

    {
        GameModel gm;
        stringstream example;
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|g|g|g|g|g|";
        example >> gm;
        cout << gm << endl;

        StepProfit scores = gm.updateAndReturnProfit();
        cout << scores << endl;
        assert(scores.sun == 1);
        assert(scores.snow == 0);
        assert(scores.rocket == 0);
    }

    {
        GameModel gm;
        stringstream example;
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|g|g|x|x|g|g|";
        example << "|g|g|x|x|g|g|";
        example >> gm;
        cout << gm << endl;

        StepProfit scores = gm.updateAndReturnProfit();
        cout << scores << endl;
        assert(scores.sun == 0);
        assert(scores.snow == 2);
        assert(scores.rocket == 0);
    }
}

void test9() {
    // todo
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|b|e|p|8|e|8|";
    example << "|g|p|g|8|8|e|";
    example << "|8|b|8|g|8|8|";
    example << "|b|8|g|b|p|b|";
    example << "|p|e|p|8|p|e|";
    example << "|g|e|b|g|e|p|";
    example << "|e|b|g|e|g|8|";
    example >> gm;
    cout << gm << endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 2, 2, 2, 3);
    cout << scores << endl;
    assert(scores.sun == 1);
    assert(scores.snow == 0);
    assert(scores.rocket == 0);
}

void test10() {
    // todo [hand right] [3, 4] [swap to bottom] [3, 5] score: 370 +snow: 2
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|g|b|e|8|e|p|";
    example << "|e|8|g|p|g|g|";
    example << "|p|g|e|g|p|g|";
    example << "|8|g|b|e|b|8|";
    example << "|8|b|8|p|g|e|";
    example << "|e|e|b|e|p|p|";
    example << "|8|8|g|b|g|e|";
    example >> gm;
    cout << gm << endl;

    auto scores = Action::getScoreAfterSwap(gm, 2, 3, 2, 4);
    scores = scores + Action::getScoreAfterSwap(gm, 2, 4, 3, 4);
    cout << scores << endl;
    assert(scores.score == 110);
    assert(scores.sun == 0);
    assert(scores.snow == 0);
    assert(scores.rocket == 0);
}

void test11() {
    // todo [hammer] [4, 2] [swap to right] [3, 1] score: 50, +sun: 1
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|g|p|b|b|8|8|";
    example << "|b|g|p|g|e|g|";
    example << "|p|p|e|8|e|p|";
    example << "|b|g|b|p|s|p|";
    example << "|e|p|e|8|b|b|";
    example << "|p|p|b|~|g|g|";
    example << "|||8|8|b|e|8|";
    example >> gm;
    cout << gm << endl;

    auto scores = Action::hammer(gm, 3, 1);
    scores = scores + Action::getScoreAfterSwap(gm, 2, 0, 2, 1);
    cout << scores << endl;
    assert(scores.sun == 0);  // no sun as result
    // seems like correct
}

void testAll() {
    cout << "Start tests" << endl;
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
    test9();
    test10();
    test11();
    cout << "End tests" << endl;
}

int main() {
    if (CALIBRATE) {
        system("rm ../screen-test.png");
        system(
            "~/platform-tools/adb exec-out screencap -p > ../screen-test.png");
        testShow();
    } else {
        testAll();

        GameObjectImages gameObjectImages;

        while (1) {
            cin.get();
            system("rm ../screen.png");
            system(
                "~/platform-tools/adb exec-out screencap -p > ../screen.png");

            ScreenReader screenReader;
            GameModel gm = screenReader.buildModel(gameObjectImages);
            cout << flush << gm << endl;
            printSteps(Action::checkAllSwapsAndBoosterExplode(gm));
            cout << endl;
            printSteps(Action::checkAllSwapsAndBoosterExplodeLvl2(gm));
            cout << endl;
            printSteps(Action::checkAllHammer(gm));
            cout << endl;
            printSteps(Action::checkAllSupportHand(gm));
        }
    }

    return 0;
}