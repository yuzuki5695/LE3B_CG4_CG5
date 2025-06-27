#include "JsonLoader.h"
#include<json.hpp>
#include<fstream>

using json = nlohmann::json;

std::vector<BezierPoint> JsonLoader::LoadBezierFromJSON(const std::string& filePath) {
    std::vector<BezierPoint> points;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("JSONファイルを開けませんでした");
    }

    json j;
    file >> j;

    // "Curve" の最初のスプラインを使う
    if (!j.contains("Curve") || !j["Curve"].is_array() || j["Curve"].empty()) {
        throw std::runtime_error("Curveデータが存在しません");
    }

    for (const auto& pointData : j["Curve"][0]) {
        BezierPoint pt;
        pt.handleLeft = {
            pointData["handle_left"][0],
            pointData["handle_left"][1],
            pointData["handle_left"][2]
        };
        pt.controlPoint = {
            pointData["control_point"][0],
            pointData["control_point"][1],
            pointData["control_point"][2]
        };
        pt.handleRight = {
            pointData["handle_right"][0],
            pointData["handle_right"][1],
            pointData["handle_right"][2]
        };
        points.push_back(pt);
    }

    return points;
}