#include "ImgDBManager.h"
#include <iostream>
#include <string>

void ImgDBManager::runDataLogger()
{
	while (true)
	{
		if (!sm_m.attach())
		{
			// Receiver attaches
			return 1;
		}
		SharedData *data = sm_m.getData();
		sm_m.acquireSemaphore();
		if (data->status == 2)
		{
			// Ready for Receiver
			std::vector<cv::Mat> imageBatch = sm_m.readMats();
			std::vector<std::string> imageNames;
			for(const auto & image: imageBatch)
			{
				imageNames.push_back(name);
			}
			DML(imageNames, images);

			sm_m.releaseSemaphore();
			data->status = 0; // Ready for Sender again
		}
		else
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

ImgDBManager::~ImgDBManager()
{
	closeDatabase();
}

bool ImgDBManager::openDatabase()
{
	int rc = sqlite3_open(dbPath_.c_str(), &db_);
	if (rc != SQLITE_OK)
	{
		handleSQLError("Failed to open database");
		return false;
	}
	std::cout << "Database opened successfully." << std::endl;
	return true;
}

void ImgDBManager::closeDatabase()
{
	if (db_)
	{
		sqlite3_close(db_);
		db_ = nullptr;
		std::cout << "Database closed." << std::endl;
	}
}

bool ImgDBManager::createTable()
{
	const char *sql = "CREATE TABLE IF NOT EXISTS Images (Name TEXT PRIMARY KEY, Data BLOB);";
	char *errMsg = nullptr;
	int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK)
	{
		handleSQLError(std::string("Failed to create table: ") + errMsg);
		sqlite3_free(errMsg);
		return false;
	}
	std::cout << "Table 'Images' created or already exists." << std::endl;
	return true;
}

bool ImgDBManager::insertImage(const std::vector<std::string> &imageNames, const std::vector<cv::Mat> &images)
{
	if (images.empty())
	{
		std::cerr << "Error: Cannot insert empty image." << std::endl;
		return false;
	}
	for (int i = 0; i < images.size(); ++i)
	{
		std::vector<uchar> buffer;
		size_t dot_pos = images[i].find_last_of('.');
		std::string file_extension;
		if (dot_pos != std::string::npos)
		{
			file_extension = images[i].substr(dot_pos + 1);
			if(file_extension == "png")
				cv::imencode(".png", images.at(i), buffer);
			if(file_extension == "jpg")
				cv::imencode(".jpg", images.at(i), buffer);
			if(file_extension == "bmp")
				cv::imencode(".bmp", images.at(i), buffer);
		}
		sqlite3_stmt *stmt = nullptr;
		const char *sql = "INSERT INTO Images (Name, Data) VALUES (?, ?);";
		int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
		if (rc != SQLITE_OK)
		{
			handleSQLError("Failed to prepare statement for insert");
			return false;
		}
		sqlite3_bind_text(stmt, 1, imageNames[i].c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_blob(stmt, 2, buffer.data(), buffer.size(), SQLITE_STATIC);
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
		{
			handleSQLError("Failed to execute insert statement");
			sqlite3_finalize(stmt);
			return false;
		}
		sqlite3_finalize(stmt);
		std::cout << "Image '" << imageNames[i] << "' inserted successfully." << std::endl;
		return true;
	}
}

cv::Mat ImgDBManager::retrieveImage(const std::string &imageName)
{
	sqlite3_stmt *stmt = nullptr;
	const char *sql = "SELECT Data FROM Images WHERE Name = ?;";
	int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
	if (rc != SQLITE_OK)
	{
		handleSQLError("Failed to prepare statement for retrieve");
		return cv::Mat();
	}

	sqlite3_bind_text(stmt, 1, imageName.c_str(), -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		const void *blobData = sqlite3_column_blob(stmt, 0);
		int blobSize = sqlite3_column_bytes(stmt, 0);

		std::vector<uchar> buffer(static_cast<const uchar *>(blobData),
								  static_cast<const uchar *>(blobData) + blobSize);
		cv::Mat image = cv::imdecode(buffer, cv::IMREAD_COLOR); // Decode image

		sqlite3_finalize(stmt);
		std::cout << "Image '" << imageName << "' retrieved successfully." << std::endl;
		return image;
	}
	else if (rc == SQLITE_DONE)
	{
		std::cerr << "Image '" << imageName << "' not found." << std::endl;
	}
	else
	{
		handleSQLError("Failed to execute retrieve statement");
	}

	sqlite3_finalize(stmt);
	return cv::Mat();
}

void ImgDBManager::handleSQLError(const std::string &message)
{
	std::cerr << "SQLite Error: " << message << ": " << sqlite3_errmsg(db_) << std::endl;
}

void ImgDBManager::DML(const std::vector<std::string> &imageNames, const std::vector<cv::Mat> &images)
{
	openDatabase();
	createTable();
	insertImage(imageNames, images);
	closeDatabase();
}