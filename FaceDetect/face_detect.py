import cv2
import argparse

def main(model):
    # Define video capture object with camera device index
    cap = cv2.VideoCapture(0)

    if not cap.isOpened():
        print("Cannot open camera.")
        exit()

    while True:
        # Capture frame by frame
        ret, frame = cap.read()
        # If frame is read correctly, ret is True
        if not ret:
            print("Can't receive frame (stream end?). Exiting ...")
            break

        # Process frame here
        detected_frame = faceDetect(frame, model)

        # Display processed frame
        cv2.imshow("Face Detection", detected_frame)

        # Set button "Space" for terminating program and frame latency
        if cv2.waitKey(1) == 32:
            break
    
    # After the loop release the cap object
    cap.release()
    # Destroy all the windows
    cv2.destroyAllWindows()


def faceDetect(image, model):

    # Resize image 
    hImg, wImg = image.shape[:2]
    ratio = 480 / wImg
    image = cv2.resize(image, (480, int(hImg * ratio)), interpolation=cv2.INTER_AREA) 
    # Convert to grayscale
    grayImg = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    # Call Haar Cascades model and detect face
    detector = cv2.CascadeClassifier(model)
    # "haarcascade_frontalface_alt2" is chosen
    # scaleFactor=1.1, minNeighbors=7, minSize=(30, 30)
    bbox = detector.detectMultiScale(grayImg, scaleFactor=1.1, minNeighbors=7, 
                                    minSize=(30, 30), flags=cv2.CASCADE_SCALE_IMAGE)

    # Draw detected rectangular bounding box
    for (x, y, w, h) in bbox:
        cv2.rectangle(image, (x, y - int(0.05 * h)), (x + w, y + h + int(0.05 * h)), (0, 255, 0), 2)

    return image


def parsing():

    ap = argparse.ArgumentParser()
    ap.add_argument("--model", required=True, type=str, help="Haar Cascades model path")
    args = ap.parse_args()

    return args


if __name__ == "__main__":

    args = parsing()
    main(args.model)