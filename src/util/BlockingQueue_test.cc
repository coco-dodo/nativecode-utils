#include "util/BlockingQueue.h"

#include <iostream>
#include <pthread.h>
using namespace joeyutil;

static BlockingQueue<int> queue(10);	
static void *func(void *)
{
	while(1)
	{
		int item = queue.Get();
		std::cout <<"pop:" << item << std::endl;
	}
}

int main()
{
	pthread_t pid;
	pthread_create(&pid, NULL, func, NULL);

	queue.Put(1);
	queue.Put(2);

	pthread_join(pid, NULL);

	std::cout << "finish!" << std::endl;

	return 0;
}
