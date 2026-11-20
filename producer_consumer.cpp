#include <iostream>
#include <queue>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <chrono>

const int BUFFER_CAPACITY = 5;
std::queue<int> buffer;
std::mutex m;
std::condition_variable not_full, not_empty;
bool is_finished = false;

void producer()
{
	for (int num = 1; num <= 20 ; num ++)
	{
		std::unique_lock<std::mutex> lock(m);
		not_full.wait(lock, []{ return buffer.size() < BUFFER_CAPACITY;});
		
		buffer.push(num);
		std::cout << "Producer : " << buffer.size() << std::endl;
		
		lock.unlock();
		not_empty.notify_one();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	{
		std::lock_guard<std::mutex> lock(m);
		is_finished = true;
	}

	not_empty.notify_all();
}

void consumer()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(m);
		not_empty.wait(lock, []{ return !buffer.empty() || is_finished;});
		if (buffer.empty() && is_finished) break;
		int value = buffer.front();
		buffer.pop();
		// if ((std::this_thread::get_id()) != 1)
		 std::cout << "Consumer id " << std::this_thread::get_id() << " : "<< buffer.size() << std::endl;
		lock.unlock();
		not_full.notify_one();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}
int main(int argc, char const *argv[])
{
	std::thread p(producer), c1(consumer), c2(consumer);
	p.join();
	c2.join();
	c1.join();
	return 0;
}