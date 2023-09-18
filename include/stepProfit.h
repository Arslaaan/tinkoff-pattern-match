#pragma once
#include <iostream>

struct StepProfit {
    int score = 0;  // score from basic objects
    int sun = 0;    // amount of suns
    int snow = 0;
    int rocket = 0;

    bool exists() const;

    friend std::ostream& operator<<(std::ostream& out, const StepProfit& profit);

    StepProfit operator+(const StepProfit& profit);
};