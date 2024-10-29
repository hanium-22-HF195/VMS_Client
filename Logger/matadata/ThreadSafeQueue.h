#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

template<typename T>
class ThreadSafeQueue {
private:
    queue<T> queue;
    mutable mutex mtx;  // mutable로 선언하여 const 함수에서도 락을 걸 수 있게 함

public:
    ThreadSafeQueue() = default;

    // 값 추가
    void push(const T& value) {
        mtx.lock();  
        queue.push(value);
        mtx.unlock(); 
    }

    // 값 가져오기 시도, 성공하면 result에 값 저장 후 true 반환
    bool try_pop(T& result) {
        mtx.lock(); 
        if (queue.empty()) {
            mtx.unlock(); 
            return false;
        }
        result = queue.front();
        queue.pop();
        mtx.unlock(); 
        return true;
    }

    // 큐가 비어있는지 확인
    bool empty() const {
        mtx.lock(); 
        bool is_empty = queue.empty();
        mtx.unlock(); 
        return is_empty;
    }

    // 큐 크기 반환
    size_t size() const {
        mtx.lock(); 
        size_t size = queue.size();
        mtx.unlock();
        return size;
    }

    // front에 접근 (읽기 전용)
    T front() const {
        mtx.lock();
        T result = queue.front();
        mtx.unlock();
        return result;
    }
};

#endif // THREADSAFEQUEUE_H
