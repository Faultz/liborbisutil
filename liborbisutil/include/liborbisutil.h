#pragma once

#include <stdio.h>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <mutex>
#include <set>
#include <unordered_map>
#include <memory>
#include <functional>
#include <queue>
#include <condition_variable>
#include <future>
#include <csetjmp>

#include <gnmx.h>
#include <kernel.h>
#include <KernelExt.h>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/os.h>

#include <libhttp.h>
#include <net.h>

#include <pad.h>

#define LIBORBISUTIL_HTTP_CTX_DEFAULT -1

using posix_thread = thread;

inline uintptr_t process_base_address = []() -> uintptr_t {
	if (process_base_address)
		return process_base_address;

	SceKernelModuleInfo processModuleInfo;
	processModuleInfo.size = sizeof(SceKernelModuleInfo);
	if (sceKernelGetModuleInfo(0, &processModuleInfo) < 0)
	{
		printf("Failed to get process module info\n");
		return 0;
	}

	SceAppInfo appInfo;
	sceKernelGetAppInfo(getpid(), &appInfo);

	printf("loaded liborbisutil on %s\n", appInfo.TitleId);

	return reinterpret_cast<uintptr_t>(processModuleInfo.segmentInfo->address);
	}();

namespace liborbisutil
{
	namespace logger
	{
		namespace internal
		{
			enum level
			{
				info,
				success,
				warning,
				error,
				debug,
				verbose,
				max
			};

			const char* get_color(level lvl);
			const char* get_prefix(level lvl);

			// Log a message with the specified level, file, and line
			template <typename... Args>
			void log(level lvl, const char* file, int line, std::string format, Args... args);
		}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
		template<typename ...Args>
		void internal::log(level lvl, const char* file, int line, std::string format, Args ...args)
		{
			try
			{
				std::string color = get_color(lvl);
				std::string prefix = get_prefix(lvl);

				auto size = static_cast<size_t>(std::snprintf(nullptr, 0, format.data(), args...) + 1);

				std::unique_ptr<char[]> buffer(new char[size]);
				snprintf(buffer.get(), size, format.c_str(), args...);

				// Format the message to include file and line information
				std::string full_str = "[" + color + std::string(prefix) + "/" + std::string(file) + ":" + std::to_string(line) + "\033[0m] " + buffer.get();

				printf("%s", full_str.c_str());
			}
			catch (std::exception& e)
			{
				printf("Failed to format string: %s\n", e.what());
			}
		}
#pragma clang diagnostic pop

		// Log a message with the specified level, file, and line
		template <typename... Args>
		void info(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::info, file, line, format, args...);
		}

