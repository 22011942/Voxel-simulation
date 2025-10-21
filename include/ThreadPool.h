#pragma once

#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <future>
#include <type_traits>

class ThreadPool {
private:
	std::vector<std::thread> workers;

	std::mutex mutex;

	std::condition_variable cv;

	std::queue<std::function<void()>> queue;

	void worker();

	bool stop;

public:
	ThreadPool(std::size_t nr_threads = std::thread::hardware_concurrency());
	~ThreadPool();

	template<typename F, typename... Args>
	auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

	ThreadPool(ThreadPool&) = delete;
	ThreadPool(const ThreadPool&) = delete;

	ThreadPool& operator=(ThreadPool&&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
};

template<typename F, typename... Args>
inline auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {

	auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
	auto encapsulated_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

	std::future<typename std::invoke_result<F, Args...>::type> future_object = encapsulated_ptr->get_future();

	{
		std::unique_lock<std::mutex> lock(mutex);
		queue.emplace([encapsulated_ptr]() {
			(*encapsulated_ptr)(); // execute the fx
			});
	}
	cv.notify_one();
	return future_object;
}