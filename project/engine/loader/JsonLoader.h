#pragma once
#include <string>
#include <Vector3.h>
#include<vector>

struct BezierPoint {
    Vector3 handleLeft;
    Vector3 controlPoint;
    Vector3 handleRight;
};

class JsonLoader {
public:
    std::vector<BezierPoint> LoadBezierFromJSON(const std::string& filePath);
};