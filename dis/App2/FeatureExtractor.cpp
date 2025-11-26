#include "FeatureExtractor.h"
#include <iostream>

// void FeatureExtractor::sendBatchImgs(const std::vector<cv::Mat>& mats)
// {
// 	// Implementation for sending batch images if needed
// }

std::vector<ImageFeatures> FeatureExtractor::readImgs()
{
	std::vector<cv::Mat> &imageBatch;
	if (!sm_m.attach())
	{
		// Modifier attaches
		return 1;
	}
	SharedData *data = sm_m.getData();
	shm.acquireSemaphore();
	if (data->status == 1)
	{
		// Ready for Modifier
		imageBatch = sm_m.readMats();
		data->status = 2; // Ready for Receiver
	}
	shm.releaseSemaphore();
	return imageBatch;
}

std::vector<ImageFeatures> FeatureExtractor::extractFeatures(const std::vector<cv::Mat> &imageBatch)
{
	std::vector<ImageFeatures> imagesFeatures;
	imagesFeatures.reserve(imageBatch.size());
	for (const auto &image : imageBatch)
	{
		ImageFeatures currentFeatures;
		currentFeatures.image = image.clone();
		// Detect SIFT keypoints
		siftDetector->detect(image, currentFeatures.keypoints);
		imagesFeatures.push_back(currentFeatures);
	}
	return imagesFeatures;
}

void FeatureExtractor::runModifier()
{
	while (true)
	{
		if (!sm_m.attach())
		{
			// Modifier attaches
			return 1;
		}
		SharedData *data = sm_m.getData();
		if (data->status == 1)
		{
			// Ready for Modifier
			sm_m.acquireSemaphore();
			std::vector<cv::Mat> imageBatch = sm_m.readMats();
			// Process features as needed
			std::vector<ImageFeatures> sist_data = extractFeatures(imageBatch);
			std::vector<cv::Mat> mats;
			for(const auto item : sift_data)
			{
				// Draw key-points on the image for visualization
    			cv::Mat imageWithKeypoints;
    			cv::drawKeypoints(item.image, keypoints, imageWithKeypoints, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
				mats.push_back(imageWithKeypoints);
			}
			sm_m.writeMats(mats);
			sm_m.releaseSemaphore();
			// Ready for Receiver
			data->status = 2; 
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}