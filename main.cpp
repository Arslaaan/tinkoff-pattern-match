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
        out << "Score: " << profit.score;
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

    bool checkEquality(const MatrixPoint& point1, const MatrixPoint& point2,
                       const string& standartValue) {
        if (!point1.isCorrect() || !point2.isCorrect()) {
            return false;
        }
        bool isHorizontal = point1.row == point2.row;
        bool isVertical = point1.col == point2.col;
        if (!isHorizontal && !isVertical) {
            return false;
        }
        bool isEqual = true;
        if (isHorizontal) {
            for (int j = point1.col; j <= point2.col; ++j) {
                if (matrix[point1.row][j] != standartValue) {
                    return false;
                }
            }
        }
        if (isVertical) {
            for (int i = point1.row; i <= point2.row; ++i) {
                if (matrix[i][point1.col] != standartValue) {
                    return false;
                }
            }
        }
        return true;
    }

    // replace without check that all values in row or column are equal
    // think that they 100% equal
    void replacePointToPoint(const MatrixPoint& point1,
                             const MatrixPoint& point2,
                             const string& replaceValue, int& resultScore,
                             int& boosterCounter,
                             const MatrixPoint& boosterPoint = {-1, -1},
                             const string& boosterValue = "") {
        if (!point1.isCorrect() || !point2.isCorrect()) {
            return;
        }
        bool isHorizontal = point1.row == point2.row;
        bool isVertical = point1.col == point2.col;
        if (isHorizontal) {
            for (int j = point1.col; j <= point2.col; ++j) {
                matrix[point1.row][j] = replaceValue;
                resultScore += 10;
            }
        } else if (isVertical) {
            for (int i = point1.row; i <= point2.row; ++i) {
                matrix[i][point1.col] = replaceValue;
                resultScore += 10;
            }
        }
        if (isHorizontal || isVertical) {
            if (boosterPoint.isCorrect()) {
                matrix[boosterPoint.row][boosterPoint.col] = boosterValue;
                boosterCounter++;
            }
        }
    }

    void replaceComplexTypeFiveBySun(const MatrixPoint& currentPoint,
                                     const MatrixPoint& point1,
                                     const MatrixPoint& point2,
                                     const MatrixPoint& pointReplacedBySun,
                                     const string& prev, int& resultScore,
                                     int& suns) {
        const auto& beginThree =
            MatrixPoint{currentPoint.row, currentPoint.col - 3};
        const auto& endThree =
            MatrixPoint{currentPoint.row, currentPoint.col - 1};
        if (beginThree.isCorrect() && endThree.isCorrect() &&
            point1.isCorrect() && point2.isCorrect() &&
            matrix[point1.row][point1.col] == prev &&
            matrix[point2.row][point2.col] == prev) {
            replacePointToPoint(beginThree, endThree, "x", resultScore, suns);
            matrix[point1.row][point1.col] = "x";
            matrix[point2.row][point2.col] = "x";
            matrix[pointReplacedBySun.row][pointReplacedBySun.col] =
                mapper.at("sun");
            resultScore += 20;
            suns++;
        }
    }

    void collectRockets(const string& prev, int row, int col, int currentScore,
                        int& resultScore, int& rockets) {
        if (currentScore == 40) {
            if (checkEquality({row, col - 4}, {row, col - 1}, prev)) {
                replacePointToPoint({row, col - 4}, {row, col - 1}, "x",
                                    resultScore, rockets, {row, col - 4}, "~");
            } else {
                replacePointToPoint({row - 4, col}, {row - 1, col}, "x",
                                    resultScore, rockets, {row - 1, col}, "|");
            }
        }
    }

    void collectSun(const string& prev, int row, int col, int currentScore,
                    int& resultScore, int& suns) {
        if (currentScore == 30) {
            // xxxo
            // -x--
            // -x--
            const auto& current = MatrixPoint{row, col};
            replaceComplexTypeFiveBySun(current, {row + 1, col - 2},
                                        {row + 2, col - 2}, {row, col - 2},
                                        prev, resultScore, suns);
            // -x--
            // -x--
            // xxxo
            replaceComplexTypeFiveBySun(current, {row - 1, col - 2},
                                        {row - 2, col - 2}, {row, col - 2},
                                        prev, resultScore, suns);
            // --x-
            // --x-
            // xxxo
            replaceComplexTypeFiveBySun(current, {row - 1, col - 1},
                                        {row - 2, col - 1}, {row, col - 1},
                                        prev, resultScore, suns);
            // x---
            // xxxo
            // x---
            replaceComplexTypeFiveBySun(current, {row - 1, col - 3},
                                        {row + 1, col - 3}, {row, col - 3},
                                        prev, resultScore, suns);
            // --x-
            // xxxo
            // --x-
            replaceComplexTypeFiveBySun(current, {row - 1, col - 1},
                                        {row + 1, col - 1}, {row, col - 1},
                                        prev, resultScore, suns);
            // x---
            // x---
            // xxxo
            replaceComplexTypeFiveBySun(current, {row - 1, col - 3},
                                        {row - 2, col - 3}, {row, col - 3},
                                        prev, resultScore, suns);
            // xxxo
            // x---
            // x---
            replaceComplexTypeFiveBySun(current, {row + 1, col - 3},
                                        {row + 2, col - 3}, {row, col - 3},
                                        prev, resultScore, suns);
            // xxxo
            // --x-
            // --x-
            replaceComplexTypeFiveBySun(current, {row + 1, col - 1},
                                        {row + 2, col - 1}, {row, col - 1},
                                        prev, resultScore, suns);
        } else if (currentScore == 50) {
            if (checkEquality({row, col - 5}, {row, col - 1}, prev)) {
                replacePointToPoint({row, col - 5}, {row, col - 1}, "x",
                                    resultScore, suns, {row, col - 5}, "o");
            } else {
                replacePointToPoint({row - 5, col}, {row - 1, col}, "x",
                                    resultScore, suns, {row - 1, col}, "o");
            }
        }
    }

    void collectSnows(const string& prev, int row, int col, int currentScore,
                      int& resultScore, int& snows) {
        if (currentScore == 20) {
            if (checkEquality({row, col - 2}, {row, col - 1}, prev) &&
                checkEquality({row + 1, col - 2}, {row + 1, col - 1}, prev)) {
                replacePointToPoint({row, col - 2}, {row, col - 1}, "x",
                                    resultScore, snows);
                replacePointToPoint({row + 1, col - 2}, {row + 1, col - 1}, "x",
                                    resultScore, snows, {row + 1, col - 2}, mapper.at("snow"));
            }
        }
    }

    void collectBoosters(const string& prev, int row, int col, int currentScore,
                         StepProfit& stepProfit) {
        if (prev != "x") {
            collectSun(prev, row, col, currentScore, stepProfit.score,
                       stepProfit.sun);
            collectRockets(prev, row, col, currentScore, stepProfit.score,
                           stepProfit.rocket);
            collectSnows(prev, row, col, currentScore, stepProfit.score,
                         stepProfit.snow);
        }
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

    bool isValid() {
        int i = 0;
        int j = 0;
        for (const auto& row : matrix) {
            for (const auto& cell : row) {
                if (cell.size() != 1) {
                    cout << "Error in cell: (" << i << ", " << j << ")" << endl;
                    return false;
                }
                j++;
            }
            i++;
        }
        return true;
    }

    StepProfit updateAndReturnProfit() {
        StepProfit sum;
        StepProfit profit;
        do {
            int i = 0;
            int j = 0;
            profit = {0, 0, 0, 0};
            // check horizontal chains and boosters
            for (i = 0; i < ROW_SIZE; ++i) {
                int startChainCol = COL_SIZE;
                string prev = matrix[i][0];
                int score = prev == "x" ? 0 : 10;
                for (j = 1; j <= COL_SIZE; ++j) {
                    if (j != COL_SIZE && prev != "x" && matrix[i][j] == prev) {
                        score += 10;
                        if (score == 20) {
                            startChainCol = j - 1;
                        }
                    } else {
                        if (score >= 20) {
                            StepProfit localProfit;
                            collectBoosters(prev, i, j, score, localProfit);
                            if (localProfit.exists()) {
                                profit = profit + localProfit;
                            } else if (score == 30) {
                                // no boosters collected
                                const auto& beginThree = MatrixPoint{i, j - 3};
                                const auto& endThree = MatrixPoint{i, j - 1};
                                if (beginThree.isCorrect() &&
                                    endThree.isCorrect()) {
                                    int nothing = -1;  // todo need fix this
                                    replacePointToPoint(beginThree, endThree,
                                                        "x", profit.score,
                                                        nothing);
                                }
                            }
                            if (MatrixPoint{i, j}.isCorrect() &&
                                matrix[i][j] != "x") {
                                score = 10;
                            }
                        }
                        if (j < COL_SIZE) {
                            prev = matrix[i][j];
                        }
                    }
                }
            }
            // check vertical chains and boosters
            for (j = 0; j < COL_SIZE; ++j) {
                int startChainRow = ROW_SIZE;
                string prev = matrix[0][j];
                int score = prev == "x" ? 0 : 10;
                for (i = 1; i <= ROW_SIZE; ++i) {
                    if (i != ROW_SIZE && prev != "x" && matrix[i][j] == prev) {
                        score += 10;
                        if (score == 20) {
                            startChainRow = i - 1;
                        }
                    } else {
                        if (score > 20) {
                            StepProfit localProfit;
                            if (score == 50) {
                                collectSun(prev, i, j, score, localProfit.score,
                                           localProfit.sun);
                            }
                            collectRockets(prev, i, j, score, localProfit.score,
                                           localProfit.rocket);
                            if (localProfit.exists()) {
                                profit = profit + localProfit;
                            } else if (score == 30) {
                                // no boosters collected
                                const auto& beginThree = MatrixPoint{i - 3, j};
                                const auto& endThree = MatrixPoint{i - 1, j};
                                if (beginThree.isCorrect() &&
                                    endThree.isCorrect()) {
                                    int nothing = -1;  // todo need fix this
                                    replacePointToPoint(beginThree, endThree,
                                                        "x", profit.score,
                                                        nothing);
                                }
                            }
                        }
                        if (MatrixPoint{i, j}.isCorrect() &&
                            matrix[i][j] != "x") {
                            score = 10;
                        }
                    }
                    if (i < ROW_SIZE) {
                        prev = matrix[i][j];
                    }
                }
            }

            for (j = 0; j < COL_SIZE; ++j) {
                notEmptyShiftToBottomInColumn(j);
            }

            sum = sum + profit;
            cout << *this << endl;
        } while (profit.exists());
        return sum;
    }

    StepProfit explodeBooster(int row, int col) {
        // todo add explode sun by swap
        // todo refactor this
        const string typeName = matrix[row][col];
        int score = 0, suns = 0, rockets = 0, snows = 0;
        matrix[row][col] = "x";
        if (typeName == mapper.at("vrocket")) {
            for (int i = 0; i < matrix.size(); ++i) {
                if (matrix[i][col] != "x") {
                    setCellDirect(i, col, "x");
                    score += 10;
                }
            }
            rockets--;
        } else if (typeName == mapper.at("grocket")) {
            for (int j = 0; j < matrix[0].size(); ++j) {
                if (matrix[row][j] != "x") {
                    setCellDirect(row, j, "x");
                    score += 10;
                }
            }
            rockets--;
        } else if (typeName == mapper.at("sun")) {
            map<string, int> counter;
            for (int i = 0; i < matrix.size(); ++i) {
                for (int j = 0; j < matrix[0].size(); ++j) {
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
            for (int i = 0; i < matrix.size(); ++i) {
                for (int j = 0; j < matrix[0].size(); ++j) {
                    if (frequent == matrix[i][j]) {
                        matrix[i][j] = "x";
                        score += 10;
                    }
                }
            }
            suns--;
        } else {
            // ignore snow because unpredictable
            matrix[row][col] = mapper.at("snow");
        }
        if (score > 0) {
            for (int j = 0; j < COL_SIZE; ++j) {
                notEmptyShiftToBottomInColumn(j);
            }
        }
        // cout << *this << endl;
        return {score, suns, snows, rockets};
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
                              THRESH_BINARY, size, -200);

            double minVal, maxVal = 1;
            Point minLoc, maxLoc;
            while (maxVal > 0) {
                minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
                if (maxVal > 0) {
                    floodFill(res, maxLoc, 0);
                    const auto position =
                        detectRowAndCol(maxLoc.x + gameObject.second.cols,
                                        maxLoc.y + gameObject.second.rows);
                    gm.setCell(position.first, position.second,
                               gameObject.first);
                }
            }
        }
        if (!gm.isValid()) {
            cout << "Building model failed" << endl;
        }
        return gm;
    }
};

