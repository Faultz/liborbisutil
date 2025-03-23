#include "filesystem.h"
#include "string.h"

#include <kernel.h>
#include <sstream>

#include "logger.h"

namespace liborbisutil
{
	int convert_to_mode_str(const std::string& str, int& mode)
	{
#ifdef _DEBUG
		LOG_DEBUG("ConvertModeString called with str: %s\n", str.c_str());
#endif
		auto modeStr = str.c_str();

		int res, m, o;
		switch (*modeStr++)
		{
		case 'r':
			res = 2;
			m = SCE_KERNEL_O_RDONLY;
			o = 0;
			break;
		case 'w':
			res = 1;
			m = SCE_KERNEL_O_WRONLY;
			o = SCE_KERNEL_O_CREAT | SCE_KERNEL_O_TRUNC;
			break;
		case 'a':
			res = 1;
			m = SCE_KERNEL_O_WRONLY;
			o = SCE_KERNEL_O_CREAT | SCE_KERNEL_O_APPEND;
			break;
		default:
			return false;
		}

		if (*modeStr == '+' || (*modeStr == 'b' && modeStr[1] == '+'))
		{
			res = 1;
			m = SCE_KERNEL_O_RDWR;
		}

		mode = m | o;
#ifdef _DEBUG
		LOG_DEBUG("Converted mode: %d\n", mode);
#endif
		return res;
	}

	size_t get_file_size(int fd)
	{
#ifdef _DEBUG
		LOG_DEBUG("GetFileSize called for fd: %d\n", fd);
#endif
		SceKernelStat buffer;
		if (sceKernelFstat(fd, &buffer) < 0)
			return 0;
		return buffer.st_size;
	}

	file_system::file_system(std::string file, std::string mode)
	{
		open(file, mode);
	}

	file_system::file_system(std::string file, int mode)
	{
		open(file, mode);
	}

	file_system::~file_system()
	{
		if (is_open())
			close();
	}

	file_system& file_system::operator<<(const char* buf)
	{
		print(buf);
		return *this;
	}

	file_system& file_system::operator<<(const std::string& buf)
	{
		print(buf.c_str());
		return *this;
	}

	file_system& file_system::operator<<(const int& buf)
	{
		write(&buf, sizeof(int));
		return *this;
	}

	file_system& file_system::operator<<(const float& buf)
	{
		write(&buf, sizeof(float));
		return *this;
	}

	file_system& file_system::operator<<(const double& buf)
	{
		write(&buf, sizeof(double));
		return *this;
	}

	file_system::operator bool()
	{
		return (tell() < length);
	}

	bool file_system::is_open()
	{
		return fd >= 0;
	}

	bool file_system::is_good()
	{
		if (tell() > length)
			return false;
		return true;
	}

	size_t file_system::get_size()
	{
		return length;
	}

	std::string file_system::read_to_end()
	{
		std::string buffer;
		buffer.resize(length);
		read((void*)buffer.data(), length);
		return buffer;
	}

	int file_system::open(const std::string& path, std::string mode)
	{
		name = path;

		int flags, oflags;
		if ((flags = convert_to_mode_str(mode, oflags)) == 0)
			return 0;

		fd = sceKernelOpen(path.data(), oflags, 0777);
		if (fd < 0)
		{
			LOG_ERROR("Failed to open file: %s\n", path.data());
			return 0;
		}

		offset = 0;
		end_of_file = false;
		if (oflags & SCE_KERNEL_O_APPEND)
		{
			offset = get_file_size(fd);
			end_of_file = true;
			sceKernelLseek(fd, offset, SCE_KERNEL_SEEK_END);
		}

		length = get_file_size(fd);

		return fd;
	}

	int file_system::open(const std::string& path, int mode)
	{
		fd = sceKernelOpen(path.data(), mode, 0777);
		if (fd < 0)
		{
			LOG_ERROR("Failed to open file: %s\n", path.data());
			return 0;
		}
		offset = 0;
		end_of_file = false;
		if (mode & SCE_KERNEL_O_APPEND)
		{
			offset = get_file_size(fd);
			end_of_file = true;
			sceKernelLseek(fd, offset, SCE_KERNEL_SEEK_END);
		}
		length = get_file_size(fd);
		return fd;
	}

	size_t file_system::read(const void* buf, size_t length, size_t count)
	{
		size_t amount = sceKernelRead(fd, (void*)buf, length * count);
		if (amount < 0)
		{
			LOG_ERROR("Failed to read file\n");
			return 0;
		}
		offset = tell();
		return amount;
	}

