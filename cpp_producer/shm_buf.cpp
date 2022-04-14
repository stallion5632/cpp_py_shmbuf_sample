#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "bytes_buffer.hpp"
#include "shm_buf.h"
using namespace boost::interprocess;
mapped_region *s_region = 0;


SharedMemoryBuffer::SharedMemoryBuffer(const char *shm_name, uint32_t bufsize, bool delete_shm)
{
    _shm_name = shm_name;
    _bufsize = bufsize;
    _delete_shm = delete_shm;
    if (!init_shm())
        std::cerr << "init_shm failed!" << std::endl;
}

SharedMemoryBuffer::~SharedMemoryBuffer()
{   
    if (s_region)
        delete s_region;

    if (_delete_shm)
        shared_memory_object::remove(_shm_name);
}

void SharedMemoryBuffer::remove_shm(const char *shm_name)
{
    shared_memory_object::remove(shm_name);
}

bool SharedMemoryBuffer::init_shm()
{
    try
    {
        shared_memory_object shm;
        try
        {
            shm = shared_memory_object(open_only, _shm_name, read_write);
            boost::interprocess::offset_t size;
            shm.get_size(size);
            _bufsize = (uint32_t)size;
        }
        catch(const std::exception& e)
        {
            if (_bufsize > 0)
            {
                shm = shared_memory_object(create_only, _shm_name, read_write);
                shm.truncate(_bufsize);
            }
            else
            {
                std::cerr << "open or create shm " << _shm_name << "failed!" << std::endl;
                return false;
            }
        }
        boost::interprocess::offset_t size;
        if (shm.get_size(size))
           std::cout << shm.get_name() << ", size:" << size << std::endl;
        if (size == 0)
            return false;

        // Map the whole shared memory in this process
        s_region = new mapped_region(shm, read_write);
        _bytesBuf = new BytesBuffer((char *)s_region->get_address(), _bufsize);
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
        return false;
    }
}

bool SharedMemoryBuffer::write_shm(const char *data, uint32_t len)
{
    bool ret = false;
    ret = _bytesBuf->append((char *)&len, 4);
    if (!ret)
        return false;
    ret = _bytesBuf->append(data, len);
    if (!ret)
        return false;
    return true;
}

bool SharedMemoryBuffer::readable()
{
    return _bytesBuf->readable_size() > 0 ? true : false;
}

uint32_t SharedMemoryBuffer::read_shm(std::string& data)
{
    uint32_t len = 0;
    if (readable())
        _bytesBuf->retrieve((char *)&len, 4);
    else
        return 0;

    data.resize(len);
    if (_bytesBuf->readable_size() >= len)
        len = _bytesBuf->retrieve((char *)data.data(), len);
    else
        return 0;
    return len;
}
