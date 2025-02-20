#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

// Function declaration
void ThreadMethod(std::vector<int>& myVector, int start, int end, int threadIndex);

#define LEVEL 2  // Corrected macro definition
#define N 10       // Number of threads


int main()
{
#if LEVEL == 1

	const int vectorSize = 20;
	std::vector<int> myVector(vectorSize, 0);

	std::cout << "Before: ";
	for (int num : myVector) {
		std::cout << num << " ";
	}
	std::cout << std::endl;

	std::vector<std::thread> threads;
	int chunkSize = vectorSize / N; // Divide the vector into N equal parts

	// Create N worker threads, each modifying a separate range of the vector
	for (int i = 0; i < N; ++i)
	{
		int start = i * chunkSize;

		int end;
		if (i == N - 1)
		{
			end = vectorSize;  // Last thread covers the remaining elements
		}
		else
		{
			end = (i + 1) * chunkSize;  // Regular case
		}

		threads.push_back(std::thread([&, start, end, i]()
			{
				ThreadMethod(myVector, start, end, i);
			}));

	}

	// Join all threads
	for (auto& thread : threads) {
		thread.join();
	}

	std::cout << "After: ";
	for (int num : myVector) {
		std::cout << num << " ";
	}
	std::cout << std::endl;

#endif
#if LEVEL == 2

	std::mutex mtx;

	auto printMessage = [&mtx](int id)
		{
			//Lockguard scenario
			//std::lock_guard<std::mutex> lock(mtx);
			//std::cout << "Hello" << " from " << " thread " << id << "!" << std::endl;

			//Mutex Scenario
			mtx.lock();
			std::cout << "Hello" << " from " << " thread " << id << "!" << std::endl;
			mtx.unlock();
		};

	std::vector<std::thread> threads;
	for (int i = 0; i < N; ++i)
	{
		threads.emplace_back(printMessage, i);
	}

	for (auto& thread : threads)
	{
		thread.join();
	}


#endif
#if LEVEL == 3

#endif
#if LEVEL == 4

#endif
#if LEVEL == 5

#endif

	return 0;
}

// Function definition
void ThreadMethod(std::vector<int>& myVector, int start, int end, int threadIndex)
{
	for (int i = start; i < end; ++i) {
		myVector[i] = threadIndex;  // Increment elements by the thread index
	}
}
