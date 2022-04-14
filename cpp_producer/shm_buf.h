#pragma once
#include <stdint.h>
#include <string>

class BytesBuffer;
class SharedMemoryBuffer
{
public:
    SharedMemoryBuffer(const char *shm_name, uint32_t bufsize = 0, bool delete_shm = false);
    ~SharedMemoryBuffer();
    static void remove_shm(const char *shm_name);
    bool write_shm(const char *data, uint32_t len);
    uint32_t read_shm(std::string& data);
    bool readable();

private:
    bool init_shm();

    bool _delete_shm = false;
    uint32_t _bufsize = 0;
    const char *_shm_name;
    BytesBuffer *_bytesBuf = 0;
};