	size_t file_system::readOffset(void* data, size_t length, size_t offset)
	{
		if (fd < 0)
		{
			LOG_ERROR("Invalid file descriptor\n");
			return 0;
		}
		auto res = sceKernelLseek(fd, offset, SCE_KERNEL_SEEK_SET);
		if (res < 0)
		{
			LOG_ERROR("Failed to seek file\n");
			return 0;
		}

		res = read(data, length);
		if (res < 0)
		{
			LOG_ERROR("Failed to read file\n");
			return 0;
		}

		return res;
	}

	size_t file_system::print(const char* format, ...)
	{
		va_list args;
		va_start(args, format);

		auto str = string::format(format, args);

		va_end(args);

		return write(str.data(), str.size());
	}

	size_t file_system::write(const void* buf, size_t length, size_t count)
	{
		size_t amount = sceKernelWrite(fd, buf, length * count);
		if (amount < 0)
		{
			LOG_ERROR("Failed to write file\n");
			return 0;
		}
		return amount;
	}

	size_t file_system::seek(size_t offset, int whence)
	{
		auto res = sceKernelLseek(fd, offset, whence);
		if (res < 0)
		{
			LOG_ERROR("Failed to seek file\n");
			return 0;
		}

		offset = res;

		return res;
	}

	size_t file_system::tell()
	{
		auto res = sceKernelLseek(fd, 0, SCE_KERNEL_SEEK_CUR);
		if (res < 0)
		{
			LOG_ERROR("Failed to tell file\n");
			return 0;
		}
		return res;
	}

	bool file_system::getline(std::string& line, char delimeter)
	{
		if (fd < 0)
		{
			LOG_ERROR("Invalid file descriptor\n");
			return false;
		}
		line.clear();
		char c;
		while (true)
		{
			auto res = sceKernelRead(fd, &c, 1);
			if (res < 0)
			{
				LOG_ERROR("Failed to read file\n");
				return false;
			}
			if (res == 0)
			{
				end_of_file = true;
				return false;
			}
			if (c == delimeter)
				return true;
			line += c;
		}

		return false;
	}

	void file_system::close()
	{
		sceKernelClose(fd);
	}

	directory_iterator::Iterator& directory_iterator::Iterator::operator++()
	{
		current_index++;
		return *this;
	}

	const std::string& directory_iterator::Iterator::operator*() const
	{
		if (current_index >= parent_dir.files.size())
			return parent_dir.files.back();

		return parent_dir.files[current_index];
	}

	bool directory_iterator::Iterator::operator!=(const Iterator& other) const
	{
		return current_index != other.current_index;
	}

	directory_iterator::directory_iterator(const std::string& directory, bool recursive)
		: directory_path(directory), recurse(recursive)
	{
		scan_directory();
	}

	directory_iterator::~directory_iterator()
	{
		directory_path.clear();
		files.clear();
	}

	directory_iterator::Iterator directory_iterator::begin()
	{
		return Iterator(*this);
	}

	directory_iterator::Iterator directory_iterator::end()
	{
		return Iterator(*this, true);
	}

	void directory_iterator::scan_directory()
	{
		char buffer[8192];
		SceKernelDirent* direntry;

		std::string fullDirectoryName = directory_path;
		if (!string::ends_with(fullDirectoryName, "/") || !string::ends_with(fullDirectoryName, "\\"))
			fullDirectoryName.append("/");

		auto fd = filesystem::open(fullDirectoryName, SCE_KERNEL_O_RDONLY, 0);
		if (fd < 0)
		{
			printf("Failed to open directory: %s\n", fullDirectoryName.data());
			return;
		}

		while (true)
		{
			memset(buffer, 0, sizeof(buffer));
			auto res = sceKernelGetdents(fd, buffer, sizeof(buffer));
			if (res < 0)
			{
				printf("Failed to get directory entries: %s\n", fullDirectoryName.data());
				filesystem::close(fd);
				return;
			}

			if (res == 0)
				break;

			direntry = reinterpret_cast<SceKernelDirent*>(buffer);
			while (direntry->d_fileno != 0)
			{
				std::string name = direntry->d_name;
				if (name != "." && name != "..")
				{
					SceKernelStat stat;
					auto file = fullDirectoryName + name;

					if (sceKernelStat(file.data(), &stat) < 0)
					{
						printf("Failed to stat file: %s\n", file.data());
						direntry = (SceKernelDirent*)((char*)direntry + direntry->d_reclen);
						continue;
					}

					if ((stat.st_mode & SCE_KERNEL_S_IFDIR) != 0)
					{
						if (recurse)
						{
							files.push_back(file);

							directory_iterator sub(file, true);
							for (auto& subfile : sub)
							{
								files.push_back(subfile);
							}
						}
					}
					else
					{
						files.push_back(file);
					}
				}
				direntry = (SceKernelDirent*)((char*)direntry + direntry->d_reclen);
			}
		}
		filesystem::close(fd);
	}

