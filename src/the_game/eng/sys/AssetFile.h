#ifndef ASSET_FILE_H
#define ASSET_FILE_H

#include <cstddef>
#include <string>

#if defined(__ANDROID__)
	class AAsset;
	class AAssetManager;
#else
	#include <iosfwd>
#endif

namespace sys {
    class CannotOpenFileException;

    class AssetFile {
#ifdef __ANDROID__
        static AAssetManager* asset_manager_;
        AAsset* asset_file_;
#else
        std::fstream *file_stream_;
#endif
        int mode_;
        std::string name_;
        size_t size_, pos_override_;
    public:
        AssetFile(const char *file_name, int mode = IN);
        AssetFile(const std::string &file_name, int mode = IN) : AssetFile(file_name.c_str(), mode) {}

        ~AssetFile();

        size_t size() { return size_; }

        inline int mode() { return mode_; }

        std::string name() { return name_; }

        size_t pos();

        bool Read(char *buf, size_t size);

        bool ReadFloat(float &f);

#ifndef __ANDROID__
        bool Write(const char *buf, size_t size);
#endif

        void Seek(size_t pos);

        operator bool();

        enum {
            IN, OUT
        };

        static void AddPackage(const char *name);
        static void RemovePackage(const char *name);
#ifdef __ANDROID__
        static void InitAssetManager(class AAssetManager*);
        int32_t descriptor(off_t *start, off_t *len);
#endif
    };
}


#endif /* ASSET_FILE_H */
