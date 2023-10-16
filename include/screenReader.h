#pragma once
#include "figureImages.h"
#include "gameModel.h"
using namespace cv;

class ScreenReader {
    Mat gameScreen;
    Mat gref;
    const int error = 30;  // pixels

    std::pair<int, int> detectRowAndCol(int x, int y);

    bool inRange(int value, int prop);

   public:
    static std::vector<std::vector<int>> grid;
    ScreenReader();

    ScreenReader(std::string screenshotName);

    GameModel buildModel(const FigureImages& gameObjectImages);

    void calibrate(const FigureImages& gameObjectImages);
};