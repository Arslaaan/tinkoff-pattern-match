#include "stepOrderGenerator.h"

StepOrderGenerator::StepOrderGenerator() {
    for (int i = 0; i < ROW_SIZE; ++i) {
        // basicActions.push_back({{"[hboom]"}, {{i, 0}}, {0, 0, 0, 0}});
        for (int j = 0; j < COL_SIZE; ++j) {
            if (i < ROW_SIZE - 1) {
                basicActions.push_back(
                    {{"[swap bottom]"}, {{i, j}}, {0, 0, 0, 0}});
                // basicActions.push_back(
                //     {{"[hand bottom]"}, {{i, j}}, {0, 0, 0, 0}});
            }
            if (j < COL_SIZE - 1) {
                basicActions.push_back(
                    {{"[swap right]"}, {{i, j}}, {0, 0, 0, 0}});
                // basicActions.push_back(
                //     {{"[hand right]"}, {{i, j}}, {0, 0, 0, 0}});
            }
        }
    }
    startSize = basicActions.size();
}

void StepOrderGenerator::fillUp(const GameModel& gm) {
    basicActions.erase(basicActions.begin() + startSize, basicActions.end());
    for (int i = 0; i < ROW_SIZE; ++i) {
        for (int j = 0; j < COL_SIZE; ++j) {
            if (gm.isBoosterAt(i, j)) {
                basicActions.push_back({{"[touch]"}, {{i, j}}, {0, 0, 0, 0}});
            } else {
                // basicActions.push_back({{"[hammer]"}, {{i, j}}, {0, 0, 0, 0}});
            }
        }
    }
    // std::sort(basicActions.begin(), basicActions.end(), std::greater());
}

std::vector<StepOrder> StepOrderGenerator::generate(int deep) {
    std::vector<StepOrder> result(basicActions.begin(), basicActions.end());
    if (deep >= 2) {
        size_t predictedSize = 0;
        for (int d = 1; d <= deep; ++d) {
            predictedSize += std::pow(basicActions.size(), d);
        }
        std::cout << "Variants: " << predictedSize << std::endl;
        size_t beginIndex = 0;
        size_t endIndex = result.size();  // endIndex not included
        for (int d = 2; d <= deep; ++d) {
            for (int i = beginIndex; i < endIndex; ++i) {
                for (int j = 0; j < basicActions.size(); ++j) {
                    StepOrder newOrder = result.at(i);
                    newOrder.addStepOrder(basicActions[j]);
                    result.emplace_back(newOrder);
                    // if (result.size() % 50 == 0) {
                    //     std::cout << result.size() << "/" << predictedSize
                    //               << std::endl;
                    // }
                }
            }
            beginIndex = endIndex;
            endIndex = result.size();
        }
    }
    return result;
}
