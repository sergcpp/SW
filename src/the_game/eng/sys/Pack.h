#ifndef PACK_H
#define PACK_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace sys {
	typedef void(*onfile_func)(const char *name, void *data, int size);

	struct FileDesc {
		char name[120];
		uint32_t off, size;
	};
	static_assert(sizeof(FileDesc) == 128, "!!!");
	static_assert(offsetof(FileDesc, off) == 120, "!!!");
	static_assert(offsetof(FileDesc, size) == 124, "!!!");

	void ReadPackage(const char *pack_name, onfile_func on_file);
	void WritePackage(const char *pack_name, std::vector<std::string> &file_list);

	std::vector<FileDesc> EnumFilesInPackage(const char *pack_name);

	bool ReadFromPackage(const char *pack_name, const char *fname, size_t pos, char *buf, size_t size);
	bool ReadFromPackage(const char *pack_name, const char *fname, size_t pos, char *buf, size_t size);
}

#endif // PACK_H