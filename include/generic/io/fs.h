#ifndef _generic_io_fs_h_
#define _generic_io_fs_h_

#include <cstdio>
#include <filesystem>
#include <string>
#include <system_error>

namespace generic
{
namespace io
{

//Policy based file access. _FileIO supplies the concrete backend so a caller can
//swap plain file access for an archive or virtual filesystem without touching
//the code that reads/writes. Semantics mirror C stdio: read/write take
//(buffer, elementSize, elementCount, file) and return the number of *elements*
//transferred.
template<typename _FileIO>
struct fs
{
    typedef typename _FileIO::Type Type;

    static bool exists(const std::string &path) { return _FileIO::exists(path); }
    static bool create_directory(const std::string &path) { return _FileIO::create_directory(path); }

    static Type *open(const std::string &path, const char *mode) { return _FileIO::open(path, mode); }
    static size_t read(void *buffer, size_t size, size_t count, Type *file) { return _FileIO::read(buffer, size, count, file); }
    static size_t write(const void *buffer, size_t size, size_t count, Type *file) { return _FileIO::write(buffer, size, count, file); }
    static void close(Type *file) { _FileIO::close(file); }
};

//Default backend, C stdio on top of std::filesystem.
struct StdFileIO
{
    typedef FILE Type;

    static bool exists(const std::string &path)
    {
        std::error_code error;
        return std::filesystem::exists(path, error);
    }

    static bool create_directory(const std::string &path)
    {
        std::error_code error;
        return std::filesystem::create_directories(path, error);
    }

    static Type *open(const std::string &path, const char *mode) { return std::fopen(path.c_str(), mode); }
    static size_t read(void *buffer, size_t size, size_t count, Type *file) { return std::fread(buffer, size, count, file); }
    static size_t write(const void *buffer, size_t size, size_t count, Type *file) { return std::fwrite(buffer, size, count, file); }

    static void close(Type *file)
    {
        if(file)
            std::fclose(file);
    }
};

}//namespace io
}//namespace generic

#endif //_generic_io_fs_h_
