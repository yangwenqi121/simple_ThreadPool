#include "ThreadPool.h"
using namespace threading;


threading::ThreadPool* threading::ThreadPool::_ThreadPool=nullptr;
std::mutex ThreadPool::_mutex_singleton;

ThreadPool::ThreadPool():_aliveCount(0),_maxCount(0),_currentCount(0)
{

}

ThreadPool * threading::ThreadPool::getSingleton(void)
{
	std::unique_lock<std::mutex> mutex_singleton(_mutex_singleton);
	if (!_ThreadPool)
		_ThreadPool = new ThreadPool;
	
	return _ThreadPool;
}

void threading::ThreadPool::destroySingleton(void)
{
	if (_ThreadPool)
	{
		delete _ThreadPool;
		_ThreadPool = nullptr;
	}
}

void threading::ThreadPool::shutdown(void)
{
	adjustCount(0);
	::CloseHandle(_hExitEvent);
	::CloseHandle(_hSemaphore);
	::DeleteCriticalSection(&_cs);
	_currentCount = 0;
	_maxCount = 0;
	_aliveCount = 0;
	_hExitEvent = nullptr;
	_hSemaphore = nullptr;
}

bool ThreadPool::Initialize(int initCount, int maxCount)
{
	_ThreadPool = this;
	 _currentCount = initCount;
	 _maxCount = maxCount;
	  
	 /*create semaphore */
	 _hSemaphore=::CreateSemaphore(nullptr, 0, _maxCount, nullptr);
	 if (_hSemaphore == nullptr)
		 return false;
	 /*create exit event */
	 _hExitEvent = ::CreateEvent(nullptr, true, false, nullptr);
	 if (_hExitEvent == nullptr)
		 return false;
	 /*create critical section */
	 ::InitializeCriticalSection(&_cs);

	 /*create threads..*/
	 for (int i=0;i<_currentCount;++i)
	 {
		 HANDLE hThread = ::CreateThread(nullptr, 0, fnThread, nullptr, 0, nullptr);
		 ::CloseHandle(hThread);
	 }
	 return true;
	
}
void threading::ThreadPool::addTask(std::shared_ptr<ITask> task)
{
	if (!task)
		return;
	EnterCriticalSection(&_cs);
	_taskQueue.push(task);

	if (_aliveCount<_currentCount)  //此处需要同步
		ReleaseSemaphore(_hSemaphore, 1, nullptr);

	LeaveCriticalSection(&_cs);

}
void threading::ThreadPool::addTask(std::vector<std::shared_ptr<ITask>> tasks)
{
	if (tasks.size() == 0)
		return;
	EnterCriticalSection(&_cs);
	
	for (auto& it : tasks)
		_taskQueue.push(it);

	if (_aliveCount < _currentCount)
		ReleaseSemaphore(_hSemaphore, _currentCount - _aliveCount >= tasks.size() ? tasks.size() : _currentCount - _aliveCount,nullptr);

	LeaveCriticalSection(&_cs);

}
bool threading::ThreadPool::adjustCount(int count)
{
	if (count > _maxCount || count == _currentCount)
		return false;
	/* increase working threads. */
	if (count >_currentCount)
	{
		/*create threads */
		for (int i = 0; i < count-_currentCount; i++)
		{
			auto threads = ::CreateThread(nullptr, 0, fnThread, nullptr, 0, nullptr);
			if (threads)
				::CloseHandle(threads);
		}
		/*increase _currentCount */
		_currentCount = count;
	}
	/* decrease working threads  */
	else if (count<_currentCount)
	{
		::SetEvent(_hExitEvent); //打开事件
		::ReleaseSemaphore(_hSemaphore, _currentCount - count, nullptr);
		//让线程睡眠一会有机会再推出，放在释放不完全，不断的尝试睡眠
		do  
		{
			::Sleep(20);
		} while (_currentCount > count);

		::ResetEvent(_hExitEvent);
	}
	return true;

}
unsigned long threading::ThreadPool::fnThread(void* parameter)
{
	while (true)
	{
		::WaitForSingleObject(_ThreadPool->_hSemaphore, INFINITE);
		_ThreadPool->_aliveCount++;
		/*check exit status */
		if (::WaitForSingleObject(_ThreadPool->_hExitEvent, 0) == WAIT_OBJECT_0)
		{
			_ThreadPool->_aliveCount--;
			break;
		}
		/*run task */
		while (auto task = _ThreadPool->getTask())
		{ 	
			task->execute();
		}
		_ThreadPool->_aliveCount--;
	}
	
	_ThreadPool->_currentCount--;
	return 0;
}

int threading::ThreadPool::getTaskCount(void)
{
	int count = 0;
	EnterCriticalSection(&_cs);
	count = _taskQueue.size();
	LeaveCriticalSection(&_cs);

	return count;
}

std::shared_ptr<ITask> threading::ThreadPool::getTask(void)
{
	std::shared_ptr<ITask>task = nullptr;//隐式构造函数
	if (_taskQueue.size()==0)
	return task;

	EnterCriticalSection(&_cs);
	task = _taskQueue.front();
	_taskQueue.pop();
	LeaveCriticalSection(&_cs);
	return task;
}
