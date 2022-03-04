#pragma once

#include <opencv2/opencv.hpp>
#include <memory>
#include <mutex>
#include <deque>

#pragma pack(push, 1)
struct PacketHeader
{
    char begin[5];
    unsigned int purpose;
    unsigned int dataSize;

    void reset(){
        memset(begin, 0, sizeof(begin));
    }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct ImageData
{
    int64_t cameraID;
    int64_t timeCode;
    int dataSize;
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

typedef std::shared_ptr<char> packet_type;

class PacketSendBuffer{
    public:
        PacketSendBuffer(const size_t maxPacketSize){
            maxPacketSize_ = maxPacketSize;
        }

        bool is_first(packet_type packet){
            std::lock_guard<std::mutex> lg(lock_);
            bool empty = packets_.empty();

            if(packets_.size() < maxPacketSize_){
                packets_.push_back(packet);
            }

            return empty;
        }

        packet_type next_packet(){
            std::lock_guard<std::mutex> lg(lock_);

            packets_.pop_front(); // this packet is trasmitted.
            packet_type packet = nullptr;

            if(!packets_.empty()){
                packet = packets_.front();
            }

            return packet;
        }

        packet_type front(){
            return packets_.front();
        }


    private:
        std::mutex lock_;
        std::deque<packet_type> packets_;
        size_t maxPacketSize_;
};

class PacketReceiveBuffer{
    public:
        PacketReceiveBuffer(const size_t maxPacketSize){
            maxPacketSize_ = maxPacketSize;
        }

        void enqueue(packet_type packet){
            std::lock_guard<std::mutex> lg(lock_);

            if(packets_.size() > maxPacketSize_){
                packets_.pop_front();
                std::cerr<<"receive buf full"<<std::endl;
            }
            packets_.push_back(packet);
        }

        packet_type dequeue(){
            std::lock_guard<std::mutex> lg(lock_);

            packet_type packet = nullptr;
            if(!packets_.empty()){
                packet = packets_.front();
                packets_.pop_front();
            }

            return packet;
        }

    private:
        std::mutex lock_;
        std::deque<packet_type> packets_;
        size_t maxPacketSize_;
};

void charArrayDeletor(char* x);

packet_type makeJpgImagePacket(const int64_t cameraID, const int64_t timeCode, const std::vector<uchar>& data);
packet_type makeImageJoint2DPacket(const int64_t cameraID, const int64_t timeCode, const cv::Mat& img, std::vector<std::array<float, 2>> joint2Ds);


bool parseJpgImagePacket(const char* data, cv::Mat& img, int64_t& cameraID, int64_t& timeCode);