# -*- coding: utf-8 -*-

kReserved: int = 12
kShmBufSize: int = 1920 * 1080 * 3

class BytesBuffer:

    def __init__(self, buf: memoryview, bufsize: int = kShmBufSize):
        self.bufsize = bufsize
        self.buf = buf
        r = self.rindex()
        w = self.windex()
        if r < kReserved or r > bufsize:
            self.set_rindex(kReserved)
        if w < kReserved or w > bufsize:
            self.set_windex(kReserved)
       
    def rindex(self):
        '''
        read index
        '''
        return int.from_bytes(self.buf[0:4], "little")

    def windex(self):
        '''
        write index
        '''
        return int.from_bytes(self.buf[4:8], "little")

    def set_rindex(self, r):
        '''
        update read index
        '''
        self.buf[0:4] = r.to_bytes(4, "little")

    def set_windex(self, w):
        '''
        update write index
        '''
        self.buf[4:8] = w.to_bytes(4, "little")
    
    def set_buf_full(self, f : int = 1):
        '''
        set the shared memory buffer flag full
        '''
        full = f
        self.buf[8:12] = full.to_bytes(4, "little")
    
    def buf_full(self):
        '''
        check if the shared memory buffer is full
        '''
        return 1 == int.from_bytes(self.buf[8:12], "little")

    def append(self, b : bytearray):
        '''
        append the byte data to shared memory buffer
        '''
        length = len(b)
        r = self.rindex()
        w = self.windex()

        if self.writeable_size() < length: # space not enough
            return False 
        else:
            if (r < w) and (self.bufsize - w < length): # 右边的空间不够，需要使用到左边空间
                self.buf[w: self.bufsize] = b[:self.bufsize-w]
                new_w = kReserved + length + w - self.bufsize
                self.buf[kReserved: new_w] = b[self.bufsize-w:]
                w = new_w
            else:
                self.buf[w: w + length] = b
                w += length
            self.set_windex(w)
            
            if w == r or abs(w - r) == self.bufsize - kReserved:
                self.set_buf_full(1)
            
            return True

    def retrieve_all(self):
        '''
        retrieve all the byte from shared memory buffer
        '''
        if self.readable_size() == 0:
            return None
        r = self.rindex()
        w = self.windex()
        if w > r: 
            b = self.buf[r:w]
        else:
            b = bytearray().join([self.buf[r:self.bufsize], self.buf[kReserved:w]])
        self.set_rindex(kReserved)
        self.set_windex(kReserved)
        self.set_buf_full(0)
        return b

    def readable_size(self):
        '''
        return the readable byte size of shared memory buffer
        '''
        r = self.rindex()
        w = self.windex()
        if w == r:
            if self.buf_full():
                return self.bufsize - kReserved
            else:
                return 0
        elif w > r:
            return w - r
        else:
            return self.bufsize -r + w - kReserved

    def writeable_size(self):
        '''
        return the writeable byte size of shared memory buffer
        '''
        r = self.rindex()
        w = self.windex()
        if w == r:
            if self.buf_full():
                return 0
            else:
                return self.bufsize - kReserved
        elif w > r:
            return self.bufsize -r + w - kReserved
        else:
            return r - w 

    def retrieve(self, length : int):
        '''
        retrieve the bytes of shared memory size
        '''
        readable = self.readable_size()
        if readable == 0:
            return None
        r = self.rindex()
        w = self.windex()
        b = None
        if length < readable:
            if r < w or self.bufsize - r >= length: 
                # 两种情况下直接读取，r<w或 r>w,且右边的区域比较大
                b = self.buf[r:r + length]
                r += length
            else:
                new_r = length + kReserved + r - self.bufsize
                b = bytearray().join([self.buf[r: self.bufsize], self.buf[kReserved:new_r]])
                r = new_r 
            self.set_rindex(r)
            self.set_buf_full(0)
        else:
            b = self.retrieve_all()
        return b

    def retrieve_as_int(self, length : int):
        '''
        retrieve 4 bytes and convert to int
        '''
        if self.readable_size() >= 4:
            b = self.retrieve(length)
            if len(b) == 4:
                return int.from_bytes(b, "little")

if __name__ == "__main__":
    array = bytearray(kShmBufSize)
    buf = BytesBuffer(array)
    buf.append(b"123abc")
    b = buf.retrieve(3)
    print(b)
    buf.append(b"456")
    b = buf.retrieve_all()
    print(b)

    buf.append(b"123abc")
    b = buf.retrieve(3)
    print(b)
    buf.append(b"456")
    buf.append(b"ef")
    b = buf.retrieve_all()
    print(b)