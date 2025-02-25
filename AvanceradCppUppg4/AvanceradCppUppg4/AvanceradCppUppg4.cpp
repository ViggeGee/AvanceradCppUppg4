#include <iostream>
#include <vector>
#include <thread>
#include <fstream>
#include <string>
#include <chrono>
#include <algorithm>
#include <queue>
#include <mutex>
#include <condition_variable>

// Function declaration
void ThreadMethod(std::vector<int>& myVector, int start, int end, int threadIndex);

void ThreadMethodTwo(const std::vector<std::string>& lines, int start, int end, const std::string& searchTerm, int threadIndex);

std::mutex mtx;
int totalMatchCount = 0;
#define LEVEL 4  // Corrected macro definition
#define N 10       // Number of threads

struct Order
{
	int id;
	int processingTime; // Time in milliseconds
};

void customerThread(int id);
void baristaThread(int id);

std::queue<Order> orderQueue;
std::mutex queueMutex;
std::condition_variable queueCV;
std::mutex printMutex;
const int MAX_QUEUE_SIZE = 5;


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
	std::ifstream file("bible.txt");
	if (!file.is_open()) {
		std::cerr << "Error: Could not open the file." << std::endl;
		return 1;
	}

	std::vector<std::string> lines;
	std::string line;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	file.close();


	std::cout << "Nbr of threads used for searching, N = " << N << std::endl;

	// Calculate the number of lines per thread
	int totalLines = lines.size();
	int linesPerThread = totalLines / N;
	std::cout << "Lines per thread: " << linesPerThread << std::endl;

	std::string searchTerm = "apple";
	std::cout << "Searching bible.txt for '" << searchTerm << "'" << std::endl;

	std::vector<std::thread> threads;

	// Start the timer for the entire search
	auto startTime = std::chrono::high_resolution_clock::now();

	// Create N worker threads, each searching a portion of the text
	for (int i = 0; i < N; ++i) {
		int start = i * linesPerThread;
		int end = (i == N - 1) ? totalLines : (i + 1) * linesPerThread;

		threads.emplace_back(ThreadMethodTwo, std::ref(lines), start, end, std::ref(searchTerm), i);
	}

	// Join all threads
	for (auto& thread : threads) {
		thread.join();
	}

	// Calculate the total execution time
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
	std::cout << "Total execution time: " << duration << "ms" << std::endl;
	std::cout << "Total match count is: " << totalMatchCount << std::endl;


#endif
#if LEVEL == 4

	const int numCustomers = 5;
	const int numBaristas = 2;

	std::vector<std::thread> customerThreads;
	std::vector<std::thread> baristaThreads;

	for (int i = 0; i < numCustomers; ++i) {
		customerThreads.emplace_back(customerThread, i + 1);
	}

	for (int i = 0; i < numBaristas; ++i) {
		baristaThreads.emplace_back(baristaThread, i + 1);
	}

	for (auto& t : customerThreads) {
		t.join();
	}

	for (auto& t : baristaThreads) {
		t.join();
	}

#endif

	return 0;
}


void customerThread(int id) 
{
	while (true) 
	{
		Order order;
		order.id = id;
		order.processingTime = rand() % 5000 + 1000;


		//Wait in queue until theres room for a new order
		std::unique_lock<std::mutex> lock(queueMutex);
		queueCV.wait(lock, [] { return orderQueue.size() < MAX_QUEUE_SIZE; });

		//Push new order to orderqueue when room is there
		orderQueue.push(order);
		{
			//Print info. Lock to ensure that only one thread writes at a time.
			std::lock_guard<std::mutex> printLock(printMutex);
			std::cout << "Customer " << id << " placed order: " << order.id << std::endl;
			std::cout << "Number of placed orders: " << orderQueue.size() << std::endl;
		}

		//Notify barista threads that new order is placed
		queueCV.notify_all();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000)); 
	}
}

void baristaThread(int id) 
{
	while (true)
	{
		//Wait until there is an order in the queue
		std::unique_lock<std::mutex> lock(queueMutex);
		queueCV.wait(lock, [] { return !orderQueue.empty(); });

		//Take the first order and pop it from the queue
		Order order = orderQueue.front();
		orderQueue.pop();
		{
			//Print info. Lock to ensure that only one thread writes at a time.
			std::lock_guard<std::mutex> printLock(printMutex);
			std::cout << "Barista " << id << " started order: " << order.id << std::endl;
		}

		//Notify customers that there is new room in queue
		queueCV.notify_all();
		lock.unlock();

		int progress = 0;
		while (progress < 100) {
			std::this_thread::sleep_for(std::chrono::milliseconds(order.processingTime / 5));
			progress += 20;
			{
				//Print info. Lock to ensure that only one thread writes at a time.
				std::lock_guard<std::mutex> printLock(printMutex);
				std::cout << "Barista " << id << " - Order " << order.id << " progress: " << progress << "%" << std::endl;
			}
		}

		{			
			//Print info. Lock to ensure that only one thread writes at a time.
			std::lock_guard<std::mutex> printLock(printMutex);
			std::cout << "Barista " << id << " completed order: " << order.id << std::endl;
		}
	}
}
// Function definition
void ThreadMethod(std::vector<int>& myVector, int start, int end, int threadIndex)
{
	for (int i = start; i < end; ++i) {
		myVector[i] = threadIndex;  // Increment elements by the thread index
	}
}

void ThreadMethodTwo(const std::vector<std::string>& lines, int start, int end, const std::string& searchTerm, int threadIndex)
{
	
	// Start the timer for this thread
	auto startTime = std::chrono::high_resolution_clock::now();

	int matchCount = 0;

	// Search each line in the assigned range
	for (int i = start; i < end; ++i) {
		size_t pos = lines[i].find(searchTerm);
		while (pos != std::string::npos) {
			matchCount++;
			pos = lines[i].find(searchTerm, pos + searchTerm.size());
		}
	}

	// Calculate the execution time for this thread
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	// Print the results for this thread
	mtx.lock();
	std::cout << "Thread ID: " << std::this_thread::get_id() << " found " << matchCount << " occurrences of '" << searchTerm << "' in " << duration << "ms" << std::endl;
	std::cout<<std::endl;

	totalMatchCount += matchCount;
	mtx.unlock();
}