	path path::operator/(const std::string& new_path)
	{
		path p = *this;
		p.append_path(new_path);
		return p;
	}

	path& path::operator/=(const std::string& new_path)
	{
		append_path(new_path);
		return *this;
	}

	path path::operator/(const path& new_path)
	{
		path p = *this;
		p.append_path(new_path.path_str);
		return p;
	}

	path& path::operator/=(const path& new_path)
	{
		append_path(new_path.path_str);
		return *this;
	}

	path path::get_parent_path() const
	{
		auto pos = path_str.find_last_of("/\\");
		if (pos == std::string::npos)
			return path("");

		return path(path_str.substr(0, pos));
	}

	std::string path::get_file_name() const
	{
		auto pos = path_str.find_last_of("/\\");
		if (pos == std::string::npos)
			return path_str;
		return path_str.substr(pos + 1);
	}

	std::string path::get_extension() const
	{
		auto pos = path_str.find_last_of(".");
		if (pos == std::string::npos)
			return "";

		return path_str.substr(pos + 1);
	}

	path path::normalize() const
	{
		std::vector<std::string> parts;

		std::istringstream stream(path_str);
		std::string part;

		while (std::getline(stream, part, '/'))
		{
			if (part == "." || part.empty())
				continue;

			if (part == "..")
			{
				if (!parts.empty())
					parts.pop_back();
			}
			else if (part != ".")
			{
				parts.push_back(part);
			}
		}

		std::string result;
		for (const auto& p : parts)
		{
			if (!result.empty())
				result += "/";

			result += p;
		}

		return path(result);
	}

	path path::relative_to(const path& base) const
	{
		path normalizedBase = base.normalize();
		path normalizedPath = this->normalize();

		if (normalizedPath.str().substr(0, normalizedBase.str().size()) != normalizedBase.str())
			return normalizedPath;

		auto target = normalizedPath.str().substr(normalizedBase.str().size());
		if (target[0] == '/')
			target = target.substr(1);

		return path(target);
	}

	bool path::is_relative() const
	{
		return path_str.empty() || path_str[0] != '/';
	}

	bool path::is_absolute() const
	{
		return !is_relative();
	}

	void path::append_path(const std::string& path)
	{
		if (!path_str.empty() && path_str.back() != '/' && path_str.back() != '\\') {
			path_str.push_back('/');
		}
		path_str += path;
	}

	namespace filesystem
	{
		bool is_directory(const std::string& path)
		{
			SceKernelStat stat;
			auto res = sceKernelStat(path.c_str(), &stat);
			if (res < 0)
			{
				return false;
			}
			return stat.st_mode & SCE_KERNEL_S_IFDIR;
		}

		bool exists(const std::string& path)
		{
			SceKernelStat stat;
			auto res = sceKernelStat(path.c_str(), &stat);
			return res >= 0;
		}

		size_t get_size(const std::string& path)
		{
			SceKernelStat stat;
			auto res = sceKernelStat(path.c_str(), &stat);
			if (res < 0)
				return 0;

			return stat.st_size;
		}

		bool create_directory(const std::string& path, bool full_path)
		{
			for (int i = 1; i < path.length(); i++)
			{
				if (path[i] == '/' || path[i] == '\\')
				{
					auto subpath = path.substr(0, i + 1);
					if (!exists(subpath))
					{
						auto res = sceKernelMkdir(subpath.c_str(), 0777);
						if (res < 0)
						{
							printf("sceKernelMkdir(%s) failed: 0x%08X\n", subpath.c_str(), res);
							return false;
						}
					}
				}
			}

			if(full_path)
				sceKernelMkdir(path.c_str(), 0777);

			return true;
		}

