from multiprocessing import Process, resource_tracker
from multiprocessing.shared_memory import SharedMemory


def remove_shm_from_resource_tracker():
    """Monkey-patch multiprocessing.resource_tracker so SharedMemory won't be tracked

    More details at: https://bugs.python.org/issue38119
    """

    def fix_register(name, rtype):
        if rtype == "shared_memory":
            return
        return resource_tracker._resource_tracker.register(self, name, rtype)
    resource_tracker.register = fix_register

    def fix_unregister(name, rtype):
        if rtype == "shared_memory":
            return
        return resource_tracker._resource_tracker.unregister(self, name, rtype)
    resource_tracker.unregister = fix_unregister

    if "shared_memory" in resource_tracker._CLEANUP_FUNCS:
        del resource_tracker._CLEANUP_FUNCS["shared_memory"]


def create_shm():
    remove_shm_from_resource_tracker()

    print("create_shm started")
    shared_mem = SharedMemory(create=True, size=1024, name="python_testshm")
    shared_mem.buf.obj.write(b"X" * 1024)
    shared_mem.close()
    print("create_shm finished")

def destroy_shm():
    remove_shm_from_resource_tracker()

    print("destroy_shm started")
    shared_mem = SharedMemory(create=False, name="python_testshm")
    result = shared_mem.buf.tobytes()
    shared_mem.close()
    shared_mem.unlink()
    print("destroy_shm finished")

def main():
    p1 = Process(target=create_shm)
    p1.start()
    p1.join()
    p2 = Process(target=destroy_shm)
    p2.start()
    p2.join()


if __name__ == "__main__":
    main()
