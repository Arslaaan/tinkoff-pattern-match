#include <gtest/gtest.h>

#include <iostream>
#include <sstream>

#include "actions.h"
#include "figureImages.h"
#include "gameModel.h"
#include "stepOrderGenerator.h"

TEST(MATRIX, test1) {
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|b|e|e|b|8|b|";
    example << "|g|8|b|8|e|g|";
    example << "|8|b|p|p|b|p|";
    example << "|8|e|g|p|8|g|";
    example << "|e|8|p|8|g|8|";
    example << "|8|g|8|b|e|b|";
    example << "|e|8|o|8|8|g|";
    example >> gm;
    std::cout << gm << std::endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 3, 2, 4, 2);
    std::cout << scores << std::endl;
    ASSERT_EQ(scores.score, 80);
    ASSERT_EQ(scores.snow, 1);
    ASSERT_EQ(scores.sun, 0);
    ASSERT_EQ(scores.rocket, 1);
}

TEST(MATRIX, test2) {
    // todo i dont know, all seems corect
    // error in [hand right] [5, 2] [swap to bottom] [5, 3] score: 110 +snow: 1,
    // +rocket: 1
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    // example << "|b|e|8|b|e|p|";
    // example << "|b|e|e|p|g|b|";
    // example << "|p|g|8|g|8|g|";
    // example << "|g|b|e|b|p|b|";
    // example << "|p|g|8|e|p|g|";
    // example << "|g|8|e|g|e|g|";
    // example << "|e|8|p|e|g|8|";
    // example >> gm;
    // cout << gm << endl;

    // gm.swap(4, 1, 4, 2);
    // auto profit1 = gm.updateAndReturnProfit();
    // gm.swap(4, 2, 5, 2);
    // auto profit2 = gm.updateAndReturnProfit();
    // cout << profit1 + profit2 << endl;
}

TEST(MATRIX, test3) {
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|p|b|g|8|e|p|";
    example << "|e|e|g|8|e|e|";
    example << "|g|g|8|g|~|8|";
    example << "|e|p|g|e|g|b|";
    example << "|p|8|g|g|8|p|";
    example << "|b|8|8|b|p|g|";
    example << "|p|b|g|8|b|b|";
    example >> gm;
    std::cout << gm << std::endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 2, 2, 2, 3);
    std::cout << scores << std::endl;
    ASSERT_EQ(scores.score, 80);
    ASSERT_EQ(scores.snow, 0);
    ASSERT_EQ(scores.sun, 1);
    ASSERT_EQ(scores.rocket, 0);
}

TEST(MATRIX, test4) {
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|e|p|b|e|g|g|";
    example << "|e|8|8|g|e|p|";
    example << "|g|b|~|e|g|8|";
    example << "|p|g|8|p|p|b|";
    example << "|e|||b|e|8|8|";
    example << "|g|p|e|e|p|e|";
    example << "|8|e|b|8|b|b|";
    example >> gm;
    std::cout << gm << std::endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 1, 3, 1, 4);
    std::cout << scores << std::endl;
    ASSERT_EQ(scores.score, 60);
    ASSERT_EQ(scores.snow, 0);
    ASSERT_EQ(scores.sun, 0);
    ASSERT_EQ(scores.rocket, 0);
}

TEST(MATRIX, test5) {
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|p|b|e|b|p|p|";
    example << "|e|e|p|e|p|b|";
    example << "|e|p|b|8|g|g|";
    example << "|g|p|b|s|g|p|";
    example << "|e|b|~|b|p|8|";
    example << "|g|||b|e|p|e|";
    example << "|8|p|b|8|b|b|";
    example >> gm;
    std::cout << gm << std::endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 4, 1, 4, 2);
    std::cout << scores << std::endl;
    ASSERT_EQ(scores.score, 90);
    ASSERT_EQ(scores.sun, 0);
    ASSERT_EQ(scores.snow, 0);
    ASSERT_EQ(scores.rocket, 0);
}

