

#pragma once

#include <opencv2/opencv.hpp>
#include <memory>

#pragma pack(push, 1)
struct PacketHeader
{
    char begin[5];
    unsigned int purpose;
    unsigned int dataSize;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ImageData
{
    int cameraID;
    int timeCode;
    int width;
    int height;
    int channels;
    char rgbOrder[3];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Joint2DData
{
    int numberJoint;
};
#pragma pack(pop)

enum PacketPurpose{
    Image = 0,
    ImageJpeg = 1,
    ImageJoint2D = 2,
};

void charArrayDeletor(char* x);
std::shared_ptr<char> makeImagePacket(const int cameraID, const int timeCode, const cv::Mat& img);
std::shared_ptr<char> makeJpgImagePacket(const int cameraID, const int timeCode, const std::vector<uchar>& data);
std::shared_ptr<char> makeImageJoint2DPacket(const int cameraID, const int timeCode, const cv::Mat& img, std::vector<std::array<float, 2>> joint2Ds);


bool parseImagePacket(const char* data, cv::Mat& img, int& cameraID, int& timeCode);