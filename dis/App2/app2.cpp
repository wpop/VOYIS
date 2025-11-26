#include "FeatureExtractor.h"
#include "../Lib/SharedMemory.h"

int main()
{
	key_t key = ftok("shared_memory_file", 'A');
	size_t size_ = 1024 * 1024 * 100; // 100 MB;
	SharedMemory sm(key, size_);
	FeatureExtractor sift(key, size_, sm));
	sift.runModifier();
}
