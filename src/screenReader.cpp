#include "screenReader.h"

ScreenReader::ScreenReader() {
    gameScreen = imread("../screen.png", IMREAD_COLOR);
    if (gameScreen.empty()) {
        std::cout << "Error load screen.png" << std::endl;
    }
    cvtColor(gameScreen, gref, COLOR_BGR2GRAY);
    Canny(gref, gref, 110, 330);
}

ScreenReader::ScreenReader(std::string screenshotName) {
    gameScreen = imread("../" + screenshotName, IMREAD_COLOR);
    if (gameScreen.empty()) {
        std::cout << "Error load screen.png" << std::endl;
    }
    cvtColor(gameScreen, gref, COLOR_BGR2GRAY);
    Canny(gref, gref, 110, 330);
}

GameModel ScreenReader::buildModel(const FigureImages& gameObjectImages) {
    GameModel gm;
    for (const auto& gameObject : gameObjectImages.getImages()) {
        Mat res_32f(gameScreen.rows - gameObject.second.rows + 1,
                    gameScreen.cols - gameObject.second.cols + 1, CV_32FC1);
        matchTemplate(gameScreen, gameObject.second, res_32f, TM_CCOEFF_NORMED);

        Mat res;
        res_32f.convertTo(res, CV_8U, 255.0);

        int size = ((gameObject.second.cols + gameObject.second.rows) / 4) * 2 +
                   1;  // force size to be odd
        adaptiveThreshold(res, res, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,
                          size, -160);

        double minVal, maxVal = 1;
        Point minLoc, maxLoc;
        while (maxVal > 0) {
            minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
            if (maxVal > 0) {
                floodFill(res, maxLoc, 0);
                Point point(maxLoc.x + gameObject.second.cols,
                            maxLoc.y + gameObject.second.rows);
                // std::cout << point << std::endl;
                std::pair<int, int> position =
                    detectRowAndCol(maxLoc.x + gameObject.second.cols,
                                    maxLoc.y + gameObject.second.rows);
                gm.setCell(position.first, position.second, gameObject.first);
            }
        }
    }
    return gm;
}

void ScreenReader::calibrate(const FigureImages& gameObjectImages) {
    std::vector<std::vector<int>> gridRaw = {{}, {}};
    for (const auto& gameObject : gameObjectImages.getImages()) {
        Mat gameScreenCopy = gameScreen.clone();
        Mat res_32f(gameScreenCopy.rows - gameObject.second.rows + 1,
                    gameScreenCopy.cols - gameObject.second.cols + 1, CV_32FC1);
        matchTemplate(gameScreenCopy, gameObject.second, res_32f,
                      TM_CCOEFF_NORMED);

        Mat res;
        res_32f.convertTo(res, CV_8U, 255.0);

        int size = ((gameObject.second.cols + gameObject.second.rows) / 4) * 2 +
                   1;  // force size to be odd
        adaptiveThreshold(res, res, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY,
                          size, -160);

        double minVal, maxVal = 1;
        Point minLoc, maxLoc;
        while (maxVal > 0) {
            minMaxLoc(res, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
            if (maxVal > 0) {
                floodFill(res, maxLoc, 0);
                Point point(maxLoc.x + gameObject.second.cols,
                            maxLoc.y + gameObject.second.rows);
                gridRaw[0].push_back(point.x);
                gridRaw[1].push_back(point.y);
                // rectangle(gameScreenCopy, maxLoc, point, Scalar(0, 255, 0),
                // 2);
            }
        }

        // namedWindow(gameObject.first, WINDOW_NORMAL);
        // imshow(gameObject.first, gameScreenCopy);
        // resizeWindow(gameObject.first, 800, 800);
        // waitKey(0);
    }

    for (int i = 0; i < 2; ++i) {
        grid[i].clear();
        std::sort(gridRaw[i].begin(), gridRaw[i].end());
        int prev = gridRaw[i][0];
        grid[i].push_back(prev);
        for (int j = 1; j < gridRaw[i].size(); ++j) {
            if (gridRaw[i][j] - prev > this->error) {
                grid[i].push_back(gridRaw[i][j]);
            }
            prev = gridRaw[i][j];
        }
    }
    for (int i = 0; i < 2; ++i) {
        for (const auto& it : grid[i]) {
            std::cout << it << ", ";
        }
        std::cout << std::endl;
    }
}

bool ScreenReader::inRange(int value, int prop) {
    return value >= prop - error && value <= prop + error;
}

std::pair<int, int> ScreenReader::detectRowAndCol(int x, int y) {
    // todo can be optimized
    int row = -1, col = -1;
    for (int i = 0; i < COL_SIZE; ++i) {
        if (inRange(x, grid[0][i])) {
            col = i;
        }
    }
    for (int i = 0; i < ROW_SIZE; ++i) {
        if (inRange(y, grid[1][i])) {
            row = i;
        }
    }
    return {row, col};
}

std::vector<std::vector<int>> ScreenReader::grid = {
    {272, 404, 537, 669, 802, 934},
    {878, 1011, 1143, 1276, 1408, 1541, 1673}};  // android