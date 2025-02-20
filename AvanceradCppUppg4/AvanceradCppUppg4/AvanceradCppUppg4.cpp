#include <iostream>
#include <vector>
#include <thread>

// Function declaration
void ThreadMethod(std::vector<int>& myVector);


int main()
{
    std::vector<int> myVector(20, 0);

    std::cout << "Before: ";
    for (int num : myVector) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    // Create and start worker thread, passing the vector by reference
    std::thread workerThread(ThreadMethod, std::ref(myVector));

    // Ensure the worker thread completes before program exits
    workerThread.join();

    std::cout << "After: ";
    for (int num : myVector) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}

// Function definition
void ThreadMethod(std::vector<int>& myVector)
{
    for (int& num : myVector) {
        num++;
    }
}
