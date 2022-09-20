import os
import cv2
import numpy as np

def FastFourierTransform(frame, size = 60, thresh = 17):
	# compute the FFT to find the frequency transform, then shift
	# the zero frequency component (i.e., DC component located at
	# the top-left corner) to the center where it will be more
	# easy to analyze
    h, w = frame.shape[:2]
    cX, cY = int(w / 2.0), int(h / 2.0)
    fft = np.fft.fft2(frame)

    # zero-out the center of the FFT shift (i.e., remove low
	# frequencies), apply the inverse shift such that the DC
	# component once again becomes the top-left, and then apply
	# the inverse FFT
    fftShift = np.fft.fftshift(fft)
    fftShift[cY - size:cY + size, cX - size:cX + size] = 0
    fftShift = np.fft.ifftshift(fftShift)
    recon = np.fft.ifft2(fftShift)

    # compute the magnitude spectrum of the reconstructed image,
	# then compute the mean of the magnitude values
    mag = 20 * np.log(np.abs(recon))
    mean = np.mean(mag)
    
    return mean, (mean <= thresh)

def main(vidPath, savePath):
    cap = cv2.VideoCapture(vidPath)
    num = 0
    if (cap.isOpened() == False):
        print("Open video failed.")
    else:
        while cap.isOpened():
            success, frame = cap.read()
            if success:
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                h, w = gray.shape[:2]

                # resize all frame to choose the same threshold
                ratio =  800 / h
                res = cv2.resize(gray, (int(w * ratio), 800), cv2.INTER_AREA)

                mean, blurry = FastFourierTransform(res)
                if blurry:
                    pass
                    # print(f"Frame {num}: {mean} --> Blur\n")
                else:
                    # print(f"Frame {num}: {mean} --> Not blur\n")
                    cv2.imwrite(f"{savePath}/Frame{num}.jpg", frame)

                num += 1
            else:
                break
    cap.release()

if __name__ == "__main__":
    save = "/Users/Quoc_Anh/Downloads/non_blur"
    vid = "/Users/Quoc_Anh/Downloads/anhgau.mp4"

    if (os.path.isdir(save) == False):
        os.mkdir(save)

    main(vid, save)
    print("Done.")