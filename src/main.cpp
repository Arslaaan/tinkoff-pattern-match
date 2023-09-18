#include <iostream>
#include <map>
#include <vector>

#include "constants.h"
#include "figureImages.h"
#include "gameModel.h"
#include "screenReader.h"
#include "stepOrder.h"
#include "actions.h"

using namespace cv;

// #define DEBUG

int showMatches(const std::string& screenshotName,
                const std::string& objectImageName) {
    // Load images
    Mat gameScreen = imread("../" + screenshotName, IMREAD_COLOR);
    Mat smallImage = imread("../" + objectImageName, IMREAD_COLOR);

    if (gameScreen.empty() || smallImage.empty()) {
        std::cout << "Error loading images!" << std::endl;
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
            std::cout << objectImageName << " "
                      << Point(maxLoc.x + smallImage.cols,
                               maxLoc.y + smallImage.rows)
                      << std::endl;
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

void printSteps(const std::vector<StepOrder>& steps) {
    for (const auto& order : steps) {
        std::cout << order << std::endl;
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

int main() {
    FigureImages gameObjectImages;
    if (CALIBRATION) {
        system("rm ../screen-test.png");
        system(
            "~/platform-tools/adb exec-out screencap -p > ../screen-test.png");
        ScreenReader screenReader("screen-test.png");
        screenReader.calibrate(gameObjectImages);
    } else {
        FigureImages gameObjectImages;

        while (1) {
            std::cin.get();
            system("rm ../screen.png");
            system(
                "~/platform-tools/adb exec-out screencap -p > ../screen.png");

            ScreenReader screenReader;
            GameModel gm = screenReader.buildModel(gameObjectImages);
            std::cout << std::flush << gm << std::endl;
            printSteps(Action::checkAllSwapsAndBoosterExplode(gm));
            std::cout << std::endl;
            printSteps(Action::checkAllSwapsAndBoosterExplodeLvl2(gm));
            std::cout << std::endl;
            printSteps(Action::checkAllHammer(gm));
            std::cout << std::endl;
            printSteps(Action::checkAllSupportHand(gm));
        }
    }

    return 0;
}