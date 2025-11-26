#ifndef FEATURE_EXTRACTOR_HPP
#define FEATURE_EXTRACTOR_HPP

#include "../Lib/SharedMemory.h"
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <vector>
#include <string>

// Structure to hold image data and corresponding keypoints
struct ImageFeatures {
    
    cv::Mat image;
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors; // Optional: to store SIFT descriptors
};

class FeatureExtractor {
public:
    FeatureExtractor(key_t key, size_t data, SharedMemory& sm): sm_m(sm) // sizeof(SharedData)
	{
		// Initialize SIFT detector with default parameters
		siftDetector = cv::SIFT::create();
	}
    ~FeatureExtractor() = default;

	// Extracts features from a batch of images
    std::vector<ImageFeatures> extractFeatures(const std::vector<cv::Mat>& imageBatch);

	// read images images via IPC
    std::vector<ImageFeatures> readImgs();

	void sendBatchImgs(const std::vector<cv::Mat>& mats);

	void runModifier();

    // // Extracts features from a batch of image paths
    // std::vector<ImageFeatures> extractFeaturesBatch(const std::vector<std::string>& imagePaths);

private:
    cv::Ptr<cv::SIFT> siftDetector; // SIFT feature detector
	SharedMemory& sm_m;
};

#endif // FEATURE_EXTRACTOR_HPP