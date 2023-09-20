#pragma once
#include "figureImages.h"
#include "gameModel.h"
using namespace cv;

class ScreenReader {
    // todo read grid after calibrate
    std::vector<std::vector<int>> grid = {
        {271, 405, 537, 670, 802, 934},
        {878, 1011, 1145, 1277, 1408, 1541, 1673}};
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