void showMatches(const string& screenshotName, const string& objectImageName) {
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
                      size, -200);

    while (1) {
        double minVal, maxVal;
        Point minLoc, maxLoc;
        minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

        if (maxVal > 0) {
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
}

const map<string, string> GameModel::mapper = {
    {"briefcase", "b"}, {"gold", "g"}, {"pig", "p"},     {"pocket", "e"},
    {"sandclock", "8"}, {"sun", "o"},  {"vrocket", "|"}, {"grocket", "~"},
    {"snow", "s"},      {"empty", "x"}};

namespace Action {
struct Step {
    string action;
    MatrixPoint point;
    StepProfit profit;

    bool operator>(const Step& other) const {
        return profit.score + 60 * profit.rocket + 100 * profit.sun +
                   40 * profit.snow >
               other.profit.score + 60 * other.profit.rocket +
                   100 * other.profit.sun + 40 * other.profit.snow;
    }
};
StepProfit getScoreIfSwapBooster(GameModel& gm, int row, int col) {
    if (gm.isBoosterAt(row, col)) {
        auto profit1 = gm.explodeBooster(row, col);
        auto profit2 = gm.updateAndReturnProfit();
        return profit1 + profit2;
    }
    return {0, 0, 0, 0};
}
StepProfit getScoreAfterSwap(const GameModel& gm, int currentRow,
                             int currentCol, int newRow, int newCol) {
    if (gm.isBoosterAt(currentRow, currentCol) &&
        gm.isBoosterAt(newRow, newCol)) {
        return {0, 0, 0, 0};  // ignore 2 boosters swap because unpredictable
    }
    GameModel newGm = gm;
    newGm.swap(currentRow, currentCol, newRow, newCol);
    // todo replace this by one method
    auto profit = getScoreIfSwapBooster(newGm, currentRow, currentCol);
    if (profit.exists()) {
        return profit;
    }
    profit = getScoreIfSwapBooster(newGm, newRow, newCol);
    if (profit.exists()) {
        return profit;
    }
    return newGm.updateAndReturnProfit();
};

void checkAllSwaps(GameModel& gm) {
    vector<Step> steps;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 5; ++j) {
            const StepProfit& scoreForRight =
                Action::getScoreAfterSwap(gm, i, j, i, j + 1);
            const StepProfit& scoreForBottom =
                Action::getScoreAfterSwap(gm, i, j, i + 1, j);
            if (scoreForRight.exists()) {
                steps.push_back({"swap to right", {i, j}, scoreForRight});
            }
            if (scoreForBottom.exists()) {
                steps.push_back({"swap to bottom", {i, j}, scoreForBottom});
            }
        }
    }
    sort(steps.begin(), steps.end(), greater());
    for (int i = 0; i < 5; ++i) {
        cout << "[" << steps[i].point.row + 1 << ", " << steps[i].point.col + 1
             << "] " + steps[i].action << " " << steps[i].profit << endl;
    }
};
};  // namespace Action

