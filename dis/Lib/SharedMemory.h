#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

// Define status codes for inter-process communication
enum StatusCode {
    EMPTY,
    SENDER_WRITING,
    MODIFIER_PROCESSING,
    RECEIVER_READING,
    DONE
};

// Structure to be stored in shared memory
struct SharedData {
	StatusCode m_status;
    int status; // 0: Ready for Sender, 1: Ready for Modifier, 2: Ready for Receiver
    char message[256];
};

class SharedMemory {
public:
    SharedMemory(key_t key, size_t size);
    ~SharedMemory();

    bool create(); // Creates shared memory and semaphore
    bool attach(); // Attaches to shared memory
    bool detach(); // Detaches from shared memory
    bool destroy(); // Destroys shared memory and semaphore

    SharedData* getData(); // Returns pointer to shared data

    // Semaphore operations
    void acquireSemaphore();
    void releaseSemaphore();

	 // Method to write std::vector<cv::Mat> to shared memory
    void writeMats(const std::vector<cv::Mat>& mats);

    // Method to read std::vector<cv::Mat> from shared memory
    std::vector<cv::Mat> readMats();

private:
    key_t _key;
    size_t _size;
    int _shmid;
    SharedData* _sharedData;

    void* _shm_ptr;
    int _semid; // Semaphore ID for synchronization

    // Union for semctl (required for System V semaphores)
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    };

	// Helper to serialize a single cv::Mat
    std::vector<uchar> serializeMat(const cv::Mat& mat);
    // Helper to deserialize a single cv::Mat
    cv::Mat deserializeMat(const uchar* data, size_t& offset);
};

#endif // SHARED_MEMORY_H