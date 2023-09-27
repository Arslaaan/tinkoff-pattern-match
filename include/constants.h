#pragma once
#include <map>
#include <string>
#include <vector>
#include <set>
const int ROW_SIZE = 7;
const int COL_SIZE = 6;

const std::map<std::string, std::string> FIGURE_MAPPER = {
    {"briefcase", "b"}, {"gold", "g"},  {"pig", "p"},     {"pocket", "e"},
    {"sandclock", "8"}, {"sun", "o"},   {"vrocket", "|"}, {"grocket", "~"},
    {"snow", "s"},      {"empty", "x"},

    {"b", "briefcase"}, {"g", "gold"},  {"p", "pig"},     {"e", "pocket"},
    {"8", "sandclock"}, {"o", "sun"},   {"|", "vrocket"}, {"~", "grocket"},
    {"s", "snow"},      {"x", "empty"},
};

// todo replace by enum + refactor actions after
// todo check income by booster and then fix
const std::map<std::string, int> BOOSTER_COST_MAPPER = {
    {"[hammer]", 25},         {"[touch]", 25},      {"[swap right]", 25},
    {"[swap bottom]", 25}, {"[hand right]", 25}, {"[hand bottom]", 25},
    {"[hboom]", 75},
};

const std::set<std::string> DOUBLE_SUN = {"o", "o"};
const std::set<std::string> DOUBLE_SNOW = {"s", "s"};
const std::set<std::string> DOUBLE_ROCKET = {"~", "|"};
const std::set<std::string> SUN_AND_SNOW = {"o", "s"};
const std::set<std::string> ROCKET_AND_SNOW = {"|", "~", "s"};

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
