/**
 * blocking queue_ for c++
 * @author joey
 * @date 2012-4-4
 */

#ifndef JOEYUTIL_UTIL_BLOCKINGQUEUE_H_
#define JOEYUTIL_UTIL_BLOCKINGQUEUE_H_

#ifndef BLOCKINGOP
#define BLOCKINGOP(operation)                       \
	while(1)                                        \
	{                                               \
		if((operation) == 0)                        \
			break;                                  \
		else if(errno != EINTR)                     \
		{                                           \
			perror("BLOCKINGOP signal interrupted");\
			_exit(1);                               \
		}                                           \
	}                                               
#endif
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h> 
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <list>

namespace joeyutil
{
	static void PthreadCall(int result)
	{
		if( result != 0 )
		{
			fprintf(stderr,"pthread call error!");
			_exit(1);	
		}
	}
	static void SemCall(int result)
	{
		if( result != 0 )
		{
			fprintf(stderr,"sem call error!");
			_exit(1);	
		}
	}

	template <class T>
		class BlockingQueue
		{
			public:
				BlockingQueue(uint32_t capacity);
				~BlockingQueue();

				uint32_t Size() const {return uSize_;}
				uint32_t Capacity() const {return uCapacity_;}
				
				void Put(T item);
				T Get();

				bool NonBlockingPut(T item);
				T NonBlockingGet();

				T RetryNonBlockingGet();

			private:
				uint32_t uCapacity_;
				uint32_t uSize_;

				std::list<T> queue_;

				sem_t sFull_;//初始化为队列容量，表示队列中当前可放入item的个数
				sem_t sEmpty_;//初始化为0，表示队列中当前可以使用的item的个数

				pthread_mutex_t mLock_;
		};

	
	template <class T>
		BlockingQueue<T>::BlockingQueue(uint32_t capacity)
			: uCapacity_(capacity), uSize_(0)
		{
			PthreadCall(pthread_mutex_init(&mLock_,NULL));
			SemCall(sem_init(&sEmpty_, 0, 0));
			SemCall(sem_init(&sFull_, 0, uCapacity_));
		}

	template <class T>
		BlockingQueue<T>::~BlockingQueue()
		{
			PthreadCall(pthread_mutex_destroy(&mLock_));
			SemCall(sem_destroy(&sEmpty_));
			SemCall(sem_destroy(&sFull_));
		}

	template <class T>
		void BlockingQueue<T>::Put(T item)
		{
			//是否有位置资源可以放入item?
			BLOCKINGOP(sem_wait(&sFull_));

			//put，下面操作涉及共享资源的操作queue_uSize_,要加锁
			PthreadCall(pthread_mutex_lock(&mLock_));
			queue_.push_back(item);
			uSize_++;
			PthreadCall(pthread_mutex_unlock(&mLock_));

			//更新信号量sEmpty_,表示队列中多了一个可以get的资源
			SemCall(sem_post(&sEmpty_));
		}

	template <class T>
		T BlockingQueue<T>::Get()
		{
			BLOCKINGOP(sem_wait(&sEmpty_));

			PthreadCall(pthread_mutex_lock(&mLock_));
			T item = queue_.front();
			queue_.pop_front();
			uSize_--;
			PthreadCall(pthread_mutex_unlock(&mLock_));

			SemCall(sem_post(&sFull_));
			return item;
		}

	template <class T>
		bool BlockingQueue<T>::NonBlockingPut(T item)
		{
			if(sem_trywait(&sFull_) == -1)
			{
				return false;	
			}

			PthreadCall(pthread_mutex_lock(&mLock_));
			queue_.push_back(item);
			uSize_++;
			PthreadCall(pthread_mutex_unlock(&mLock_));

			SemCall(&sEmpty_);
			return true;
		}

	template <class T>
		T BlockingQueue<T>::NonBlockingGet()
		{
			if(sem_trywait(&sEmpty_) == -1)
			{
				return NULL;
			}

			PthreadCall(pthread_mutex_lock(&mLock_));
			T item = queue_.front();
			queue_.pop_front();
			uSize_--;
			PthreadCall(pthread_mutex_unlock(&mLock_));

			SemCall(&sFull_);
			return item;
		}

	template <class T>
		T BlockingQueue<T>::RetryNonBlockingGet()
		{
			int nRetry = 0;
			int ret = 0;
			while((ret = sem_trywait(&sEmpty_) == -1) && nRetry++ < 5)
			{
				usleep(500000);
			}

			if(ret == -1)
			{
				return NULL;	
			}

			PthreadCall(pthread_mutex_lock(&mLock_));
			T item = queue_.front();
			queue_.pop_front();
			uSize_--;
			PthreadCall(pthread_mutex_unlock(&mLock_));

			SemCall(&sFull_);
			return item;
		}
}//namespace joeyutil

#endif //JOEYUTIL_UTIL_BLOCKINGQUEUE_H_
