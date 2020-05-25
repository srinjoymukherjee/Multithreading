//The output of this program will be a mess,
//as both the thread, main and t1 will try to use 
//the STDOUT concurrently

#include <iostream>
#include <thread>

using namespace std;

class ThreadFunc
{
	public:
	void operator()(string thrd)
	{
		for(size_t i = 0; i < 10; i++)
			cout<<"From "<<thrd<<" :\t"<<i<<endl;
	}
};

int main()
{
	ThreadFunc func;
	thread t1(func, "t1");
	
	for(size_t i = 0; i < 10; i++)
			cout<<"From main:\t"<<i<<endl;
			
	t1.join();
	return 0;
}
