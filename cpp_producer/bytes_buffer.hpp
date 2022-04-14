#pragma once

#include <iostream>
#include <algorithm>
#include <stdlib.h>

class BytesBuffer
{
public:
    static const uint32_t kShmBufSize = 1920 * 1080 * 3;
    static const uint32_t kReserved = 12;   // 4:reader index, 4:writer index, 4:full-check flag

    BytesBuffer(char *buf, uint32_t bufsize = kShmBufSize)
    {
        _bufsize = bufsize;
        _buf = buf;
        std::copy(buf + 8, buf + 12, (char *)&_full);
        uint32_t r = rindex();
        uint32_t w = windex();
        if (r < kReserved || r > _bufsize)
            set_rindex(kReserved);
        if (w < kReserved || w > _bufsize)
            set_windex(kReserved);
    }

     uint32_t readable_size()
    {
        uint32_t r = rindex();
        uint32_t w = windex();
        if (w == r)
        {
            if (buf_full())
                return _bufsize - kReserved;
            else
                return 0;
        }
        else if (w > r)
            return w - r;
        else
            return _bufsize - r + w - kReserved;
    }

    uint32_t writeable_size()
    {
        uint32_t r = rindex();
        uint32_t w = windex();
        if (w == r)
        {
            if (buf_full())
                return 0;
            else
                return _bufsize - kReserved;
        }
        else if (w > r)
            return _bufsize - r + w - kReserved;
        else
            return r - w;
    }

    uint32_t retrieve(char *buf, uint32_t length)
    {
        uint32_t readable = readable_size();
        if (readable == 0)
            return 0;

        uint32_t r = rindex();
        uint32_t w = windex();
        uint32_t len = 0;


        if (length < readable)
        {
            if ((r < w) || (_bufsize - r >= length))
            {
                memcpy(buf, _buf + r, length);
                r += length;
            }
            else
            {
                uint32_t new_r = length + kReserved + r - _bufsize;
                std::copy(_buf + r, _buf + _bufsize, buf);
                std::copy(_buf + kReserved, _buf + new_r, buf + r - _bufsize);
                r = new_r;
            }
            set_rindex(r);
            set_buf_full(0);
            len = length;
        }
        else
            len = retrieve_all(buf);
        return len;
    }

    bool append(const char *buf, uint32_t length)
    {
        uint32_t r = rindex();
        uint32_t w = windex();

        if (writeable_size() < length) // space not enough
            return false;
        else
        {
            if ((r < w) && (_bufsize - w < length))
            {
                std::copy(buf, buf + _bufsize - w, _buf + w);
                uint32_t new_w = kReserved + length + w - _bufsize;
                std::copy(buf + _bufsize - w, buf + length, _buf + kReserved);
                w = new_w;
            }
            else
            {
                std::copy(buf, buf + length, _buf + w);
                w += length;
                set_windex(w);
            }
            set_windex(w);
            if (w == r || abs(long(w - r)) == _bufsize - kReserved)
                set_buf_full(1);
            return true;
        }
    }

    uint32_t retrieve_all(char *buf)
    {
        uint32_t readable = readable_size();
        if (readable == 0)
            return 0;

        uint32_t r = rindex();
        uint32_t w = windex();
        uint32_t len = 0;
        if (w > r)
        {
            std::copy(_buf + r, _buf + w, buf);
            len = w - r;
        }
        else
        {
            std::copy(_buf + r, _buf + _bufsize, buf);
            len = _bufsize - r;
            std::copy(_buf + kReserved, _buf + w, buf + len);
            len += w - kReserved;
        }

        set_rindex(kReserved);
        set_windex(kReserved);
        set_buf_full(0);
        return len;
    }

private:
    uint32_t rindex()
    {
        uint32_t r = 0;
        std::copy(_buf, _buf + 4, (char *)&r);
        return r;
    }

    uint32_t windex()
    {
        uint32_t w = 0;
        std::copy(_buf + 4, _buf + 8, (char *)&w);
        return w;
    }

    void set_rindex(uint32_t r)
    {
        std::copy((char *)&r, (char *)&r + 4, _buf);
    }

    void set_windex(uint32_t w)
    {
        std::copy((char *)&w, (char *)&w + 4, _buf + 4);
    }

    void set_buf_full(uint32_t f = 1)
    {
        uint32_t full = f;
        std::copy((char *)&full, (char *)&full + 4, _buf + 8);
    }

    bool buf_full()
    {
        uint32_t full = 0;
        std::copy(_buf + 8, _buf + 12, (char *)&full);
        return full == 1;
    }

private:
    uint32_t _full = 0;
    uint32_t _bufsize = 0;
    char *_buf = 0;
};
