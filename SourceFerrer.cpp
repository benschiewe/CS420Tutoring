#include<iostream>
#include<thread>
#include<fstream>
#include<chrono>

using namespace std;

void fileToMemoryTransfer(const char* fileName, char** data, size_t& numOfBytes)
{
	ifstream inFile(fileName, ios::binary | ios::ate);
	if (!inFile)
	{
		cerr << "Error opening file" << endl;
		exit(1);
	}

	size_t size = inFile.tellg();
	char* buffer = new char[size];
	inFile.seekg(0, inFile.beg);
	inFile.read(buffer, size);
	inFile.close();

	*data = buffer;
	numOfBytes = size;
}

void updateHistogram(char* data, size_t start, size_t end, uint64_t* histogram)
{
	for (size_t i = start; i < end; i++)
	{
		histogram[data[i]]++;
	}
}


int main()
{
	const char* filename = "test.txt";
	char* data;
	size_t numOfBytes;
	fileToMemoryTransfer(filename, &data, numOfBytes);

	uint64_t histogram[256] = { 0 };
	const size_t numOfThreads = 8;
	thread threads[numOfThreads];
	size_t chunkSize = numOfBytes / numOfThreads;

	auto start = chrono::high_resolution_clock::now();
	for (size_t i = 0; i < numOfThreads; i++)
	{
		size_t start = i * chunkSize;
		size_t end = (i + 1) * chunkSize;

		//account for not evenly divisible data
		if (i == numOfThreads - 1)
		{
			end = numOfBytes;
		}

		threads[i] = thread(updateHistogram, data, start, end, histogram);
	}

	for (size_t i = 0; i < numOfThreads; i++)
	{
		threads[i].join();
	}

	auto end = chrono::high_resolution_clock::now();

	for (size_t i = 0; i < 256; i++)
	{
		cout << i << " : " << histogram[i] << endl;
	}

	cout << "Time taken: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

	// local histogram for each thread

	uint64_t localHistogram[numOfThreads][256] = { 0 };

	start = chrono::high_resolution_clock::now();

	for (size_t i = 0; i < numOfThreads; i++)
	{
		size_t start = i * chunkSize;
		size_t end = (i + 1) * chunkSize;
		//account for not evenly divisible data
		if (i == numOfThreads - 1)
		{
			end = numOfBytes;
		}
		threads[i] = thread(updateHistogram, data, start, end, localHistogram[i]);
	}

	for (size_t i = 0; i < numOfThreads; i++)
	{
		threads[i].join();
	}

	// combine local histograms
	for (size_t i = 0; i < 256; i++)
	{
		uint64_t total = 0;
		for (size_t j = 0; j < numOfThreads; j++)
		{
			total += localHistogram[j][i];
		}
	}

	end = chrono::high_resolution_clock::now();

	// print unified histogram
	for (size_t i = 0; i < 256; i++)
	{
		cout << i << " : " << histogram[i] << endl;
	}

	

	cout << "Time taken: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;


	delete[] data;


	return 0;
}