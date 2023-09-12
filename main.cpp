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

struct StepProfit {
    int score = 0;  // score from basic objects
    int sun = 0;    // amount of suns
    int snow = 0;
    int rocket = 0;

    bool exists() const {
        return score > 0 || sun > 0 || snow > 0 || rocket > 0;
    }

    friend ostream& operator<<(ostream& out, const StepProfit& profit) {
        out << "Score: " << profit.score << ",";
        out << " +sun: " << profit.sun << ",";
        out << " +snow: " << profit.snow << ",";
        out << " +rocket: " << profit.rocket;
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

   public:
    static const map<string, string> mapper;
    GameModel() {
        matrix.resize(7);
        for (auto& row : matrix) {
            row.resize(6);
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
            int resultScore = 0, suns = 0, rockets = 0, snows = 0;
            set<int> colsWithChain;
            // check horizontal chains and boosters
            for (i = 0; i < matrix.size(); ++i) {
                int startChainCol = matrix[0].size();
                string prev = matrix[i][0];
                int score = prev == "x" ? 0 : 10;
                for (j = 1; j < matrix[i].size(); ++j) {
                    if (prev != "x" && matrix[i][j] == prev) {
                        score += 10;
                        if (score == 20) {
                            startChainCol = j - 1;
                        }
                    } else {
                        if (score >= 20) {
                            if (score > 20) {
                                resultScore += score;
                                for (int k = startChainCol; k < j; ++k) {
                                    matrix[i][k] = "x";  // empty cell
                                    colsWithChain.insert(k);
                                }
                            }
                            startChainCol = matrix[0].size();
                            if (score == 20 && prev != "x" &&
                                i + 1 < matrix.size() && j - 2 >= 0 &&
                                matrix[i + 1][j - 2] == prev &&
                                matrix[i + 1][j - 1] == prev) {
                                // snow
                                matrix[i][j - 2] = mapper.at("snow");
                                matrix[i][j - 1] = "x";
                                matrix[i + 1][j - 2] = "x";
                                matrix[i + 1][j - 1] = "x";
                                resultScore += 40;
                                snows++;
                            } else if (score == 30) {
                                if (i + 2 < matrix.size() &&
                                    matrix[i + 1][j - 2] == prev &&
                                    matrix[i + 2][j - 2] == prev) {
                                    // collect sun
                                    resultScore += 20;
                                    matrix[i + 1][j - 2] = "x";
                                    matrix[i + 2][j - 2] = mapper.at("sun");
                                    suns++;
                                } else if (i - 2 >= 0 &&
                                           matrix[i - 1][j - 2] == prev &&
                                           matrix[i - 2][j - 2] == prev) {
                                    // collect sun
                                    resultScore += 20;
                                    matrix[i - 1][j - 2] = "x";
                                    matrix[i - 2][j - 2] = "x";
                                    matrix[i][j - 2] = mapper.at("sun");
                                    suns++;
                                }
                            } else if (score == 40) {
                                // grocket
                                matrix[i][j - 4] = mapper.at("grocket");
                                rockets++;
                            } else if (score == 50) {
                                matrix[i][j - 5] = mapper.at("sun");
                                suns++;
                            }
                        }
                        if (matrix[i][j] != "x") {
                            score = 10;
                        }
                    }
                    prev = matrix[i][j];
                }
                // after row check todo fix this dublicate
                if (score >= 20) {
                    if (score > 20) {
                        resultScore += score;
                        for (int k = startChainCol; k <= j - 1; ++k) {
                            matrix[i][k] = "x";  // empty cell
                            colsWithChain.insert(k);
                        }
                    }
                    if (prev != "x" && score == 20 && i + 1 < matrix.size() &&
                        matrix[i + 1][j - 2] == prev &&
                        matrix[i + 1][j - 1] == prev) {
                        // snow
                        colsWithChain.insert(j - 1);
                        colsWithChain.insert(j - 2);
                        matrix[i][j - 2] = mapper.at("snow");
                        matrix[i][j - 1] = "x";
                        matrix[i + 1][j - 2] = "x";
                        matrix[i + 1][j - 1] = "x";
                        resultScore += 40;
                        snows++;
                    } else if (score == 30) {
                        if (i + 2 < matrix.size() &&
                            matrix[i + 1][j - 2] == prev &&
                            matrix[i + 2][j - 2] == prev) {
                            // collect sun
                            resultScore += 20;
                            matrix[i + 1][j - 2] = "x";
                            matrix[i + 2][j - 2] = mapper.at("sun");
                            suns++;
                        } else if (i - 2 >= 0 && matrix[i - 1][j - 2] == prev &&
                                   matrix[i - 2][j - 2] == prev) {
                            // collect sun
                            resultScore += 20;
                            matrix[i - 1][j - 2] = "x";
                            matrix[i - 2][j - 2] = "x";
                            matrix[i][j - 2] = mapper.at("sun");
                            suns++;
                        }
                    } else if (score == 40) {
                        // grocket
                        matrix[i][j - 4] = mapper.at("grocket");
                        rockets++;
                    } else if (score == 50) {
                        matrix[i][j - 5] = mapper.at("sun");
                        suns++;
                    }
                }
            }
            // check vertical chains and boosters
            for (j = 0; j < matrix[0].size(); ++j) {
                int startChainRow = matrix.size();
                string prev = matrix[0][j];
                int score = prev == "x" ? 0 : 10;
                for (i = 1; i < matrix.size(); ++i) {
                    if (prev != "x" && matrix[i][j] == prev) {
                        score += 10;
                        if (score == 20) {
                            startChainRow = i - 1;
                        }
                    } else {
                        if (score > 20) {
                            resultScore += score;
                            colsWithChain.insert(j);
                            for (int k = startChainRow; k < i; ++k) {
                                matrix[k][j] = "x";  // empty cell
                            }
                            startChainRow = matrix.size();
                            if (score == 30) {
                                if (j + 2 < matrix[i].size() &&
                                    matrix[i - 2][j + 1] == prev &&
                                    matrix[i - 2][j + 2] == prev) {
                                    // collect sun
                                    resultScore += 20;
                                    matrix[i - 2][j + 1] = "x";
                                    matrix[i - 2][j + 2] = "x";
                                    matrix[i - 1][j] = mapper.at("sun");
                                    suns++;
                                } else if (j - 2 >= 0 &&
                                           matrix[i - 2][j - 1] == prev &&
                                           matrix[i - 2][j - 2] == prev) {
                                    // collect sun
                                    resultScore += 20;
                                    matrix[i - 2][j - 1] = "x";
                                    matrix[i - 2][j - 2] = "x";
                                    matrix[i - 1][j] = mapper.at("sun");
                                    suns++;
                                }
                            } else if (score == 40) {
                                // vrocket
                                matrix[i - 4][j] = mapper.at("vrocket");
                                rockets++;
                            } else if (score == 50) {
                                matrix[i][j - 5] = mapper.at("sun");
                                suns++;
                            }
                        }
                        if (matrix[i][j] != "x") {
                            score = 10;
                        }
                    }
                    prev = matrix[i][j];
                }
                // after col check
                if (score > 20) {
                    resultScore += score;
                    colsWithChain.insert(j);
                    for (int k = startChainRow; k <= i - 1; ++k) {
                        matrix[k][j] = "x";  // empty cell
                    }
                    if (score == 30) {
                        if (j + 2 < matrix[i].size() &&
                            matrix[i - 2][j + 1] == prev &&
                            matrix[i - 2][j + 2] == prev) {
                            // collect sun
                            resultScore += 20;
                            matrix[i - 2][j + 1] = "x";
                            matrix[i - 2][j + 2] = "x";
                            matrix[i - 1][j] = mapper.at("sun");
                            suns++;
                        } else if (j - 2 >= 0 && matrix[i - 2][j - 1] == prev &&
                                   matrix[i - 2][j - 2] == prev) {
                            // collect sun
                            resultScore += 20;
                            matrix[i - 2][j - 1] = "x";
                            matrix[i - 2][j - 2] = "x";
                            matrix[i - 1][j] = mapper.at("sun");
                            suns++;
                        }
                    } else if (score == 40) {
                        // vrocket
                        matrix[i - 4][j] = mapper.at("vrocket");
                        rockets++;
                    } else if (score == 50) {
                        matrix[i - 5][j] = mapper.at("sun");
                        suns++;
                    }
                }
                // snow not needed there because of horizontal
            }

            for (int j : colsWithChain) {
                notEmptyShiftToBottomInColumn(j);
            }

            profit = {resultScore, suns, snows, rockets};
            sum = sum + profit;
        } while (profit.exists());
        return sum;
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

namespace Action {
StepProfit getScoreAfterSwap(const GameModel& gm, int currentRow,
                             int currentCol, int newRow, int newCol) {
    GameModel newGm = gm;
    newGm.swap(currentRow, currentCol, newRow, newCol);
    return newGm.updateAndReturnProfit();
};
void checkAllSwaps(GameModel& gm) {
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 5; ++j) {
            const StepProfit& scoreForRight =
                Action::getScoreAfterSwap(gm, i, j, i, j + 1);
            const StepProfit& scoreForBottom =
                Action::getScoreAfterSwap(gm, i, j, i + 1, j);
            if (scoreForRight.exists()) {
                cout << "[" << i + 1 << ", " << j + 1
                     << "] to right :" << scoreForRight << endl;
            }
            if (scoreForBottom.exists()) {
                cout << "[" << i + 1 << ", " << j + 1
                     << "] to bottom :" << scoreForBottom << endl;
            }
        }
    }
};

};  // namespace Action

const map<string, string> GameModel::mapper = {
    {"briefcase", "b"}, {"gold", "g"},      {"pig", "p"},
    {"pocket", "e"},    {"sandclock", "8"}, {"sun", "o"},
    {"vrocket", "|"},   {"grocket", "~"},   {"snow", "s"}};

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
    example << "|g|e|8|g|e|g|";
    example << "|p|g|g|e|e|8|";
    example << "|e|e|b|g|g|b|";
    example << "|e|8|8|b|8|b|";
    example << "|p|g|p|p|e|g|";
    example << "|e|b|p|8|p|e|";
    example << "|g|b|b|g|e|e|";
    example >> gm;
    cout << gm << endl;

