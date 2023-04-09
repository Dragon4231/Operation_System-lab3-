#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std;

const int MAX_THREADS = 10;

int* arr;
int sizeL;
bool* markers;
vector<thread*> threads;
mutex mtx;
condition_variable cv;

void marker_thread(int num) {
    srand(num);
    int marked_count = 0;
    int impossible_index = -1;

    unique_lock<mutex> lck(mtx);
    cv.wait(lck);

    while (true) {
        int num = rand();
        int index = num % sizeL;

        if (markers[index]) {
            continue;
        }

        if (arr[index] != 0) {
            impossible_index = index;
            cout << "Marker " << num << " encountered an impossible index: " << index << endl;
            cv.notify_one();
            cv.wait(lck);
        }
        else {
            markers[index] = true;
            marked_count++;
            cout << "Marker " << num << " marked index " << index << endl;
            lck.unlock();
            this_thread::sleep_for(chrono::milliseconds(5));
            arr[index] = num;
            this_thread::sleep_for(chrono::milliseconds(5));
            lck.lock();
        }
    }
}

int main() {
    cout << "Enter array size: ";
    cin >> sizeL;

    arr = new int[sizeL];
    markers = new bool[sizeL];

    for (int i = 0; i < sizeL; i++) {
        arr[i] = 0;
        markers[i] = false;
    }

    int num_threads;

    cout << "Enter number of marker threads (up to " << MAX_THREADS << "): ";
    cin >> num_threads;

    num_threads = min(num_threads, MAX_THREADS); // if user write num more than max

    for (int i = 0; i < num_threads; i++) {
        threads.push_back(new thread(marker_thread, i));
    }

    cout << "Starting marker threads..." << endl;

    this_thread::sleep_for(chrono::seconds(1));

    cv.notify_all();

    while (true) {
        {
            unique_lock<mutex> lck(mtx);
            int count = 0;

            for (int i = 0; i < sizeL; i++) {
                if (!markers[i]) {
                    count++;
                }
            }

            if (count == 0) {
                break;
            }
        }

        this_thread::sleep_for(chrono::milliseconds(10));
    }

    cout << "Array contents after all markers stopped:" << endl;

    for (int i = 0; i < sizeL; i++) {
        cout << arr[i] << " ";
    }

    cout << endl;

    cout << "All marker threads stopped." << endl;

    return 0;
}