void testShow() {
    showMatches("screen.png", "pig.png");
    showMatches("screen.png", "pocket.png");
    showMatches("screen.png", "gold.png");
    showMatches("screen.png", "briefcase.png");
    showMatches("screen.png", "sandclock.png");
    showMatches("screen.png", "sun.png");
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
    // assert(scores.score == 60); ??
    assert(scores.snow == 1);
    assert(scores.sun == 0);
    assert(scores.rocket == 1);
}

void test2() {
    // GameObjectImages gameObjectImages;

    // GameModel gm;
    // stringstream example;
    // example << "|b|8|g|b|g|p|";
    // example << "|e|8|g|e|g|b|";
    // example << "|b|p|8|p|b|p|";
    // example << "|8|b|p|b|b|p|";
    // example << "|g|g|b|p|g|g|";
    // example << "|e|b|p|8|b|e|";
    // example << "|8|e|b|8|e|b|";
    // example >> gm;
    // cout << gm << endl;

    // Action::getScoreAfterSwap(gm, 3, 2, 3, 3);
    // cout << flush << gm << endl;
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
    assert(scores.score == 60);
    assert(scores.snow == 0);
    assert(scores.sun == 1); // result 2 so fix it
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

void testAll() {
    cout << "Start tests" << endl;
    // testShow();
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    cout << "End tests" << endl;
}

int main() {
    testAll();

    GameObjectImages gameObjectImages;

    while (1) {
        cin.get();
        system("rm ../screen.png");
        system("~/platform-tools/adb exec-out screencap -p > ../screen.png");

        ScreenReader screenReader;
        GameModel gm = screenReader.buildModel(gameObjectImages);
        cout << flush << gm << endl;
        Action::checkAllSwaps(gm);
    }

    return 0;
}