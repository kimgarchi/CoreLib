#pragma once
#include "stdafx.h"

class Logger
{
public:
	Logger();
	virtual ~Logger();

	Logger(const Logger& src) = delete;
	Logger& operator=(const Logger& rhs) = delete;

	void log(const std::string& entry);
	void Stop() { mExit = true; mCondVar.notify_all(); }

private:
	void processEntries();

	std::mutex mMutex;
	std::condition_variable mCondVar;
	std::queue<std::string> mQueue;
	std::thread mThread;

	std::atomic<bool> mExit;
};

Logger::Logger()
	: mExit(false)
{
	mThread = std::thread{ &Logger::processEntries, this };
}

Logger::~Logger()
{
	std::unique_lock<std::mutex> lock(mMutex);
	mExit = true;
	mCondVar.notify_all();

	mThread.join();
}

void Logger::log(const std::string& entry)
{
	std::unique_lock<std::mutex> lock(mMutex);
	mQueue.push(entry);
	mCondVar.notify_one();
}

void Logger::processEntries()
{
	std::ofstream ofs("log.txt");

	if (ofs.fail())
	{
		std::cerr << "Failed to open logfile..." << std::endl;
	}

	std::unique_lock<std::mutex> lock(mMutex);
	while (true)
	{
		if (!mExit)
		{
			mCondVar.wait(lock);
		}
		else
			break;

		lock.unlock();
		while (true)
		{
			lock.lock();

			if (mQueue.empty())
				break;
			else
			{
				ofs << mQueue.front() << std::endl;
				mQueue.pop();
			}

			lock.unlock();
		}
	}
}

void logSomeMessages(int id, Logger& logger)
{
	for (int i = 0; i < 10; ++i)
	{
		std::stringstream ss;
		ss << "Log entry " << i << " from thread " << id;
		logger.log(ss.str());
	}
}