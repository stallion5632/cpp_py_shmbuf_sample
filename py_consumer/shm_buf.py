# -*- coding: utf-8 -*-

from multiprocessing.shared_memory import SharedMemory
from bytes_buffer import BytesBuffer, kShmBufSize
from mprt_monkeypatch import remove_shm_from_resource_tracker as patch

patch()

class SharedMemoryBuffer:
    def __init__(self, shm_name: str, bufsize: int = kShmBufSize, 
        delete_shm : bool = False):
        self.bytesbuf = None
        self.shm = None
        self.delete_shm = False
        self.bufsize = bufsize
        self.shm = self._get_shm(shm_name)
        self.bytesbuf = BytesBuffer(self.shm.buf, self.bufsize)
        self.delete_shm = delete_shm
    
    def __del__(self):
        if self.bytesbuf:
            del self.bytesbuf
        if self.shm:
            self.shm.close()
        if self.delete_shm:
            self.shm.unlink()
        pass

    def _get_shm(self, shm_name: str):
        shm = None
        if shm_name:
            if self.bufsize == 0:
                try:
                    shm = SharedMemory(name=shm_name, create=False)
                    self.bufsize = shm.size
                    print(f'shm.size(): {shm.size()}')
                except:
                    print(f'open shm {shm_name} failed!')
            else:
                try:
                    shm = SharedMemory(name=shm_name, create=True, size=self.bufsize)
                except FileExistsError:
                    try:
                        shm = SharedMemory(name=shm_name, create=False)
                        self.bufsize = shm.size
                    except:
                        print(f'open shm {shm_name} failed!')
        return shm

    def write_shm(self, data: bytes):

        ok = False
        data_len = len(data)
        data = bytearray().join([data_len.to_bytes(length=4, byteorder="little"), data])
        ok = self.bytesbuf.append(data)
        return ok

    def read_shm(self):

        data = None
        if self.readable():
            data_len = self.bytesbuf.retrieve_as_int(4)
        if self.readable():
            data = self.bytesbuf.retrieve(data_len)
        return data

    def readable(self):
        return True if self.bytesbuf.readable_size() > 0 else False


if __name__ == "__main__":
    shm_name = "shm_name_test"
    bytes_data = b'123abc'
    shm_buf = SharedMemoryBuffer(shm_name, True)
    shm_buf.write_shm(bytes_data)
    bytes_data = shm_buf.read_shm()
    print(bytes(bytes_data))