TEST(MATRIX, test6) {
    // 4 6 to left should be sun
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|e|e|b|8|b|e|";
    example << "|b|b|g|g|b|g|";
    example << "|g|o|p|8|e|b|";
    example << "|g|p|e|e|g|e|";
    example << "|8|8|p|8|e|p|";
    example << "|g|b|8|8|b|e|";
    example << "|b|s|b|p|8|g|";
    example >> gm;
    std::cout << gm << std::endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 3, 4, 3, 5);
    std::cout << scores << std::endl;
    ASSERT_EQ(scores.score, 110);
    ASSERT_EQ(scores.sun, 1);
    ASSERT_EQ(scores.snow, 0);
    ASSERT_EQ(scores.rocket, 0);
}

TEST(MATRIX, test7) {
    //  [swap to right] [5, 2] score: 120 +rocket: -1
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|g|e|e|b|g|e|";
    example << "|8|b|p|8|p|8|";
    example << "|b|p|g|g|p|b|";
    example << "|p|p|e|g|e|e|";
    example << "|g|||p|b|g|p|";
    example << "|p|b|~|e|e|b|";
    example << "|e|b|p|e|g|8|";
    example >> gm;
    std::cout << gm << std::endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 4, 1, 4, 2);
    std::cout << scores << std::endl;
    ASSERT_EQ(scores.rocket, -2);
}

TEST(MATRIX, test8) {
    // test new method for collect boosters
    FigureImages gameObjectImages;
    {
        GameModel gm;
        std::stringstream example;
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|x|g|g|g|g|";
        example >> gm;
        std::cout << gm << std::endl;

        StepProfit scores = gm.updateAndReturnProfit();
        std::cout << scores << std::endl;
        ASSERT_EQ(scores.sun, 0);
        ASSERT_EQ(scores.snow, 0);
        ASSERT_EQ(scores.rocket, 2);
    }

    {
        GameModel gm;
        std::stringstream example;
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|x|x|x|x|x|";
        example << "|g|g|g|g|g|g|";
        example >> gm;
        std::cout << gm << std::endl;

        StepProfit scores = gm.updateAndReturnProfit();
        std::cout << scores << std::endl;
        ASSERT_EQ(scores.sun, 1);
        ASSERT_EQ(scores.snow, 0);
        ASSERT_EQ(scores.rocket, 0);
    }

    {
        GameModel gm;
        std::stringstream example;
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|x|x|x|x|x|x|";
        example << "|g|g|x|x|g|g|";
        example << "|g|g|x|x|g|g|";
        example >> gm;
        std::cout << gm << std::endl;

        StepProfit scores = gm.updateAndReturnProfit();
        std::cout << scores << std::endl;
        ASSERT_EQ(scores.sun, 0);
        ASSERT_EQ(scores.snow, 2);
        ASSERT_EQ(scores.rocket, 0);
    }
}

TEST(MATRIX, test9) {
    // todo
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|b|e|p|8|e|8|";
    example << "|g|p|g|8|8|e|";
    example << "|8|b|8|g|8|8|";
    example << "|b|8|g|b|p|b|";
    example << "|p|e|p|8|p|e|";
    example << "|g|e|b|g|e|p|";
    example << "|e|b|g|e|g|8|";
    example >> gm;
    std::cout << gm << std::endl;

    StepProfit scores = Action::getScoreAfterSwap(gm, 2, 2, 2, 3);
    std::cout << scores << std::endl;
    ASSERT_EQ(scores.sun, 1);
    ASSERT_EQ(scores.snow, 0);
    ASSERT_EQ(scores.rocket, 0);
}

TEST(MATRIX, test10) {
    // todo [hand right] [3, 4] [swap to bottom] [3, 5] score: 370 +snow: 2
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|g|b|e|8|e|p|";
    example << "|e|8|g|p|g|g|";
    example << "|p|g|e|g|p|g|";
    example << "|8|g|b|e|b|8|";
    example << "|8|b|8|p|g|e|";
    example << "|e|e|b|e|p|p|";
    example << "|8|8|g|b|g|e|";
    example >> gm;
    std::cout << gm << std::endl;

    auto scores = Action::getScoreAfterSwap(gm, 2, 3, 2, 4);
    scores = scores + Action::getScoreAfterSwap(gm, 2, 4, 3, 4);
    std::cout << scores << std::endl;
    ASSERT_EQ(scores.score, 110);
    ASSERT_EQ(scores.sun, 0);
    ASSERT_EQ(scores.snow, 0);
    ASSERT_EQ(scores.rocket, 0);
}

