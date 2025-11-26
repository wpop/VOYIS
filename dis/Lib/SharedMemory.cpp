#include "SharedMemory.h"
#include <iostream>
#include <cstring> // For memset
#include <string>

SharedMemory::SharedMemory(key_t key, size_t size)
	: _key(key), _size(size), _shmid(-1), _semid(-1), _sharedData(nullptr) {}

SharedMemory::~SharedMemory()
{
	if (_sharedData != nullptr)
	{
		detach();
	}
}

bool SharedMemory::create()
{
	_shmid = shmget(_key, _size, IPC_CREAT | 0666);
	if (_shmid == -1)
	{
		perror("shmget");
		return false;
	}

	_semid = semget(_key, 1, IPC_CREAT | 0666);
	if (_semid == -1)
	{
		perror("semget");
		shmctl(_shmid, IPC_RMID, nullptr); // Clean up shared memory
		return false;
	}

	// Initialize semaphore to 1 (unlocked)
	semun su;
	su.val = 1;
	if (semctl(_semid, 0, SETVAL, su) == -1)
	{
		perror("semctl SETVAL");
		shmctl(_shmid, IPC_RMID, nullptr);
		semctl(_semid, 0, IPC_RMID, nullptr);
		return false;
	}

	return true;
}

bool SharedMemory::attach()
{
	_sharedData = static_cast<SharedData *>(shmat(_shmid, nullptr, 0));
	if (_sharedData == reinterpret_cast<SharedData *>(-1))
	{
		perror("shmat");
		_sharedData = nullptr;
		return false;
	}
	return true;
}

bool SharedMemory::detach()
{
	if (shmdt(_sharedData) == -1)
	{
		perror("shmdt");
		return false;
	}
	_sharedData = nullptr;
	return true;
}

bool SharedMemory::destroy()
{
	if (_shmid != -1 && shmctl(_shmid, IPC_RMID, nullptr) == -1)
	{
		perror("shmctl IPC_RMID");
		return false;
	}
	if (_semid != -1 && semctl(_semid, 0, IPC_RMID, nullptr) == -1)
	{
		perror("semctl IPC_RMID");
		return false;
	}
	return true;
}

SharedData *SharedMemory::getData()
{
	return _sharedData;
}

void SharedMemory::acquireSemaphore()
{
	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = -1; // Decrement semaphore (acquire)
	sb.sem_flg = 0;
	if (semop(_semid, &sb, 1) == -1)
	{
		perror("semop acquire");
	}
}

void SharedMemory::releaseSemaphore()
{
	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = 1; // Increment semaphore (release)
	sb.sem_flg = 0;
	if (semop(_semid, &sb, 1) == -1)
	{
		perror("semop release");
	}
}

std::vector<uchar> SharedMemory::serializeMat(const cv::Mat &mat)
{
	std::vector<uchar> buffer;
	cv::imencode(".jpg", mat, buffer); // Using JPG for lossless compression
	// size_t dot_pos = mat.find_last_of('.');
	// std::string file_extension;
	// if (dot_pos != std::string::npos)
	// {
	// 	file_extension = mat.substr(dot_pos + 1);
	// 	if (file_extension == "png")
	// 		cv::imencode(".png", mat, buffer); // Using PNG for lossless compression
	// 	if (file_extension == "jpg")
	// 		cv::imencode(".jpg", mat, buffer); // Using JPG for lossless compression
	// 	if (file_extension == "bmp")
	// 		cv::imencode(".bmp", mat, buffer); // Using BMP for lossless compression
	// }
	return buffer;
}

cv::Mat SharedMemory::deserializeMat(const uchar *data, size_t &offset)
{
	// Read size of the encoded image
	size_t encoded_size;
	memcpy(&encoded_size, data + offset, sizeof(size_t));
	offset += sizeof(size_t);

	// Read the encoded image data
	std::vector<uchar> buffer(data + offset, data + offset + encoded_size);
	offset += encoded_size;

	return cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
}

void SharedMemory::writeMats(const std::vector<cv::Mat> &mats)
{
	if(!_shm_ptr)
	{
		throw std::runtime_error("Shared memory not attached.");
	}

	// Clear previous content (optional, depending on usage)
	memset(_shm_ptr, 0, _size);

	char *current_ptr = static_cast<char *>(_shm_ptr);

	// Write number of matrices
	size_t num_mats = mats.size();
	memcpy(current_ptr, &num_mats, sizeof(size_t));
	current_ptr += sizeof(size_t);

	for (const auto &mat : mats)
	{
		std::vector<uchar> encoded_mat = serializeMat(mat);
		size_t encoded_size = encoded_mat.size();

		// Write size of the encoded mat
		memcpy(current_ptr, &encoded_size, sizeof(size_t));
		current_ptr += sizeof(size_t);

		// Write encoded mat data
		memcpy(current_ptr, encoded_mat.data(), encoded_size);
		current_ptr += encoded_size;
	}
}

std::vector<cv::Mat> SharedMemory::readMats()
{
	if (!_shm_ptr)
	{
		throw std::runtime_error("Shared memory not attached.");
	}

	std::vector<cv::Mat> mats;
	const uchar *current_data = static_cast<const uchar *>(_shm_ptr);
	size_t offset = 0;

	// Read number of matrices
	size_t num_mats;
	memcpy(&num_mats, current_data + offset, sizeof(size_t));
	offset += sizeof(size_t);

	for (size_t i = 0; i < num_mats; ++i)
	{
		mats.push_back(deserializeMat(current_data, offset));
	}
	return mats;
}