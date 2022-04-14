# -*- coding: utf-8 -*-

import time
import multiprocessing as mp
from shm_buf import SharedMemoryBuffer
import numpy as np
import cv2

'''
Suppose the video is 3-channel 1080p 
'''
WIDTH = 1920
HEIGHT = 1080
CHANNELS = 3

def consumer():
    shm_name = "shm_name_test"
    shm_buf = SharedMemoryBuffer(shm_name)
    while True:
        if not shm_buf.readable():
            time.sleep(0.1)
            continue
        bytes_data = shm_buf.read_shm()
        if bytes_data and len(bytes_data) == HEIGHT*WIDTH*CHANNELS:
            img = np.frombuffer(bytes_data, dtype=np.uint8)
            img = img.reshape((HEIGHT, WIDTH, CHANNELS))
            cv2.imshow("img", img)
            cv2.waitKey(1)


if __name__ == "__main__":

    consume_proc = mp.Process(target=consumer, args=(), daemon=True)
    consume_proc.start()
    consume_proc.join()
