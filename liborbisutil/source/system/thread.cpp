#include "thread.h"
#include "logger.h"
#include "resolve.h"

namespace liborbisutil
{
	thread::thread(void(*func)(void* arg), std::string name, void* data, int priority, bool detach)
	{
		sceKernelCreateEventFlag(&event_flag, name.c_str(), SCE_KERNEL_EVF_ATTR_MULTI, 0, NULL);
		error = scePthreadAttrInit(&attributes);
		error = scePthreadAttrSetdetachstate(&attributes, detach ? SCE_PTHREAD_CREATE_DETACHED : SCE_PTHREAD_CREATE_JOINABLE);
		error = scePthreadAttrSetschedpolicy(&attributes, SCE_KERNEL_SCHED_FIFO);
		SceKernelSchedParam p = { 0 };
		p.sched_priority = SCE_KERNEL_PRIO_FIFO_DEFAULT - priority;
		error = scePthreadAttrSetschedparam(&attributes, &p);

		this->name = name;
		thread_func = func;
		thread_arg = data;
		if(!detach)
			is_joinable = true;

		handle = nullptr;

		int res = scePthreadCreate(&handle, &attributes, entry_point, reinterpret_cast<void*>(this), name.c_str());
		if (res < 0)
			printf("scePthreadCreate failed: 0x%08X\n", res);

		sceKernelSetEventFlag(event_flag, state::running);
	}

	thread::~thread()
	{
		if (handle)
		{
			if (is_running())
			{
				if (is_joinable)
					join();
			}
		}

		scePthreadAttrDestroy(&attributes);
		sceKernelDeleteEventFlag(event_flag);
	}

	void thread::stop()
	{
		if (handle)
		{
			sceKernelClearEventFlag(event_flag, ~state::running);
		}
	}
	void thread::join()
	{
		if (handle)
		{
			scePthreadJoin(handle, nullptr);
		}
	}
	void thread::detach()
	{
		if (handle)
		{
			scePthreadDetach(handle);
		}
	}
	bool thread::joinable() const
	{
		return is_joinable;
	}
	bool thread::is_running() const
	{
		return sceKernelPollEventFlag(event_flag, state::running, SCE_KERNEL_EVF_WAITMODE_AND, nullptr) == 0;
	}

	void* thread::entry()
	{
		error = sceKernelWaitEventFlag(event_flag, state::running, SCE_KERNEL_EVF_WAITMODE_AND, nullptr, nullptr);
		if (error < 0)
		{
			LOG_ERROR("Failed to wait for event flag: 0x%08X\n", error);
			return nullptr;
		}

		if (thread_func)
		{
			thread_func(thread_arg);
		}

		scePthreadExit(nullptr);
		return nullptr;
	}
	void thread::sleep(unsigned int ms)
	{
		sceKernelUsleep(ms * 1000);
	}

	void* thread::entry_point(void* arg)
	{
		return static_cast<thread*>(arg)->entry();
	}

	namespace threading
	{
		static bool use_system_library = false;
		static uintptr_t thread_offset = 0x0058248;

		uintptr_t resolve_kernel_base()
		{
			auto libkernel = resolve::get_module_address<uint64_t>("libkernel.sprx");
			if (!libkernel)
			{
				LOG_WARNING("Failed to resolve libkernel base address from libkernel.sprx... attempting to find system library\n");

				libkernel = resolve::get_module_address<uint64_t>("libkernel_sys.sprx");
				if (!libkernel)
				{
					LOG_ERROR("Failed to resolve libkernel base address from libkernel_sys.sprx, giving up.\n");
					return 0;
				}

				use_system_library = true;
				thread_offset = 0x0;
			}

			return libkernel;
		}

		posix_thread* get_thread(std::function<bool(posix_thread*)> callback)
		{
			auto resolved_kernel = resolve_kernel_base();
			if (!resolved_kernel)
				return nullptr;
			auto cur_thread = *reinterpret_cast<posix_thread**>(resolve::get_absolute_address(thread_offset, resolved_kernel));
			while (cur_thread != nullptr)
			{
				if(callback(cur_thread))
					return cur_thread;

				cur_thread = cur_thread->next;
			}

			return nullptr;
		}

		posix_thread* get_current()
		{
			return reinterpret_cast<posix_thread*>(scePthreadSelf());
		}

		posix_thread* get_by_id(int id)
		{
			auto resolved_kernel = resolve_kernel_base();
			if (!resolved_kernel)
				return nullptr;

			return get_thread([&](posix_thread* thread) {
				if (thread->tid == id)
					return true;

				return false;
				});
		}

		posix_thread* get_by_name(const std::string& name)
		{
			auto resolved_kernel = resolve_kernel_base();
			if (!resolved_kernel)
				return nullptr;

			return get_thread([&](posix_thread* thread) {
				if (get_name(thread) == name)
					return true;
				
				return false;
				});
		}

		std::string get_name(posix_thread* thread)
		{
			char threadName[256];
			auto res = sceKernelGetThreadName(thread->tid, threadName);
			if (res < 0)
			{
				LOG_ERROR("Failed to get thread name: 0x%08X\n", res);
				return "";
			}

			return threadName;
		}
		std::vector<posix_thread*> get_threads()
		{
			std::vector<posix_thread*> threads;

			auto resolved_kernel = resolve_kernel_base();
			if (!resolved_kernel)
				return threads;

			get_thread([&](posix_thread* thread) {
				threads.push_back(thread);

				return false;
				});

			return threads;
		}
		size_t count()
		{
			auto resolved_kernel = resolve_kernel_base();
			if (!resolved_kernel)
				return 0;

			size_t count;
			get_thread([&](posix_thread* thread) {
				count++;
				return false;
				});

			return count;
		}

