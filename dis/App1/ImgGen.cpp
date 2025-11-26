#include "ImgGen.h"
#include <thread> // Required for std::this_thread
#include <chrono> // Required for std::chrono::seconds

void ImgGen::sendBatchImgs(const std::vector<cv::Mat> &mats)
{
	// if (!sm_m.create())
	// { 
	// 	// Sender creates shared memory and semaphore
	// 	return 1;
	// }
	// if (!sm_m.attach())
	// {
	// 	return 1;
	// }
	SharedData *data = sm_m.getData();
	sm_m.acquireSemaphore();
	if (data->status == 0) // Ready for Sender
	{
		sm_m.writeMats(mats);
		data->status = 1; // Ready for Modifier
        sm_m.releaseSemaphore();
	}
	else
	{
		// Sleep for 100 milliseconds
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void ImgGen::readImgInf(const std::string &folderPath, const int batchSize)
{
    std::vector<cv::String> filenames;
    cv::glob(folderPath + "*.jpg", filenames, false);
    cv::glob(folderPath + "*.png", filenames, false);
    cv::glob(folderPath + "*.jpeg", filenames, false);
    cv::glob(folderPath + "*.bmp", filenames, false);
    if (filenames.empty())
    {
        std::cerr << "Error: No images found in the specified folder: " << folderPath << std::endl;
        return;
    }
    int currentIndex = 0;
    std::vector<cv::Mat> images;
    while (true)
    {
        std::cout << "Loading batch starting from index " << currentIndex << std::endl;
        for (int i = 0; i < batchSize; ++i)
        {
            int index = (currentIndex + i) % filenames.size(); // Wrap around if needed
            cv::Mat image = cv::imread(filenames[index]);

            if (image.empty())
            {
                std::cerr << "Warning: Could not read image: " << filenames[index] << std::endl;
                continue;
            }

            images.push_back(image);
            std::cout << "Read image: " << filenames[index] << std::endl;
        }
        currentIndex = (currentIndex + batchSize) % filenames.size(); // Update index for next batch
		sendBatchImgs(images);
		images.clear();
    }
}
