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

void printSteps(const std::vector<StepOrder>& steps) {
    for (const auto& order : steps) {
        std::cout << order << std::endl;
    }
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