		int open(const std::string& path, int flags, int mode)
		{
			auto fd = sceKernelOpen(path.c_str(), flags, mode);
			if (fd < 0)
			{
				printf("sceKernelOpen(%s) failed: 0x%08X\n", path.c_str(), fd);
			}

			return fd;
		}
		int close(int fd)
		{
			auto res = sceKernelClose(fd);
			if (res < 0)
			{
				printf("sceKernelClose(%d) failed: 0x%08X\n", fd, res);
			}

			return res;
		}
		size_t read(int fd, void* buffer, size_t size)
		{
			auto res = sceKernelRead(fd, buffer, size);
			if (res < 0)
			{
				printf("sceKernelRead(%d) failed: 0x%08X\n", fd, res);
			}

			return res;
		}
		size_t write(int fd, const void* buffer, size_t size)
		{
			auto res = sceKernelWrite(fd, buffer, size);
			if (res < 0)
			{
				printf("sceKernelWrite(%d) failed: 0x%08X\n", fd, res);
			}
			return res;
		}
		int seek(int fd, long offset, int whence)
		{
			auto res = sceKernelLseek(fd, offset, whence);
			if (res < 0)
			{
				printf("sceKernelLseek(%d) failed: 0x%08X\n", fd, res);
			}
			return res;
		}
		long tell(int fd)
		{
			auto res = sceKernelLseek(fd, 0, SEEK_CUR);
			if (res < 0)
			{
				printf("sceKernelLseek(%d) failed: 0x%08X\n", fd, res);
			}
			return res;
		}
		int flush(int fd)
		{
			auto res = sceKernelFsync(fd);
			if (res < 0)
			{
				printf("sceKernelFsync(%d) failed: 0x%08X\n", fd, res);
			}
			return res;
		}

		bool copy(const std::string& src, const std::string& dst)
		{
			if (is_directory(src))
			{
				if (!exists(src))
				{
					printf("Source directory does not exist: %s\n", src.c_str());
					return false;
				}

				if (!create_directory(dst))
				{
					printf("Failed to create directory: %s\n", dst.c_str());
					return false;
				}
				directory_iterator it(src, true);
				for (auto& file : it)
				{
					auto entryPath = file.substr(src.size() + 1);

					std::string oldEntry = src + "/" + entryPath;
					std::string newEntry = dst + "/" + entryPath;
					if (!copy(file, newEntry))
					{
						printf("Failed to copy file: %s\n", file.c_str());
						return false;
					}
				}
				return true;
			}

			return copy_file(src, dst);
		}

		bool move(const std::string& src, const std::string& dst, bool copy_)
		{
			if (copy_)
			{
				if(!copy(src, dst))
				{
					printf("Failed to copy file: %s\n", src.c_str());
					return false;
				}

				return true;
			}

			if (is_directory(src))
			{
				if (!exists(src))
				{
					printf("Source directory does not exist: %s\n", src.c_str());
					return false;
				}
				if (!create_directory(dst))
				{
					printf("Failed to create directory: %s\n", dst.c_str());
					return false;
				}
				directory_iterator it(src, true);
				for (auto& file : it)
				{
					auto entryPath = file.substr(src.size() + 1);

					std::string oldEntry = src + "/" + entryPath;
					std::string newEntry = dst + "/" + entryPath;
					if (!move(file, newEntry, copy_))
					{
						printf("Failed to move file: %s\n", file.c_str());
						return false;
					}
				}
				if (!remove(src))
				{
					printf("Failed to remove directory: %s\n", src.c_str());
					return false;
				}

				return true;
			}

			return move_file(src, dst);
		}

		bool remove(const std::string& path)
		{
			if (is_directory(path))
			{
				directory_iterator it(path, true);
				for (auto& file : it)
				{
					if (!remove(file))
					{
						printf("Failed to remove file: %s\n", file.c_str());
						return false;
					}
				}

				return true;
			}

			return remove_file(path);
		}

		bool rename(const std::string& src, const std::string& dst)
		{
			if (is_directory(src))
			{
				if (!exists(src))
				{
					printf("Source directory does not exist: %s\n", src.c_str());
					return false;
				}
				if (!create_directory(dst))
				{
					printf("Failed to create directory: %s\n", dst.c_str());
					return false;
				}
				directory_iterator it(src, true);
				for (auto& file : it)
				{
					auto entryPath = file.substr(src.size() + 1);
					std::string oldEntry = src + "/" + entryPath;
					std::string newEntry = dst + "/" + entryPath;
					if (!rename(file, newEntry))
					{
						printf("Failed to rename file: %s\n", file.c_str());
						return false;
					}
				}
				return remove(src);
			}
			return rename_file(src, dst);
		}

		void list(const std::string& path, std::vector<std::string>& files, bool recurse)
		{
			directory_iterator it(path, recurse);
			for (auto& file : it)
			{
				files.push_back(file);
			}
		}

