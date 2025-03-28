import cv2
import numpy as np

def load_nv12_opencv(file_path, width, height, outputPath):
    with open(file_path, 'rb') as f:
        data = np.frombuffer(f.read(), dtype=np.uint8)
    # 转换为NV12格式的OpenCV矩阵
    nv12 = data.reshape(height * 3 // 2, width)
    rgb = cv2.cvtColor(nv12, cv2.COLOR_YUV2RGB_NV21)
    cv2.imwrite(outputPath, rgb)

for item in range(0, 10):
    load_nv12_opencv(f"/home/dml/dvpp/image/image{item}.png", 1280, 720, f"output{item}.jpg")
