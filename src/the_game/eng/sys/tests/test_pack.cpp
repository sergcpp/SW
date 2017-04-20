#include "test_common.h"

#include <algorithm>
#include <fstream>
#include <memory>

#include "../AssetFile.h"
#include "../Pack.h"

namespace {
    std::vector<std::string> file_list = { "./constant.fs", "./src/CMakeLists.txt" };
}

void test_pack() {

    {   // Save/Load package
        sys::WritePackage("./my_pack.pack", file_list);

        auto OnFile = [](const char *name, void *data, int size){
            auto it = std::find(file_list.begin(), file_list.end(), name);
            assert(it != file_list.end());

            sys::AssetFile in_file(name, sys::AssetFile::IN);
            assert(in_file.size() == size);

            std::unique_ptr<char[]> buf(new char[size]);
            in_file.Read(buf.get(), (size_t)size);
            const char *p1 = (char*)data;
            const char *p2 = buf.get();
            assert(memcmp(p1, p2, (size_t)size) == 0);
        };
        sys::ReadPackage("./my_pack.pack", OnFile);

        std::vector<sys::FileDesc> list = sys::EnumFilesInPackage("./my_pack.pack");

        assert(std::string(list[0].name) == "./constant.fs");
        assert(std::string(list[1].name) == "./src/CMakeLists.txt");
    }

    {   // Add package to AssetFile
        sys::AssetFile::AddPackage("./my_pack.pack");

        sys::AssetFile in_file1("./constant.fs", sys::AssetFile::IN);
        std::ifstream in_file2("./constant.fs", std::ios::ate | std::ios::binary);
        size_t size = (size_t) in_file2.tellg();
        in_file2.seekg(0, std::ios::beg);
        assert(in_file1.size() == size);

        std::unique_ptr<char[]> buf1(new char[size]), buf2(new char[size]);
        assert(in_file1.Read(buf1.get(), size));
        in_file2.read(buf2.get(), size);
        assert(memcmp(buf1.get(), buf2.get(), size) == 0);
    }
}
