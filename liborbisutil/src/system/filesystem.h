#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace liborbisutil
{
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
}