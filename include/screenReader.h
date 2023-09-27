#pragma once
#include "figureImages.h"
#include "gameModel.h"
using namespace cv;

class ScreenReader {
    std::vector<std::vector<int>> grid = {
        {272, 404, 537, 669, 802, 934},
        {878, 1011, 1143, 1276, 1408, 1541, 1673}}; //android
    Mat gameScreen;
    Mat gref;
    const int error = 30;  // pixels

    std::pair<int, int> detectRowAndCol(int x, int y);

    bool inRange(int value, int prop);

   public:
    ScreenReader();

    ScreenReader(std::string screenshotName);

    GameModel buildModel(const FigureImages& gameObjectImages);

    void calibrate(const FigureImages& gameObjectImages);
};