		template <typename... Args>
		void success(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::success, file, line, format, args...);
		}

		template <typename... Args>
		void warning(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::warning, file, line, format, args...);
		}

		template <typename... Args>
		void error(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::error, file, line, format, args...);
		}

		template <typename... Args>
		void debug(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::debug, file, line, format, args...);
		}

		template <typename... Args>
		void verbose(const char* file, int line, std::string format, Args... args)
		{
			internal::log(internal::verbose, file, line, format, args...);
		}

		// Log a message with the specified level, file, and line
		template <typename... Args>
		void log(internal::level lvl, const char* file, int line, std::string format, Args... args)
		{
			internal::log(lvl, file, line, format, args...);
		}
	}

	#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
	// Macros to simplify logging calls
	#define LOG_INFO(format, ...) liborbisutil::logger::info(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
	#define LOG_SUCCESS(format, ...) liborbisutil::logger::success(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
	#define LOG_WARNING(format, ...) liborbisutil::logger::warning(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
	#define LOG_ERROR(format, ...) liborbisutil::logger::error(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
	#define LOG_DEBUG(format, ...) liborbisutil::logger::debug(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
	#define LOG_VERBOSE(format, ...) liborbisutil::logger::verbose(__FILENAME__, __LINE__, format, ##__VA_ARGS__)
	#define LOG(level, format, ...) liborbisutil::logger::log(level, __FILENAME__, __LINE__, format, ##__VA_ARGS__)

	namespace string
	{
		std::string to_lower(const std::string& str);
		std::string to_upper(const std::string& str);

		std::wstring to_wide(const std::string& str);
		std::string to_narrow(const std::wstring& str);

		bool begins_with(const std::string& str, const std::string& prefix);
		bool ends_with(const std::string& str, const std::string& suffix);

		bool contains(const std::string& str, const std::string& sub, const bool sensitive = true);
		bool contains(const std::string& str, const std::vector<std::string>& subs, const bool sensitive = true);

		std::string join(const std::vector<std::string>& strings, const std::string& delimiter = "\0");
		std::vector<std::string> tokenize(const std::string& str, char delimiter = '\0');
		std::vector<std::string> split(const std::string& str, const std::string& delimiter = "\0");
		std::string concatenate(const std::vector<std::string>& strings, char delimiter, size_t idx = 0);

		std::string reverse(const std::string& str);
		std::string replace(const std::string& str, const std::string& from, const std::string& to);
		std::string erase(const std::string& str, const std::string& sub);

		const char** vector_to_pointer_array(const std::vector<std::string>& strings);
		std::vector<std::string> pointer_array_to_vector(const char* const* strings, size_t count);

		bool is_numeric(const std::string& str);
		bool is_integer(const std::string& str);
		bool is_alphabetic(const std::string& str);


		template <typename T>
		std::string to_string(const T& value)
		{
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}

		template <typename T>
		T from_string(const std::string& str)
		{
			std::istringstream iss(str);
			T value;
			iss >> value;
			return value;
		}

		template <typename ...Args>
		std::string format(const std::string& fmt, Args... args)
		{
			try
			{
				size_t size = std::snprintf(nullptr, 0, fmt.data(), args...) + 1;
				std::unique_ptr<char[]> buffer(new char[size]);
				std::snprintf(buffer.get(), size, fmt.data(), args...);
				return std::string(buffer.get(), buffer.get() + size - 1);
			}
			catch (std::exception e)
			{
				printf("Failed to format string: %s\n", e.what());
				return {};
			}
		}

		template<typename ...Args>
		std::string va(const std::string& fmt, Args... args)
		{
			return format(fmt, args...);
		}


		template <typename ...Args>
		std::wstring format(const std::wstring& fmt, Args... args)
		{
			try
			{
				size_t size = std::swprintf(nullptr, 0, fmt, args...) + 1;
				std::unique_ptr<wchar_t[]> buffer(new wchar_t[size]);
				std::swprintf(buffer.get(), size, fmt, args...);
				return std::wstring(buffer.get(), buffer.get() + size - 1);
			}
			catch (std::exception e)
			{
				printf("Failed to format string: %s\n", e.what());
				return {};
			}
		}
	}

	class file_system
	{
	public:
		file_system() = default;
		file_system(std::string file, std::string mode);
		file_system(std::string file, int mode);

		~file_system();

		file_system& operator<<(const char* buf);
		file_system& operator<<(const std::string& buf);
		file_system& operator<<(const int& buf);
		file_system& operator<<(const float& buf);
		file_system& operator<<(const double& buf);

		operator bool();
		operator FILE* () { return reinterpret_cast<FILE*>(this); }

		bool is_open();
		bool is_good();

		size_t get_size();

		std::string read_to_end();

		int open(const std::string& path, std::string mode);
		int open(const std::string& path, int mode);
		size_t read(const void* buf, size_t length, size_t count = 1);
		size_t readOffset(void* data, size_t length, size_t offset);
		size_t print(const char* format, ...);
		size_t write(const void* buf, size_t length, size_t count = 1);
		size_t seek(size_t offset, int whence);
		size_t tell();

		bool getline(std::string& line, char delimeter = '\n');

		void close();

		std::string name;
		int fd;
		size_t offset;
		bool end_of_file;
		int mode;
		size_t length;
	};

	class directory_iterator
	{
	public:

		class Iterator
		{
		public:
			Iterator(directory_iterator& parent, bool end = false)
				: parent_dir(parent), current_index(end ? parent.files.size() : 0) {
			}

			Iterator& operator++();

			const std::string& operator*() const;
			bool operator!=(const Iterator& other) const;

			directory_iterator& parent_dir;
			size_t current_index;
		};

		directory_iterator(const std::string& directory, bool recursive = false);
		~directory_iterator();

		Iterator begin();
		Iterator end();

		void scan_directory();

		std::string directory_path;
		std::vector<std::string> files;
		bool recurse;
	};

	class path
	{
	public:
		path() = default;
		explicit path(const std::string& path) : path_str(path) {}

		path operator/(const std::string& new_path);
		path& operator/=(const std::string& new_path);

		path operator/(const path& new_path);
		path& operator/=(const path& new_path);

		std::string str() { return path_str; }

		path get_parent_path() const;
		std::string get_file_name() const;
		std::string get_extension() const;
		path normalize() const;
		path relative_to(const path& base) const;

		bool is_relative() const;
		bool is_absolute() const;

	private:
		void append_path(const std::string& path);

		std::string path_str;
	};

	namespace filesystem
	{
		bool is_directory(const std::string& path);
		bool exists(const std::string& path);
		size_t get_size(const std::string& path);
		bool create_directory(const std::string& path);

		int open(const std::string& path, int flags, int mode = 0777);
		int close(int fd);
		size_t read(int fd, void* buffer, size_t size);
		size_t write(int fd, const void* buffer, size_t size);
		int seek(int fd, long offset, int whence);
		long tell(int fd);
		int flush(int fd);

		bool copy(const std::string& src, const std::string& dst);
		bool move(const std::string& src, const std::string& dst, bool copy_ = false);
		bool remove(const std::string& path);
		bool rename(const std::string& src, const std::string& dst);

		void list(const std::string& path, std::vector<std::string>& files, bool recurse = false);

		bool read_file(const std::string& path, const void* buffer, size_t size);
		bool read_file(const std::string& path, std::vector<uint8_t>& buffer);
		bool write_file(const std::string& path, const void* buffer, size_t size);
		bool append_file(const std::string& path, const void* buffer, size_t size);
		bool copy_file(const std::string& src, const std::string& dst);
		bool move_file(const std::string& src, const std::string& dst);
		bool remove_file(const std::string& path);
		bool rename_file(const std::string& src, const std::string& dst);
	}

	template<typename T>
	class symbol
	{
	public:
		symbol(uintptr_t address, uintptr_t base = process_base_address) : address(address), base(base) {}

		T* get()
		{
			return reinterpret_cast<T*>(base + address);
		}

		constexpr operator T* () const
		{
			return this->get();
		}

		constexpr T* operator->() const
		{
			return get();
		}

		constexpr symbol<T>& operator=(uintptr_t address)
		{
			this->address = address;
			return *this;
		}

		constexpr operator uintptr_t() const
		{
			return reinterpret_cast<uintptr_t>(this->get());
		}

		uintptr_t get_address() const { return address; }
		uintptr_t get_base() const { return base; }
	private:
		uintptr_t address;
		uintptr_t base;
	};


	namespace memory
	{
		class Chunk
		{
		public:
			Chunk(size_t size, void* ptr = nullptr, Chunk* prev = nullptr, Chunk* next = nullptr)
				: m_size(size), m_ptr(ptr), m_prev(prev), m_next(next) {
			}
			bool operator<(const Chunk& rhs) const
			{
				return (m_size < rhs.m_size) || (m_size == rhs.m_size
					&& (!rhs.m_ptr || reinterpret_cast<size_t>(m_ptr) < reinterpret_cast<size_t>(rhs.m_ptr)));
			}
			bool operator==(const Chunk& rhs) const
			{
				return m_ptr == rhs.m_ptr && m_size == rhs.m_size;
			}

			size_t m_size;
			void* m_ptr;
			Chunk* m_prev;
			Chunk* m_next;
		};
		class allocator
		{
		public:
			~allocator();

			void clear();
			void* allocate(const size_t length);
			void* reallocate(void* data, const size_t length);
			template<typename T>
			T* allocate(const size_t count)
			{
				return static_cast<T*>(allocate(sizeof(T) * count));
			}
			template<typename T>
			T* allocate()
			{
				return allocate<T>(sizeof(T));
			}
			template<typename T>
			void free(T* data)
			{
				free(static_cast<void*>(data));
			}
			void free(void* data);
			void free(const void* data);
			bool empty();
		private:
			std::mutex mtx;
			std::vector<void*> pointers;
		};
		class heap_allocator
		{
		public:
			heap_allocator() = default;
			heap_allocator(void* start, size_t size, const std::string& name);
			~heap_allocator();

			void set(void* start, size_t size, const std::string& name);

			void* allocate(size_t size, size_t alignment = 16, const std::string& name = "");
			void free(void* ptr);

			void clear();

			struct chunk_compare
			{
				bool operator()(const Chunk* lhs, const Chunk* rhs) const;
			};

			void* MapStart;
			size_t Size;
			size_t CurrentSize;
			std::string Name;
			std::mutex mtx;
			std::set<Chunk*, chunk_compare> free_chunks;
			std::unordered_map<void*, Chunk*> chunks;
		};
		class mapped_allocator : public heap_allocator
		{
		public:
			mapped_allocator() = default;
			mapped_allocator(size_t size, int protection = SCE_KERNEL_PROT_CPU_ALL, int flags = 0x1000 | 0x2, uintptr_t address = process_base_address);
			mapped_allocator(const mapped_allocator&) = delete;
			virtual ~mapped_allocator();

			void initialize(size_t size, int protection = SCE_KERNEL_PROT_CPU_ALL, int flags = 0x1000 | 0x2, uintptr_t address = process_base_address);

			template<typename T>
			T* allocate(const size_t count)
			{
				return static_cast<T*>(allocate(sizeof(T) * count, 0));
			}

			void* allocate(size_t size, const std::string& name = "");

			uintptr_t Base;
			size_t Size;
			bool Initialised;
			size_t CurrentSize;
		};
		class direct_memory_allocator : public heap_allocator
		{
		public:
			direct_memory_allocator() = default;
			direct_memory_allocator(size_t size, SceKernelMemoryType type, int protection = SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW, std::string name = "");
			direct_memory_allocator(const direct_memory_allocator&) = delete;
			virtual ~direct_memory_allocator();

			void initialize(size_t size, SceKernelMemoryType type, std::string name = "", int protection = SCE_KERNEL_PROT_CPU_RW | SCE_KERNEL_PROT_GPU_RW);

			template<typename T>
			T* allocate(const size_t count, size_t alignment = 16)
			{
				return static_cast<T*>(allocate(sizeof(T) * count, alignment));
			}

			void* allocate(size_t size, size_t alignment = 16, const std::string& name = "");
			void* allocate(const sce::Gnm::SizeAlign& sizeAlign, const std::string& name = "");

			off_t Start;
			size_t End;
			bool Initialised;
			SceKernelMemoryType Type;
		};

		enum type
		{
			onion,
			garlic_wc,
			garlic_wb,
			unknown
		};

		struct region
		{
			uintptr_t start;
			uintptr_t end;
			size_t size;
			type type;
			int prot;
		};

		bool query(uintptr_t address, region& out);
		bool protect(uintptr_t address, size_t size, int prot, int* prev);

		void write(uintptr_t address, const void* buffer, size_t size);
		void read(uintptr_t address, void* buffer, size_t size);

		// Template for reading data
		template <typename T>
		T read(uintptr_t address)
		{
			T value;
			read(address, reinterpret_cast<void*>(&value), sizeof(T));
			return value;
		}

		template<class T = uint8_t>
		std::vector<T> read(uintptr_t address, size_t size)
		{
			std::vector<T> buffer(size);
			read(address, buffer.data(), size);
			return buffer;
		}

		// Template for writing data
		template <typename T>
		void write(uintptr_t address, const T& value, size_t count = 0)
		{
			write(address, reinterpret_cast<const void*>(&value), sizeof(T) * count);
		}

		template<class T = uint8_t>
		void write(uintptr_t address, const std::vector<T>& buffer)
		{
			write(address, buffer.data(), buffer.size());
		}
	}

	namespace resolve
	{
		uintptr_t get_process_base();
		uintptr_t get_export_address(const std::string& module, const std::string& function);

		size_t get_module_size(const std::string& module);
		SceKernelModuleInfo get_module_info(const std::string& module = "");

		uint64_t read_instruction(uintptr_t address, int opcode_size, int opcode_length);

		bool load_module(int module);
		bool unload_module(int module);

		bool load_module(const std::string& module);
		bool unload_module(const std::string& module);

		bool is_loaded(int module);
		template<typename T = uintptr_t>

		T get_absolute_address(uintptr_t address, uintptr_t base = -1)
		{
			if (base == -1)
				base = get_process_base();

			return reinterpret_cast<T>(base + address);
		}

		template<typename T = uintptr_t>
		T get_module_address(const std::string& module)
		{
			auto library = sceKernelLoadStartModule(module.c_str(), 0, nullptr, 0, nullptr, nullptr);
			if (library < 0)
			{
				printf("sceKernelLoadStartModule(%s) failed: 0x%08X\n", module.c_str(), library);
				return 0;
			}

			SceKernelModuleInfo moduleInfo;
			moduleInfo.size = sizeof(SceKernelModuleInfo);
			if (sceKernelGetModuleInfo(library, &moduleInfo) < 0)
			{
				printf("sceKernelGetModuleInfo(%s) failed\n", module.c_str());
				return 0;
			}

			return reinterpret_cast<T>(moduleInfo.segmentInfo->address);
		}

		template<typename T = uintptr_t>
		T get_module_address(const std::string& module, const std::string& function)
		{
			return (T)get_export_address(module, function);
		}
	}

	namespace math
	{
		class vector2
		{
		public:
			float x, y;

			vector2();
			vector2(float x, float y);
			vector2(float vec[2]);

			vector2 operator+(const vector2 vec);
			vector2 operator+(const float* vec);
			vector2 operator-(const vector2 vec);
			vector2 operator-(const float* vec);
			vector2 operator-() const;
			vector2 operator*(const vector2 vec);
			vector2 operator*(const float scalar);

			void operator-=(const vector2 vec);
			void operator+=(const vector2 vec);
			void operator*=(const vector2 vec);
			void operator*=(const float scalar);

			bool operator==(const vector2 vec);
			bool operator==(const float value);
			bool operator!=(const vector2 vec);
			bool operator!=(const float value);

			operator float* ();

			float size();
			void normalize(float tolerance);
		};

		class vector3
		{
		public:
			float x, y, z;

			vector3();
			vector3(float x, float y, float z);
			vector3(float vec[3]);

			void set(float x, float y, float z);

			vector3 operator+(const vector3& vec) const;
			vector3 operator+(const float* vec) const;
			vector3 operator-(const vector3& vec) const;
			vector3 operator-(const float* vec) const;
			vector3 operator-() const;
			vector3 operator*(const vector3& vec) const;
			vector3 operator*(const float scalar) const;
			vector3 operator/(const float f) const;
			vector3 operator/(const vector3& vec) const;
			vector3 operator|(const vector3& vec) const;
			vector3 operator&(const vector3& vec) const;
			vector3 operator^(const vector3& vec) const;

			void operator|=(const vector3& vec) const;
			void operator&=(const vector3& vec) const;
			void operator^=(const vector3& vec) const;

			bool operator==(const vector3& vec) const;
			bool operator==(const float value) const;
			bool operator!=(const vector3& vec) const;
			bool operator!=(const float value) const;

			void operator-=(const vector3& vec);
			void operator+=(const vector3& vec);
			void operator*=(const vector3& vec);
			void operator*=(const float scalar);
			void operator/=(const float scalar);
			void operator/=(const vector3& vec);

			friend vector3 operator*(const float f, const vector3& vec);

			float operator[](const size_t index);

			operator float* ();

			vector3 abs() const;
			vector3 cross(const vector3& a) const;
			float mag() const;
			float square_mag() const;
			float clamp_angle(float r) const;

			bool is_zero();

			void inv_scale(const vector3& vec);
			void extend(float distance);
			void extend(const vector3& v, float distance);
			void scale(float f);
			void scale(const vector3& a, float f);
			void vectors(vector3* forward, vector3* right, vector3* up);
			void lerp(float t, const vector3& a, const vector3& b);
			void lerp(float t, const vector3& a);

			vector3 forward(float x = 1.0f);
			vector3 normal();
			vector3 normalize();

			float distance(const vector3& vec);
			float length();
			float dot(const vector3& vector);
		};

		class vector4
		{
		public:
			float x, y, z, w;

			vector4();
			vector4(float x, float y, float z, float w);
			vector4(float vec[4]);

			vector4 operator+(const vector4 vec);
			vector4 operator+(const float* vec);
			vector4 operator-(const vector4 vec);
			vector4 operator-(const float* vec);
			vector4 operator-() const;
			vector4 operator*(const vector4 vec);
			vector4 operator*(const float scalar);

			void operator-=(const vector4 vec);
			void operator+=(const vector4 vec);
			void operator*=(const vector4 vec);
			void operator*=(const float scalar);

			bool operator==(const vector4 vec);
			bool operator==(const float value);
			bool operator!=(const vector4 vec);
			bool operator!=(const float value);

			operator float* ();
		};

		struct matrix3x3
		{
			union
			{
				struct
				{
					vector3 aside;
					vector3 up;
					vector3 dir;
				};

				float	m[3][3];
				struct
				{
					float   _11, _12, _13;
					float	_21, _22, _23;
					float	_31, _32, _33;
				};
			};

			matrix3x3()
			{
				for (int i = 0; i < 3; ++i)
					for (int j = 0; j < 3; ++j)
						m[i][j] = 0.0f;
			}

			static matrix3x3 lookat(const vector3& eye, const vector3& target, const vector3& up) {
				vector3 forward = (target - eye).normalize();
				vector3 right = up.cross(forward).normalize();
				vector3 newUp = forward.cross(right);

				matrix3x3 result;

				result.m[0][0] = right.x;
				result.m[1][0] = right.y;
				result.m[2][0] = right.z;

				result.m[0][1] = newUp.x;
				result.m[1][1] = newUp.y;
				result.m[2][1] = newUp.z;

				result.m[0][2] = -forward.x;
				result.m[1][2] = -forward.y;
				result.m[2][2] = -forward.z;

				return result;
			}

			matrix3x3 inverse() const;
		};
	}

	class thread
	{
	public:
		thread() = default;
		thread(void(*func)(), std::string name);
		void join();
		void detach();
		bool joinable() const;
		static void sleep(unsigned int ms);
	private:
		ScePthread handle;
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

		std::vector<posix_thread*> freeze(bool freeze_user_ctx = true);
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


	namespace http
	{
		struct request
		{
			std::string url;
			std::string path;
			std::string data;
			SceHttpMethods method;
			std::unordered_map<std::string, std::string> headers;

			request() : url(""), path(""), data("") {}

			// get
			request(const std::string& uri, const std::string& path, SceHttpMethods method, const std::unordered_map<std::string, std::string>& headers = {})
				: url(uri), path(path), method(method), headers(headers) {
			}

			// post
			request(const std::string& uri, const std::string& path, const std::string& data, SceHttpMethods method, const std::unordered_map<std::string, std::string>& headers = {})
				: url(uri), path(path), data(data), method(method), headers(headers) {
			}
		};

		struct response
		{
			bool success;
			int req_id;
			int status;
			SceHttpMethods method;
			std::string data;

			std::unordered_map<std::string, std::string> response_headers;

			response() : success(false), req_id(-1), status(-1), method(SCE_HTTP_METHOD_GET) {}
			response(bool success, int req_id, int status, SceHttpMethods method, const std::string& data, const std::unordered_map<std::string, std::string>& response_headers = {})
				: success(success), req_id(req_id), status(status), method(method), data(data), response_headers(response_headers) {
			}
		};

		struct connection
		{
			int id;
			std::string url;
			std::string user_agent;

			std::vector<request> requests;
			std::vector<response> responses;

			connection() : id(-1) {}
		};

		namespace internal
		{
			inline int is_running = false;
			inline bool using_override = false;
			inline int template_id = LIBORBISUTIL_HTTP_CTX_DEFAULT;
			inline int memory_id = LIBORBISUTIL_HTTP_CTX_DEFAULT;
			inline int http_ctx = LIBORBISUTIL_HTTP_CTX_DEFAULT;
			inline int ssl_ctx = LIBORBISUTIL_HTTP_CTX_DEFAULT;

			inline std::unordered_map<std::string, connection> connections;

			bool load_modules();

			response get(connection& conn, request req);
			response post(connection& conn, request req);
			response put(connection& conn, request req);
			response del(connection& conn, request req);
			response download(connection& conn, request req, const std::string& path);
		}

		response get(const std::string& uri, const std::unordered_map<std::string, std::string>& headers = {});
		response post(const std::string& uri, const std::string& data, const std::unordered_map<std::string, std::string>& headers = {});
		response put(const std::string& uri, const std::string& data, const std::unordered_map<std::string, std::string>& headers = {});
		response del(const std::string& uri, const std::unordered_map<std::string, std::string>& headers = {});
		response download(const std::string& uri, const std::string& path, const std::unordered_map<std::string, std::string>& headers = {});

		connection find_connection(const std::string& url);
		connection create_connection(const std::string& url, const std::string& user_agent = "liborbisutil");
		void destroy_connection(connection conn);
		void destroy_connections();

		bool initialize(int http_context_override = LIBORBISUTIL_HTTP_CTX_DEFAULT);
		void finalize();
	}

	enum socket_type
	{
		tcp,
		udp
	};

	class socket
	{
	public:
		socket() = default;
		socket(const std::string& ip, uint16_t port, socket_type type = tcp);
		socket(uint32_t ip, uint16_t port, socket_type type = tcp);

		bool connect();
		void close();
		bool receive(void* buffer, size_t length);
		bool send(const void* buffer, size_t length);

		bool is_connected() const { return connected; };
	private:
		int fd;
		bool connected;
		uint32_t ip;
		uint16_t port;
		socket_type type;
		SceNetSockaddrIn server_addr;
	};

	namespace exception
	{
		struct exception_info
		{
			std::string message;
			std::string module;
			std::string function;
			uintptr_t address;
			SceDbgUcontext context;
		};

		struct exception_action
		{
			int type;
			std::function<void(int, siginfo_t*, SceDbgUcontext*)> callback;
			bool log;
		};

		namespace handler
		{
			inline jmp_buf recovery_point;
			inline std::unordered_map<int, exception_action> actions;

			void register_action(int type, std::function<void(int, siginfo_t*, SceDbgUcontext*)> callback, bool log);
			void unregister_action(int type);
		}
	}

	namespace hook
	{
		class detour
		{
		public:
			detour() = default;
			detour(void* target, void* replace, bool enabled = false);
			detour(uintptr_t target, void* replace, bool enabled = false);
			~detour();

			void enable() const;
			void disable() const;

			void create(void* target, void* replace, bool enabled = false);
			void create(uintptr_t target, void* replace, bool enabled = false);
			void clear();

			template <typename T>
			T* get() const
			{
				return static_cast<T*>(this->get_original());
			}

			template<typename R = void, typename... Args>
			R invoke(Args... args)
			{
				return reinterpret_cast<R(*)(Args...)>(this->original)(args...);
			}

			void* get_original() const;
			bool is_created() const { return this->created; }
		private:
			void* target;
			void* replace;
			void* original;

			bool created;
		};

		class manager
		{
		public:
			manager() = default;
			~manager();

			bool create(const std::string& name, void* target, void* replace);
			bool create(const std::string& name, const std::string& module, const std::string& function, void* replace);

			bool enable(const std::string& name);
			bool disable(const std::string& name);

			bool remove(const std::string& name);
			void clear();

			detour* get(const std::string& name);

			template<typename R>
			bool create(const std::string& name, void* target, R replace)
			{
				return create(name, target, reinterpret_cast<void*>(replace));
			}

			template<typename R>
			bool create(const std::string& name, uintptr_t target, R replace)
			{
				return create(name, reinterpret_cast<void*>(target), reinterpret_cast<void*>(replace));
			}

			template<typename R>
			bool create(const std::string& name, const std::string& module, const std::string& function, R replace)
			{
				return create(name, module, function, reinterpret_cast<void*>(replace));
			}

			template<typename R = void, typename... Args>
			R invoke(const std::string& name, Args... args)
			{
				return reinterpret_cast<R(*)(Args...)>(this->get(name)->get_original())(args...);
			}
		private:
			std::unordered_map<std::string, std::unique_ptr<detour>> hooks;
		};

		template <typename T>
		static void set(void* place, T value)
		{
			int prot;
			memory::protect(reinterpret_cast<uintptr_t>(place), sizeof(T), SCE_KERNEL_PROT_CPU_ALL, &prot);

			*static_cast<T*>(place) = value;

			memory::protect(reinterpret_cast<uintptr_t>(place), sizeof(T), prot, &prot);
		}
	}

	namespace patcher
	{
		struct info
		{
			info(uintptr_t address)
				: address(address), set(false)
			{
			}

			bool is_set();

			void enable();
			void disable();
			void toggle();
			void restore();

			uintptr_t address;
			std::vector<uint8_t> original;
			std::vector<uint8_t> patched;
			bool set;
		};

		void enable(const std::string& name);
		void disable(const std::string& name);
		void toggle(const std::string& name);
		void restore(const std::string& name);

		info* get(const std::string& name);

		info* nop(const std::string& name, const std::string& module, uint64_t offset, size_t size);
		info* nop(const std::string& name, uintptr_t address, size_t size);

		info* create(const std::string& name, const std::string& module, uint64_t offset, const std::vector<uint8_t>& patched, const bool set = false);
		info* create(const std::string& name, uintptr_t address, const std::vector<uint8_t>& patched, const bool set = false);

		void remove(const std::string& name);
	}

	namespace pad
	{
		namespace state
		{
			enum hook
			{
				read,
				read_state,
				max
			};
		}

		namespace buttons
		{
			enum input_id
			{
				cross,
				square,
				circle,
				triangle,
				dpad_left,
				dpad_right,
				dpad_up,
				dpad_down,
				r1,
				r2,
				l1,
				l2,
				l3,
				r3,
				start,
				select,
				touchpad,
				max
			};
		}

		struct button_state
		{
			bool pressed;
			bool released;
			float input;
			float duration;
		};

		inline hook::manager detour_manager;

		inline hook::detour scePadRead_d;
		inline hook::detour scePadReadState_d;

		inline button_state current_buttons[buttons::max];
		inline button_state previous_buttons[buttons::max];

		inline std::function<void(ScePadData*, int)> callback[state::max];
		inline bool should_monitor[state::max];

		inline ScePadData current_pad_data;
		inline ScePadData previous_pad_data;

		inline uint32_t current_button_mask;
		inline uint32_t previous_button_mask;

		inline uint32_t pressed_button_mask;
		inline uint32_t released_button_mask;

		inline math::vector2 left_stick;
		inline math::vector2 right_stick;

		inline float l2_pressure;
		inline float r2_pressure;

		bool is_down(uint32_t button, float repeat_rate = 0.0f);
		bool is_pressed(uint32_t button);
		bool is_released(uint32_t button);

		inline float get_left_stick_x() { return left_stick.x; }
		inline float get_left_stick_y() { return left_stick.y; }

		inline float get_right_stick_x() { return right_stick.x; }
		inline float get_right_stick_y() { return right_stick.y; }

		void monitor(ScePadData* data);
		void register_callback(state::hook id, std::function<void(ScePadData*, int)> cb);
		void initialize(state::hook id, bool monitor, std::function<void(ScePadData*, int)> cb);
		void finalize();
	}
}