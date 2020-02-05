#pragma once
#include "ITask.h"
#include <memory>
#include <queue>
#include<Windows.h>
#include<vector>
#include <mutex>
namespace threading
{
	class ThreadPool
	{
	public:
		static ThreadPool* getSingleton(void);
		static void destroySingleton(void);
		void shutdown(void);
		bool Initialize(int initNum,int maxNum);
		void addTask(std::shared_ptr<ITask> task);
		void addTask(std::vector<std::shared_ptr<ITask>>tasks);
		bool adjustCount(int num);
		static unsigned long __stdcall fnThread(void* parameter);
	private:
		ThreadPool();
		int getTaskCount(void);
		std::shared_ptr<ITask> getTask(void);

	private:
		std::queue<std::shared_ptr<ITask>>_taskQueue;
		CRITICAL_SECTION _cs;//可能所有线程获取同一份任务
		HANDLE _hExitEvent;
		HANDLE _hSemaphore;
		int _maxCount;
		int _currentCount;
		int _aliveCount;  
		static ThreadPool* _ThreadPool;
		static std::mutex _mutex_singleton;
	};
}