#ifndef IMG_DB_MANAGER_H
#define IMG_DB_MANAGER_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <sqlite3.h>

class ImgDBManager
{
public:
	ImgDBManager(key_t key, size_t data, SharedMemory &sm const std::string &dbPath)
		: sm_m(sm), dbPath_(dbPath), db_(nullptr) {}
	~ImgDBManager();

	void runDataLogger();

	bool openDatabase();
	void closeDatabase();
	bool createTable();
	bool insertImage(const std::vector<std::string> &imageNames, const std::vector<cv::Mat> &images);
	cv::Mat retrieveImage(const std::string &imageName);

private:
	std::string dbPath_;
	sqlite3 *db_;
	SharedMemory &sm_m;

	// Helper function to handle SQLite errors
	void handleSQLError(const std::string &message);
	void DML(const std::vector<std::string> &imageNames, const std::vector<cv::Mat> &images);
};

#endif // IMG_DB_MANAGER_H
