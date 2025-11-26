#include "../Lib/SharedMemory.h"
#include "ImgGen.h"

int main()
{
	key_t key = ftok("shared_memory_file", 'A');
	size_t size_ = 1024 * 1024 * 100; // 100 MB;
	SharedMemory& sm(key, size_);
	ImgGen publisher(key, size_, sm);
	publisher.readImgInf("../images", 3);
}