    gm.swap(5, 4, 4, 4);
    cout << gm.updateAndReturnProfit() << endl;
    cout << flush << gm << endl;
}

void test2() {
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|b|8|g|b|g|p|";
    example << "|e|8|g|e|g|b|";
    example << "|b|p|8|p|b|p|";
    example << "|8|b|p|b|b|p|";
    example << "|g|g|b|p|g|g|";
    example << "|e|b|p|8|b|e|";
    example << "|8|e|b|8|e|b|";
    example >> gm;
    cout << gm << endl;

    Action::getScoreAfterSwap(gm, 3, 2, 3, 3);
    cout << flush << gm << endl;
}

void test3() {
    GameObjectImages gameObjectImages;

    GameModel gm;
    stringstream example;
    example << "|g|e|8|e|p|b|";
    example << "|8|b|e|p|8|b|";
    example << "|b|8|g|e|8|8|";
    example << "|e|8|e|b|g|g|";
    example << "|b|p|e|e|8|e|";
    example << "|8|~|b|8|b|b|";
    example << "|e|e|g|8|b|e|";
    example >> gm;
    cout << gm << endl;

    cout << Action::getScoreAfterSwap(gm, 2, 3, 3, 3) << endl;
    cout << flush << gm << endl;
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
    cout << flush << gm << endl;
    assert(scores.score == 60);
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
    cout << flush << gm << endl;
    // todo need add support if swap was with booster because they explode and
    // then fix test assert(scores.score == 60);
}

int main() {
    // testShow();
    // test1();
    // test2();
    // test3();
    // test4();
    // test5();

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