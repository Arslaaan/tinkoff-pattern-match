#pragma once

#include <iostream>
#include <map>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <string>

#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;

class FigureImages {
    std::map<std::string, Mat> gameObjects;

   public:
    FigureImages();

    const std::map<std::string, Mat>& getImages() const;
};