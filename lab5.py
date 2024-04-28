from ultralytics import YOLO
import threading
import queue
import time
import cv2
import argparse


lock = threading.Lock()
result_queue = []




def fun_thread_read(path_video: str, frame_queue: queue.Queue, event_stop: threading.Event):
    cap = cv2.VideoCapture(path_video)
    id_frame = 0
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            print("End of video!")
            break
        frame_queue.put((frame, id_frame))
        id_frame += 1
        time.sleep(0.0001)
    event_stop.set()

def fun_thread_safe_predict(frame_queue: queue.Queue, event_stop: threading.Event):
    global result_queue, lock
    model = YOLO(model="yolov8n-pose.pt")
    while True:
        try:
            frame, idd = frame_queue.get(timeout=1)
            result_queue.append((model.predict(source=frame, device='cpu')[0].plot(), idd))
        except queue.Empty:
            if event_stop.is_set():
                print(f'Thread {threading.get_ident()} final!')
                break
    
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--vid', type=str, default="tst.mp4",  help='input video')
    parser.add_argument('--thr', type=int, default=2, help='number threads')
    parser.add_argument('--out', type=str, default="result.mp4",  help='output video')
    args = parser.parse_args()

    threads = []
    frame_queue = queue.Queue(1000)
    event_stop = threading.Event()
    video_path = args.vid
    thread_read = threading.Thread(target=fun_thread_read, args=(video_path, frame_queue, event_stop,))
    thread_read.start()
    start_t = time.monotonic()
    for _ in range(args.thr):
        threads.append(threading.Thread(target=fun_thread_safe_predict, args=(frame_queue, event_stop,)))

    for thr in threads:
        thr.start()

    for thr in threads:
        thr.join()

    thread_read.join()
    end_t = time.monotonic()
    print(f'Time: {end_t - start_t}')
    
    w = result_queue[0][0].shape[1]
    h = result_queue[0][0].shape[0]
    
    vidos = cv2.VideoWriter(args.out, -1, 30, (w, h))
    
    srt = sorted(result_queue, key=lambda x: x[1])
    
    for s in srt:
        vidos.write(s[0])
        
    vidos.release()