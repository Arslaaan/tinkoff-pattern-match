#pragma once
#include "figureImages.h"
#include "gameModel.h"
using namespace cv;

class ScreenReader {
    // std::vector<std::vector<int>> screenshotGrid = {
    //     {250, 391, 529, 668, 810}, {835, 975, 1115, 1255, 1395, 1538, 1676}};
    std::vector<std::vector<int>> grid = {
        {272, 404, 537, 670, 802, 934},
        {878, 1011, 1143, 1276, 1410, 1541, 1675}};
    Mat gameScreen;
    Mat gref;
    const int error = 25;  // pixels

    std::pair<int, int> detectRowAndCol(int x, int y);

    bool inRange(int value, int prop);

   public:
    ScreenReader();

    ScreenReader(std::string screenshotName);

    GameModel buildModel(const FigureImages& gameObjectImages);

    void calibrate(const FigureImages& gameObjectImages);
};