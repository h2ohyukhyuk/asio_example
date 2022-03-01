
#include "packet.h"

void charArrayDeletor(char* x){
    delete[] x;
}

std::shared_ptr<char> makeImagePacket(const int cameraID, const int timeCode, const cv::Mat& img){

    const size_t imageSize = img.rows * img.cols * img.channels();

    if(imageSize <= 0)
        return nullptr;

    const size_t dataSize = sizeof(PacketHeader) + sizeof(ImageData) + imageSize + 3;
    std::shared_ptr<char> packet;
    packet.reset(new char[dataSize], charArrayDeletor);

    PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(packet.get());
    pHeader->begin[0] = 'b';
    pHeader->begin[1] = 'e';
    pHeader->begin[2] = 'g';
    pHeader->begin[3] = 'i';
    pHeader->begin[4] = 'n';

    pHeader->purpose = PacketPurpose::Image;
    pHeader->dataSize = dataSize - sizeof(PacketHeader) + 1;

    ImageData* pImageData = reinterpret_cast<ImageData*>(packet.get() + sizeof(PacketHeader));
    pImageData->cameraID = cameraID;
    pImageData->timeCode = timeCode;
    pImageData->width = img.cols;
    pImageData->height = img.rows;
    pImageData->channels = img.channels();
    pImageData->rgbOrder[0] = 'b';
    pImageData->rgbOrder[1] = 'g';
    pImageData->rgbOrder[2] = 'r';

    char* pImage = reinterpret_cast<char*>(packet.get() + sizeof(PacketHeader) + sizeof(ImageData));

    memcpy(pImage, img.data, imageSize);

    char* pEnd = pImage + imageSize;
    pEnd[0] = 'e';
    pEnd[1] = 'n';
    pEnd[2] = 'd';

    return packet;
}

std::shared_ptr<char> makeImageJoint2DPacket(const int cameraID, const int timeCode, const cv::Mat& img, std::vector<std::array<float, 2>> joint2Ds){

    const size_t imageSize = img.rows * img.cols * img.channels();
    const size_t jontSize = sizeof(float)*2*joint2Ds.size();
    const size_t dataSize = sizeof(PacketHeader) + sizeof(ImageData) + imageSize + sizeof(Joint2DData) + jontSize + 3;
    std::shared_ptr<char> packet;
    packet.reset(new char[dataSize], charArrayDeletor);

    PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(packet.get());
    pHeader->begin[0] = 'b';
    pHeader->begin[1] = 'e';
    pHeader->begin[2] = 'g';
    pHeader->begin[3] = 'i';
    pHeader->begin[4] = 'n';

    pHeader->purpose = PacketPurpose::Image;
    pHeader->dataSize = dataSize - sizeof(PacketHeader);

    ImageData* pImageData = reinterpret_cast<ImageData*>(packet.get() + sizeof(PacketHeader));
    pImageData->cameraID = cameraID;
    pImageData->timeCode = timeCode;
    pImageData->width = img.cols;
    pImageData->height = img.rows;
    pImageData->channels = img.channels();
    pImageData->rgbOrder[0] = 'b';
    pImageData->rgbOrder[1] = 'g';
    pImageData->rgbOrder[2] = 'r';

    char* pImage = reinterpret_cast<char*>(packet.get() + sizeof(PacketHeader) + sizeof(ImageData));

    memcpy(pImage, img.data, imageSize);

    Joint2DData* pJoint2DData = reinterpret_cast<Joint2DData*>(pImage + imageSize);
    float* pJoint2D = reinterpret_cast<float*>(pImage + imageSize + sizeof(Joint2DData));

    for( size_t i = 0; i < pJoint2DData->numberJoint; ++i){
        pJoint2D[2*i] = joint2Ds[i][0];
        pJoint2D[2*i + 1] = joint2Ds[i][1];
    }    

    char* pEnd = pImage + imageSize + sizeof(Joint2DData) + jontSize;
    pEnd[0] = 'e';
    pEnd[1] = 'n';
    pEnd[2] = 'd';
    
    return packet;
}

bool parseImagePacket(const char* data, cv::Mat& img, int& cameraID, int& timeCode){
    const ImageData* pImageData = reinterpret_cast<const ImageData*>(data);
    cameraID = pImageData->cameraID;
    timeCode = pImageData->timeCode;

    const char* pData = data + sizeof(ImageData);
    const int imgH = pImageData->height;
    const int imgW = pImageData->width;
    const int imgC = pImageData->channels;
    const int imgSize = imgH*imgW*imgC;

    if(imgC == 3){
        img = cv::Mat(imgH, imgW, CV_8UC3);
    }
    else if(imgC == 1){
        img = cv::Mat(imgH, imgW, CV_8UC1);
    }

    memcpy(img.data, pData, imgSize);
    
    return true;
}