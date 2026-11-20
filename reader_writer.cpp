#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <queue>
using namespace std;

int shared_value = 0;
condition_variable ok_to_write, ok_to_read;
mutex m;
int readers = 0, writers = 0;
queue<thread::id> waiting_queue;
void writer()
{

	for (int i = 0 ; i < 3 ; i++)
	{
		unique_lock<mutex> lock(m);
		waiting_queue.push(this_thread::get_id());
		ok_to_write.wait(lock, []{ return readers == 0 && writers == 0 && waiting_queue.front() == this_thread::get_id();});
		writers += 1;
		waiting_queue.pop();
		lock.unlock();
		
		shared_value = rand();
		
		lock.lock();
		writers -= 1;
		
		 if (writers == 0)
		 {
		 	ok_to_write.notify_all();
		 	ok_to_read.notify_all();	
		 }
		 
		
		lock.unlock();
		this_thread::sleep_for(chrono::milliseconds(5000));
	}
}

void reader()
{
	for (int i = 0 ; i < 5 ; i++)
	{
		unique_lock<mutex> lock(m);
		waiting_queue.push(this_thread::get_id());
		ok_to_read.wait(lock, []{ return writers == 0 && waiting_queue.front() == this_thread::get_id();});
		readers += 1;
		waiting_queue.pop();	
		lock.unlock();
	
		cout << shared_value << endl;
	
		lock.lock();
		readers -= 1;
		if (readers == 0 && writers == 0) ok_to_write.notify_all();
		else if (writers == 0) ok_to_read.notify_all();

		lock.unlock();
		this_thread::sleep_for(chrono::milliseconds(2000));
	}
	
}

void threading_test()
{
	vector<thread> reader_threads, writer_threads;
	for (int i = 0 ; i < 20 ; i++)
	{
		thread t1(reader);
		reader_threads.push_back(move(t1));
	}

	for (int i = 0 ; i < 10 ; i++)
	{
		thread t1(writer);
		writer_threads.push_back(move(t1));
	}

	for (int i = 0 ; i < 20 ; i++) reader_threads[i].join();
	for (int i = 0 ; i < 10 ; i++) writer_threads[i].join();
}


int main(int argc, char const *argv[])
{
	// thread readerThreads[20], writerThreads[10];
	int x = 100;
	while (x--) threading_test();
	return 0;
}