		bool freeze(ScePthread thread, bool freeze_user_ctx)
		{
			if (freeze_user_ctx)
			{
				auto res = pthread_suspend_user_context_np(reinterpret_cast<posix_thread*>(thread));
				if (res < 0)
				{
					LOG_ERROR("Failed to freeze user context: 0x%08X\n", res);
					return false;
				}

				return true;
			}
			else
			{
				auto res = pthread_suspend_np(reinterpret_cast<posix_thread*>(thread));
				if (res < 0)
				{
					LOG_ERROR("Failed to freeze thread: 0x%08lX\n", res);
					return false;
				}
				return true;
			}
		}

		bool unfreeze(ScePthread thread, bool unfreeze_user_ctx)
		{
			if (unfreeze_user_ctx)
			{
				auto res = pthread_resume_user_context_np(reinterpret_cast<posix_thread*>(thread));
				if (res < 0)
				{
					LOG_ERROR("Failed to unfreeze user context: 0x%08X\n", res);
					return false;
				}
				return true;
			}
			else
			{
				auto res = pthread_resume_np(reinterpret_cast<posix_thread*>(thread));
				if (res < 0)
				{
					LOG_ERROR("Failed to unfreeze thread: 0x%08lX\n", res);
					return false;
				}
				return true;
			}
		}

		std::vector<posix_thread*> freeze(bool freeze_user_ctx, bool freeze_current)
		{
			auto threads = get_threads();

			for (auto thread : threads)
			{
				if(!freeze_current)
					if (thread == get_current())
						continue;

				if (!freeze(reinterpret_cast<ScePthread>(thread), freeze_user_ctx))
					threads.erase(std::remove(threads.begin(), threads.end(), thread), threads.end());
			}

			return threads;
		}

		void unfreeze(const std::vector<posix_thread*>& threads, bool unfreeze_user_ctx)
		{
			for (auto thread : threads)
			{
				unfreeze(reinterpret_cast<ScePthread>(thread), unfreeze_user_ctx);
			}
		}

		ScePthread create(const std::string& name, void* (*func)(void*), bool joinable, void* arg)
		{
			ScePthread thr;
			auto res = scePthreadCreate(&thr, nullptr, func, arg, name.c_str());
			if (res < 0)
			{
				LOG_ERROR("Failed to create thread: 0x%08X\n", res);
				return thr;
			}

			if (joinable)
			{
				res = scePthreadJoin(thr, nullptr);
				if (res < 0)
				{
					LOG_ERROR("Failed to join thread: 0x%08X\n", res);
					return thr;
				}
			}

			return thr;
		}
		bool join(ScePthread thread)
		{
			auto res = scePthreadJoin(thread, nullptr);
			if (res < 0)
			{
				LOG_ERROR("Failed to join thread: 0x%08X\n", res);
				return false;
			}
			return true;
		}

		bool detach(ScePthread thread)
		{
			auto res = scePthreadDetach(thread);
			if (res < 0)
			{
				LOG_ERROR("Failed to detach thread: 0x%08X\n", res);
				return false;
			}
			return true;
		}

		void sleep(unsigned int ms)
		{
			sceKernelUsleep(ms * 1000);
		}
	}

	namespace thread_pool
	{
		void thread_worker()
		{
			while (true)
			{
				try
				{
					std::function<void()> job;
					{
						std::unique_lock<std::mutex> lock(internal::mtx);
						internal::cv.wait(lock, []()
							{
								return !internal::tasks.empty() || !internal::is_running;
							});

						if (!internal::is_running && internal::tasks.empty())
							return;

						job = std::move(internal::tasks.front());
						internal::tasks.pop();
					}

					if (job)
						job();
				}
				catch (const std::exception& e)
				{
					LOG_ERROR("ThreadWorker Exception: %s\n", e.what());
				}
				catch (...)
				{
					LOG_ERROR("ThreadWorker Exception: Unknown\n");
					break;
				}
			}
		}

		void initialize(int count)
		{
			internal::workers.resize(count);

			LOG_INFO("Starting thread pool with %i threads\n", count);

			internal::is_running = true;
			
			int i = 0;
			for (auto& worker : internal::workers)
			{
				auto thread_name = string::format("liborbisutil_thread_%i", i);

				auto initializer = [=]()
					{
						LOG_WARNING("Starting thread: %s\n", thread_name.data());

						thread_worker();

						LOG_WARNING("Thread %s has stopped\n", thread_name.data());
					};

				worker = std::thread(thread_name.c_str(), initializer);

				int cpu = 5;
				const static uint cAllCoresMask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5);
				uint mask = (cpu >= 0) ? (1 << cpu) : cAllCoresMask;

				auto native_handle = worker.native_handle();
				scePthreadSetaffinity(native_handle, mask);
				SceKernelSchedParam p = { SCE_KERNEL_PRIO_FIFO_DEFAULT - 15 };
				scePthreadSetschedparam(native_handle, SCE_KERNEL_SCHED_FIFO, &p);

				i++;
			}
		}
		void finalize()
		{
			if(!internal::is_running)
				return;

			LOG_WARNING("Stopping thread pool\n");
			{
				std::unique_lock<std::mutex> lock(internal::mtx);

				internal::mtx.unlock();
				internal::is_running = false;
			}
			
			internal::cv.notify_all();

			for (auto& worker : internal::workers)
			{
				if (worker.joinable())
					worker.join();
			}

			internal::workers.clear();

			LOG_SUCCESS("Thread pool stopped\n");
		}
	}
}