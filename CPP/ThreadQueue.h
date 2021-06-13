#include <stdint.h>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <string.h>

template <class T, int MAX_SIZE>
class ThreadQueue
{
private:
    T elements[MAX_SIZE];
    int nextRead;
    int nextWrite;

    std::mutex mtx;
    std::condition_variable cv;
    int available;
public:

    ThreadQueue() :
    nextRead(0),
    nextWrite(0),
    available(0)
    {
    }

    // return true if can get before timeout
    //  else return false
    bool get(T* const out, const uint32_t timeout_ms){
        std::unique_lock<std::mutex> lk(mtx);

        if(!cv.wait_for(lk, std::chrono::milliseconds(timeout_ms), [&]{return available > 0;})){
            // timeout,
            return false;
        }

        memcpy(out, elements + nextRead, sizeof(T));
        nextRead = (nextRead + 1) % MAX_SIZE;
        available--;
        cv.notify_all();
        return true;
    }

    // return true if put before timeout
    //  else return false
    bool put(const T* const element, const uint32_t timeout_ms){
        std::unique_lock<std::mutex> lk(mtx);

        // printf("TIMEOUT?\n");

        if(!cv.wait_for(lk, std::chrono::milliseconds(timeout_ms), [&]{return available < MAX_SIZE;})){
            // timeout,
            return false;
        }

        memcpy(elements + nextWrite, element, sizeof(T));
        nextWrite = (nextWrite + 1) % MAX_SIZE;
        available++;
        cv.notify_all();
        return true;
    }
};