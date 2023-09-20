#pragma once
#include <map>
#include <string>
#include <vector>
const int ROW_SIZE = 7;
const int COL_SIZE = 6;

const bool CALIBRATION = false;

const std::map<std::string, std::string> FIGURE_MAPPER = {
    {"briefcase", "b"}, {"gold", "g"},  {"pig", "p"},     {"pocket", "e"},
    {"sandclock", "8"}, {"sun", "o"},   {"vrocket", "|"}, {"grocket", "~"},
    {"snow", "s"},      {"empty", "x"},

    {"b", "briefcase"}, {"g", "gold"},  {"p", "pig"},     {"e", "pocket"},
    {"8", "sandclock"}, {"o", "sun"},   {"|", "vrocket"}, {"~", "grocket"},
    {"s", "snow"},      {"x", "empty"},
};

// todo replace by enum + refactor actions after
const std::map<std::string, int> BOOSTER_COST_MAPPER = {
    {"[hammer]", 50},         {"[touch]", 25},      {"[swap right]", 25},
    {"[swap bottom]", 25}, {"[hand right]", 50}, {"[hand bottom]", 50},
    {"[hboom]", 100},
};

namespace FiguresTemplates {
static const std::vector<std::vector<std::vector<std::string>>>&
    rocketTemplates = {
        {{"~", "x", "x", "x"}},
        {{"x"}, {"x"}, {"x"}, {"|"}},
};
static const std::vector<std::vector<std::vector<std::string>>>& sunTemplates =
    {
        {{"-", "x", "-"}, {"-", "x", "-"}, {"x", "o", "x"}},
        {{"-", "-", "x"}, {"-", "-", "x"}, {"x", "x", "o"}},
        {{"x", "-", "-"}, {"x", "-", "-"}, {"o", "x", "x"}},
        {{"x", "o", "x"}, {"-", "x", "-"}, {"-", "x", "-"}},
        {{"o", "x", "x"}, {"x", "-", "-"}, {"x", "-", "-"}},
        {{"x", "x", "o"}, {"-", "-", "x"}, {"-", "-", "x"}},
        {{"x", "-", "-"}, {"o", "x", "x"}, {"x", "-", "-"}},
        {{"-", "-", "x"}, {"x", "x", "o"}, {"-", "-", "x"}},
        {{"o", "x", "x", "x", "x"}},
        {{"x"}, {"x"}, {"x"}, {"x"}, {"o"}},
};
static const std::vector<std::vector<std::vector<std::string>>>& snowTemplates =
    {
        {{"x", "x"}, {"s", "x"}},
};
static const std::vector<std::vector<std::vector<std::string>>>&
    commonTemplates = {
        {{"x", "x", "x"}},
        {{"x"}, {"x"}, {"x"}},
};
}  // namespace FiguresTemplates
