#include <iostream>
#include <map>
#include <vector>

#include "actions.h"
#include "constants.h"
#include "figureImages.h"
#include "gameModel.h"
#include "screenReader.h"
#include "stepOrder.h"
#include "stepOrderGenerator.h"

using namespace cv;

void printSteps(const std::vector<StepOrder>& steps) {
    for (const auto& order : steps) {
        std::cout << order << std::endl;
    }
}

int main() {
    FigureImages gameObjectImages;
    StepOrderGenerator generator;

    {
        // calibration
        system("rm ../screen.png");
        system("~/platform-tools/adb exec-out screencap -p > ../screen.png");
        ScreenReader screenReader;
        screenReader.calibrate(gameObjectImages);
        GameModel gm = screenReader.buildModel(gameObjectImages);
        if (!gm.isValid()) {
            return 1;
        } else {
            std::cout << "Calibration success!" << std::endl;
        }
    }

    while (1) {
        std::cin.get();
        system("rm ../screen.png");
        system("~/platform-tools/adb exec-out screencap -p > ../screen.png");

        ScreenReader screenReader;
        GameModel gm = screenReader.buildModel(gameObjectImages);
        std::cout << std::flush << gm << std::endl;
        generator.fillUp(gm);
        auto stepOrders = std::move(generator.generate(2));
        for (auto& stepOrder : stepOrders) {
            Action::calculateProfit(gm, stepOrder);
        }
        std::sort(stepOrders.begin(), stepOrders.end(), std::greater());
        printSteps({stepOrders.begin(), stepOrders.begin() + 4});
    }

    return 0;
}