TEST(MATRIX, test11) {
    // todo [hammer] [4, 2] [swap to right] [3, 1] score: 50, +sun: 1
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|g|p|b|b|8|8|";
    example << "|b|g|p|g|e|g|";
    example << "|p|p|e|8|e|p|";
    example << "|b|g|b|p|s|p|";
    example << "|e|p|e|8|b|b|";
    example << "|p|p|b|~|g|g|";
    example << "|||8|8|b|e|8|";
    example >> gm;
    std::cout << gm << std::endl;

    auto scores = Action::hammer(gm, 3, 1);
    scores = scores + Action::getScoreAfterSwap(gm, 2, 0, 2, 1);
    std::cout << scores << std::endl;
    ASSERT_EQ(scores.sun, 0);  // no sun as result
}

TEST(MATRIX, test12) {
    // [swap right] [3, 3] score: 120 s/p: 4.8 -- error
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|b|8|g|g|b|8|";
    example << "|p|e|g|p|e|b|";
    example << "|b|e|b|p|8|8|";
    example << "|b|~|~|e|e|p|";
    example << "|g|g|o|8|8|g|";
    example << "|p|p|o|8|e|g|";
    example << "|b|p|o|p|b|8|";
    example >> gm;
    std::cout << gm << std::endl;

    auto scores = Action::getScoreAfterSwap(gm, 2, 2, 2, 3);
    std::cout << scores << std::endl;
    ASSERT_EQ(scores.score, 0);  // no sun as result
}

TEST(MATRIX, test13) {
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|b|8|g|g|b|8|";
    example << "|p|e|g|p|e|b|";
    example << "|b|e|b|p|8|8|";
    example << "|b|b|g|e|e|p|";
    example << "|g|g|p|8|8|g|";
    example << "|p|p|8|8|e|g|";
    example << "|b|p|e|p|b|8|";
    example >> gm;
    std::cout << gm << std::endl;

    StepOrder stepOrder({"[swap bottom]"}, {{4, 4}}, {0, 0, 0, 0});
    Action::calculateProfit(gm, stepOrder, true);
    std::cout << stepOrder.profit << std::endl;
    ASSERT_EQ(stepOrder.profit.score, 120);
}

TEST(DEEP_CALC, lvl2) {
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|b|8|g|g|b|8|";
    example << "|p|e|g|p|e|b|";
    example << "|b|e|b|p|8|8|";
    example << "|b|b|g|e|e|p|";
    example << "|g|g|p|8|8|g|";
    example << "|p|p|8|8|e|g|";
    example << "|b|p|e|p|b|8|";
    example >> gm;
    std::cout << gm << std::endl;

    StepOrderGenerator generator;
    generator.fillUp(gm);
    auto stepOrders = move(generator.generate(2));
    for (auto& stepOrder : stepOrders) {
        Action::calculateProfit(gm, stepOrder);
    }
    std::sort(stepOrders.begin(), stepOrders.end(), std::greater());
    std::cout << *stepOrders.begin() << std::endl;
}

TEST(DEEP_CALC, lvl3) {
    FigureImages gameObjectImages;

    GameModel gm;
    std::stringstream example;
    example << "|b|8|g|g|b|8|";
    example << "|p|e|g|p|e|b|";
    example << "|b|e|b|p|8|8|";
    example << "|b|b|g|e|e|p|";
    example << "|g|g|p|8|8|g|";
    example << "|p|p|8|8|e|g|";
    example << "|b|p|e|p|b|8|";
    example >> gm;
    std::cout << gm << std::endl;

    StepOrderGenerator generator;
    generator.fillUp(gm);
    auto stepOrders = move(generator.generate(3));
    for (auto& stepOrder : stepOrders) {
        Action::calculateProfit(gm, stepOrder);
    }
    std::sort(stepOrders.begin(), stepOrders.end(), std::greater());
    std::cout << *stepOrders.begin() << std::endl;
}

int main(int argc, char** argv) {
    ::testing::GTEST_FLAG(filter) = "*lvl2*";
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}