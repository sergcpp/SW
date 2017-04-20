#include "AssetFile.h"

#include <cassert>
#include <cstring>
#include <sstream>
#include <stdexcept>

#ifdef __ANDROID__
    #include <android/asset_manager.h>
    #include <android/asset_manager_jni.h>
#else
	#include <iostream>
    #include <fstream>
#endif

#include "Pack.h"

namespace sys {
	class CannotOpenFileException : public std::runtime_error {
		std::string file_name_;

		static std::ostringstream cnvt;
	public:
		CannotOpenFileException(const char *file_name) : std::runtime_error("Cannot open file!"), file_name_(file_name) {}
		~CannotOpenFileException() throw() {}

		const char *what() const throw() {
			cnvt.str("");
			cnvt << std::runtime_error::what() << " : \"" << file_name_ << "\"" << std::endl;
			return cnvt.str().c_str();
		}
	};

    struct Package {
        std::string name;
        std::vector<sys::FileDesc> file_list;
    };

    std::vector<Package> added_packages;
}


std::ostringstream sys::CannotOpenFileException::cnvt;

#ifdef __ANDROID__
AAssetManager* sys::AssetFile::asset_manager_ = nullptr;
#endif

sys::AssetFile::AssetFile(const char *file_name, int mode) : mode_(mode), name_(file_name), size_(0), pos_override_(0) {
    using namespace std;

    if (mode == IN) {
#ifdef __ANDROID__
    if(file_name[0] == '.' && file_name[1] == '/') {
        file_name += 2;
    }

    string full_path;
    if(strstr(file_name, "..")) {
        char path[100];
        char* toks[16];
        int num_toks = 0;
        strcpy(path, file_name);
        char* p = strtok(path, "/");
        while (p) {
            if (strstr(p, "..")) {
                num_toks -= 1;
                assert(num_toks >= 0);
            } else {
                toks[num_toks++] = p;
            }
            p = strtok(NULL, "/");
        }
        for (int i = 0; i < num_toks; i++) {
            full_path.append(toks[i]);
            if (i != num_toks - 1) {
                full_path.append("/");
            }
        }
        file_name = full_path.c_str();
    }

	asset_file_ = AAssetManager_open(asset_manager_, file_name, AASSET_MODE_BUFFER);
	if (!asset_file_) {
		throw CannotOpenFileException(file_name);
	}

    size_ = AAsset_getLength(asset_file_);
#else
        file_stream_ = new std::fstream();

        string fname = file_name;
        for (auto &p : added_packages) {
            for (auto &f : p.file_list) {
                if (fname == f.name) {
                    file_stream_->open(p.name, std::ios::in | std::ios::binary);
                    if (!file_stream_->good()) { throw sys::CannotOpenFileException(file_name); }
                    file_stream_->seekg(f.off, ios::beg);
                    pos_override_ = f.off;
                    size_ = f.size;
                    goto OPENED;
                }
            }
        }

        file_stream_->open(file_name, std::ios::in | std::ios::binary);
        file_stream_->seekg(0, std::ios::end);
        size_ = (size_t) file_stream_->tellg();
        file_stream_->seekg(0, std::ios::beg);
OPENED:
		if (!file_stream_->good()) { throw sys::CannotOpenFileException(file_name); }
#endif
    } else if (mode == OUT) {
#ifdef __ANDROID__
		throw std::runtime_error("Cannot write to assets folder!");
#else
		file_stream_ = new std::fstream();
        file_stream_->open(file_name, std::ios::out | std::ios::binary);
        if (!file_stream_->good()) {
            cout << "Can`t open file " << file_name << endl;
            throw CannotOpenFileException(file_name);
        }
#endif
    }
}

sys::AssetFile::~AssetFile() {
#ifdef __ANDROID__
	AAsset_close(asset_file_);
#else
    delete file_stream_;
#endif
}

bool sys::AssetFile::Read(char *buf, size_t size) {
    assert(mode_ == IN);
#ifdef __ANDROID__
	return !(AAsset_read(asset_file_, buf, size) < 0);
#else
    assert(file_stream_);
    file_stream_->read(buf, size);
    return bool(*file_stream_);
#endif
}

void sys::AssetFile::Seek(size_t pos) {
#ifdef __ANDROID__
	AAsset_seek(asset_file_, pos, SEEK_SET);
#else
    file_stream_->seekg(pos_override_ + pos);
#endif
}

sys::AssetFile::operator bool() {
#ifdef __ANDROID__
	return bool(AAsset_getLength(asset_file_));
#else
    return file_stream_ != nullptr && bool(*file_stream_);
#endif
}

bool sys::AssetFile::ReadFloat(float &f) {
    return this->Read((char *) &f, sizeof(float));
}

#ifndef __ANDROID__
bool sys::AssetFile::Write(const char *buf, size_t size) {
	assert(file_stream_);
	file_stream_->write(buf, size);
	return bool(*file_stream_);
}
#endif

size_t sys::AssetFile::pos() {
#ifdef __ANDROID__
	return AAsset_seek(asset_file_, 0, SEEK_CUR);
#else
    return (size_t)file_stream_->tellg() - pos_override_;
#endif
}

#ifdef __ANDROID__
void sys::AssetFile::InitAssetManager(class AAssetManager* am)
{
	asset_manager_ = am;
}
int32_t sys::AssetFile::descriptor(off_t *start, off_t *len) {
    return AAsset_openFileDescriptor(asset_file_, start, len);
}
#endif

void sys::AssetFile::AddPackage(const char *name) {
    size_t ln = strlen(name);
    if (ln < 6 || name[ln - 5] != '.' || name[ln - 4] != 'p' || name[ln - 3] != 'a' ||
            name[ln - 2] != 'c' || name[ln - 1] != 'k') {
        throw std::runtime_error("Invalid package file!");
    }
	added_packages.emplace_back();
	Package &p = added_packages.back();
	p.name = name;
	p.file_list = sys::EnumFilesInPackage(name);
}

void sys::AssetFile::RemovePackage(const char *name) {
    for (auto it = added_packages.begin(); it != added_packages.end(); ++it) {
        if (it->name == name) {
            added_packages.erase(it);
            return;
        }
    }
}