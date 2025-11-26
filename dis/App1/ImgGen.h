#ifndef IMG_GEN_H
#define IMG_GEN_H

#include "../Lib/SharedMemory.h"
#include <opencv2/opencv.hpp>

class ImgGen {
public:
	// ImgGen(key_t key, sizeof(SharedData), SharedMemory& sm): sm_m(sm) {};
	ImgGen(key_t key, size_t, SharedMemory& sm): sm_m(sm) {};
	~ImgGen() = default;
	void readImgInf(const std::string &folderPath, const int batchSize);
private:
	void sendBatchImgs(const std::vector<cv::Mat>& mats);
	SharedMemory& sm_m;
};

#endif // IMG_GEN_H