		bool read_file(const std::string& path, const void* buffer, size_t size)
		{
			auto fd = open(path, SCE_KERNEL_O_RDONLY);
			if (fd < 0)
			{
				return false;
			}
			auto res = read(fd, const_cast<void*>(buffer), size);
			close(fd);
			return res >= 0;
		}

		bool read_file(const std::string& path, std::vector<uint8_t>& buffer)
		{
			auto fd = open(path, SCE_KERNEL_O_RDONLY);
			if (fd < 0)
			{
				return false;
			}
			buffer.resize(sceKernelLseek(fd, 0, SEEK_END));
			sceKernelLseek(fd, 0, SEEK_SET);
			auto res = read(fd, buffer.data(), buffer.size());
			close(fd);
			return res >= 0;
		}

		bool write_file(const std::string& path, const void* buffer, size_t size)
		{
			auto fd = open(path, SCE_KERNEL_O_WRONLY | SCE_KERNEL_O_CREAT | SCE_KERNEL_O_TRUNC, 0777);
			if (fd < 0)
			{
				return false;
			}
			auto res = write(fd, buffer, size);
			close(fd);
			return res >= 0;
		}

		bool append_file(const std::string& path, const void* buffer, size_t size)
		{
			auto fd = open(path, SCE_KERNEL_O_WRONLY | SCE_KERNEL_O_CREAT | SCE_KERNEL_O_APPEND, 0777);
			if (fd < 0)
			{
				return false;
			}
			auto res = write(fd, buffer, size);
			close(fd);
			return res >= 0;
		}

		bool copy_file(const std::string& src, const std::string& dst)
		{
			if (!exists(src))
			{
				printf("Source file does not exist: %s\n", src.c_str());
				return false;
			}

			if (!create_directory(dst))
			{
				printf("Failed to create directory: %s\n", dst.c_str());
				return false;
			}

			auto fd_src = open(src, SCE_KERNEL_O_RDONLY);
			if (fd_src < 0)
			{
				printf("Failed to open source file: %s\n", src.c_str());
				return false;
			}

			auto fd_dst = open(dst, SCE_KERNEL_O_WRONLY | SCE_KERNEL_O_CREAT | SCE_KERNEL_O_TRUNC, 0777);
			if (fd_dst < 0)
			{
				printf("Failed to open destination file: %s\n", dst.c_str());
				close(fd_src);
				return false;
			}

			char buffer[8192];
			size_t bytes_read;
			while ((bytes_read = read(fd_src, buffer, sizeof(buffer))) > 0)
			{
				auto bytes_written = write(fd_dst, buffer, bytes_read);
				if (bytes_written < 0)
				{
					printf("Failed to write to destination file: %s\n", dst.c_str());
					close(fd_src);
					close(fd_dst);
					return false;
				}
			}

			auto file_size = get_size(src);
			if(file_size != get_size(dst))
			{
				printf("Failed to copy file: %s\n", src.c_str());
				close(fd_src);
				close(fd_dst);
				return false;
			}

			close(fd_src);
			close(fd_dst);

			return true;
		}

		bool move_file(const std::string& src, const std::string& dst)
		{
			if (!exists(src))
			{
				printf("Source file does not exist: %s\n", src.c_str());
				return false;
			}

			if (!create_directory(dst))
			{
				printf("Failed to create directory: %s\n", dst.c_str());
				return false;
			}

			auto res = sceKernelRename(src.c_str(), dst.c_str());
			if (res < 0)
			{
				printf("sceKernelRename(%s, %s) failed: 0x%08X\n", src.c_str(), dst.c_str(), res);
				return false;
			}

			return true;
		}

		bool remove_file(const std::string& path)
		{
			if (!exists(path))
			{
				printf("File does not exist: %s\n", path.c_str());
				return false;
			}

			auto res = sceKernelUnlink(path.c_str());
			if (res < 0)
			{
				printf("sceKernelUnlink(%s) failed: 0x%08X\n", path.c_str(), res);
				return false;
			}

			return true;
		}

		bool rename_file(const std::string& src, const std::string& dst)
		{
			if (!exists(src))
			{
				printf("Source file does not exist: %s\n", src.c_str());
				return false;
			}

			if (!create_directory(dst))
			{
				printf("Failed to create directory: %s\n", dst.c_str());
				return false;
			}

			auto res = sceKernelRename(src.c_str(), dst.c_str());
			if (res < 0)
			{
				printf("sceKernelRename(%s, %s) failed: 0x%08X\n", src.c_str(), dst.c_str(), res);
				return false;
			}
			return true;
		}
	}
}