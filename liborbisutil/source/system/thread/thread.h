#pragma once

// Thread utilities - standard includes provided by stdafx.h

using posix_thread = thread;

namespace liborbisutil
{
	class thread
	{
	public:
		thread() = default;
		thread(void(*func)(void* arg), std::string name, void* data = nullptr, int priority = 0, bool detach = false);
		~thread();
		void stop();
		void join();
		void detach();
		bool joinable() const;
		bool is_running() const;
		void* entry();
		static void sleep(unsigned int ms);

		static void* entry_point(void* arg);

		enum state
		{
			running = 1
		};
	private:
		ScePthread handle;
		std::string name;
		void(*thread_func)(void* arg);
		void* thread_arg;
		ScePthreadAttr attributes;
		SceKernelEventFlag event_flag;
		int error;

		bool is_joinable;
	};

	namespace threading
	{
		posix_thread* get_current();

		posix_thread* get_by_id(int id);
		posix_thread* get_by_name(const std::string& name);

		std::string get_name(posix_thread* thread);

		std::vector<posix_thread*> get_threads();
		size_t count();

		bool freeze(ScePthread thread, bool freeze_user_ctx = true);
		bool unfreeze(ScePthread thread, bool unfreeze_user_ctx = true);

		std::vector<posix_thread*> freeze(bool freeze_user_ctx = true, bool freeze_current = false);
		void unfreeze(const std::vector<posix_thread*>& threads, bool unfreeze_user_ctx = true);

		ScePthread create(const std::string& name, void* (*func)(void*), bool joinable, void* arg = nullptr);
		bool join(ScePthread thread);
		bool detach(ScePthread thread);

		void sleep(unsigned int ms);
	}

	namespace thread_pool
	{
		namespace internal
		{
			inline bool is_running = false;
			inline std::mutex mtx;
			inline std::condition_variable cv;
			inline std::queue<std::function<void()>> tasks;
			inline std::vector<std::thread> workers;
		}

        /**
        * @brief Push a new task into the thread pool.
        * 
        * This function accepts a callable object and its arguments, then creates a packaged task
        * and pushes it into the internal task queue. It returns a future object that can be used
        * to retrieve the result of the task once it has been executed.
        * 
        * @tparam Func The type of the callable object.
        * @tparam Args The types of the arguments to the callable object.
        * @param func The callable object to be executed.
        * @param args The arguments to be passed to the callable object.
        * @return std::future<decltype(func(args...))> A future object to retrieve the result of the task.
        */
        template<typename Func, typename... Args>
		auto push(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
			using return_type = decltype(func(args...));

			if (!internal::is_running)
				return std::future<return_type>();  // Return an empty future if not running

			auto task = std::make_shared<std::packaged_task<return_type()>>(
				std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
			);

			std::future<return_type> res = task->get_future();
			{
				std::unique_lock<std::mutex> lock(internal::mtx);
				internal::tasks.emplace([task]() { (*task)(); });
			}

			internal::cv.notify_one();
			return res;
		}

		void initialize(int count = std::thread::hardware_concurrency());
		void finalize();
	}
}