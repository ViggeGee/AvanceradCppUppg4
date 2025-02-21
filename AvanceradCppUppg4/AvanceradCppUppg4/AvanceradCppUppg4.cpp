#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>
#include <chrono>
#include <algorithm>

// Function declaration
void ThreadMethod(std::vector<int>& myVector, int start, int end, int threadIndex);

void ThreadMethodTwo(const std::vector<std::string>& lines, int start, int end, const std::string& searchTerm, int threadIndex);

std::mutex mtx;
int totalMatchCount = 0;
#define LEVEL 4  // Corrected macro definition
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

	std::vector<std::thread> consumerThreads;
	std::vector<std::thread> baristaThreads;

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
