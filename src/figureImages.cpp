#include "figureImages.h"

FigureImages::FigureImages() {
    std::vector<std::string> gameObjectNames = {
        "briefcase", "gold",    "pig",     "pocket", "sandclock",
        "sun",       "vrocket", "grocket", "snow"};  // todo base on mapper
    for (const auto& name : gameObjectNames) {
        Mat gameObject = imread("../game objects/" + name + ".png", IMREAD_COLOR);
        if (gameObject.empty()) {
            std::cout << "Error load " + name << std::endl;
        }
        Mat gtpl;
        cvtColor(gameObject, gtpl, COLOR_BGR2GRAY);
        Canny(gtpl, gtpl, 110, 330);
        gameObjects[name] = std::move(gameObject);
    }
}

const std::map<std::string, Mat>& FigureImages::getImages() const {
    return gameObjects;
}
