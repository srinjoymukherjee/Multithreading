//Shared Memory in Multithreading

#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <climits>
#include <condition_variable>
#include <fstream>

#define INF  INT_MAX/2
using namespace std;

mutex _mu;

condition_variable _cond;

enum class ret_status
{
	SUCCESS,
	FAILURE
};


class Logger
{
	mutex mu;
	once_flag flag;
	ofstream logfile;
public:
	//Below line will gurantee, that logfile will open once
	//by all threads
	void operator()(string log_str, int log_int)
	{
		call_once(flag, [&]() { logfile.open("pub_sub_log.txt"); });

		lock_guard<mutex> glock(mu, adopt_lock);
		logfile << "Thread id " << this_thread::get_id<<" :\t";
		logfile << log_str << log_int << endl;
	}
	~Logger()
	{
		logfile.close();
	}
};

class Publish
{
public:
	ret_status operator()(queue<int>& msg_que, vector<int> data_bus, Logger& log)
	{
		//After processing of Data Bus,
		//the termination condition for subscriber
		//will trigger
		data_bus.push_back(INF);

		try{
			for(auto data : data_bus)
			{
				unique_lock<mutex> ulock(_mu, defer_lock);

				ulock.lock();
				msg_que.push(data);
				log("Published:\t", data);
				ulock.unlock();
				
				/*
				Notify All other threads, who are waiting
				on this thread
				*/
				_cond.notify_all();
				
				
			}

		}catch(exception e)
		{
			cerr<<e.what()<<endl;
			return ret_status::FAILURE;
		}

		return ret_status::SUCCESS;
	}
};

class Subscribe
{
	// queue<int> msg_que;
public:
	ret_status operator()(queue<int>& msg_que, Logger& log)
	{
		// msg_que = m_que;
		int data = 0;
		try{
			while(1)
			{
				unique_lock<mutex> ulock(_mu);

				/*
				Waiting using the same _cond variable.

				Also, to protect Spurious wake,
				we are using one extra condition in the lambda function.
				*/
				_cond.wait(ulock, [=](){ return !msg_que.empty(); });

				data = msg_que.front();

				msg_que.pop();
				
				ulock.unlock();
				
				if(data == INF)
				{
					break;
				}
				
				log("Subscribed:\t", data);
			}
			
		}catch(exception e)
		{
			cerr<<e.what()<<endl;
			return ret_status::FAILURE;
		}

		return ret_status::SUCCESS;
	}
};

int main()
{
	try{
		queue<int> msg_que; //This will act as a shared memory
		Publish pub;
		Subscribe sub;
		vector<int> data_bus = {5, 9, 6, 8, 99, 999, 158};
		Logger log;

		thread t1(pub, ref(msg_que), data_bus, ref(log));
		thread t2(sub, ref(msg_que), ref(log));
	
		t1.join();
		t2.join();

	}catch(exception e)
	{
		cerr<<e.what()<<endl;
	}

	return 0;
	
}
