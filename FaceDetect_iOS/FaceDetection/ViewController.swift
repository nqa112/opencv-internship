//
//  ViewController.swift
//  FaceDetection
//
//  Created by Nguyen Quoc Anh on 09/09/2022.
//

import UIKit
import AVFoundation

class ViewController: UIViewController, AVCaptureVideoDataOutputSampleBufferDelegate {
    @IBOutlet weak var imageView: UIImageView!
    
    private let videoDataOutput = AVCaptureVideoDataOutput()
    private var captureSession: AVCaptureSession = AVCaptureSession()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
        self.addCameraInput()
        self.getFrames()
        self.captureSession.startRunning()
    }
    

    // Choose camera device
    // and wrap it into a capture input
    private func addCameraInput() {
        guard let device = AVCaptureDevice.DiscoverySession(
            deviceTypes: [.builtInWideAngleCamera, .builtInDualCamera, .builtInTrueDepthCamera],
            mediaType: .video,
            position: .front).devices.first else {
                fatalError("No back camera device found, please make sure to in an iOS device and not a simulator")
        }
        let cameraInput = try! AVCaptureDeviceInput(device: device)
        self.captureSession.addInput(cameraInput)
    }
    
    
    // Configure input frame from capture session
    // and where to send output frame
    private func getFrames() {
        videoDataOutput.videoSettings = [(kCVPixelBufferPixelFormatTypeKey as NSString) : NSNumber(value: kCVPixelFormatType_32BGRA)] as [String : Any]
        
        videoDataOutput.alwaysDiscardsLateVideoFrames = true
    
        videoDataOutput.setSampleBufferDelegate(self, queue: DispatchQueue(label: "camera.frame.processing.queue"))
        
        self.captureSession.addOutput(videoDataOutput)
        
        guard let connection = self.videoDataOutput.connection(with: AVMediaType.video),
            connection.isVideoOrientationSupported else { return }
        
        connection.videoOrientation = .portrait
    }
    
    
    // Convert frame into UIImage
    // Image processing algorithms implemented here
    func captureOutput(
        _ output: AVCaptureOutput,
        didOutput sampleBuffer: CMSampleBuffer,
        from connection: AVCaptureConnection) {
        // PROCESS THE FRAME HERE
        guard let  imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer) else { return }
            
        CVPixelBufferLockBaseAddress(imageBuffer, CVPixelBufferLockFlags.readOnly)
            
        let baseAddress = CVPixelBufferGetBaseAddress(imageBuffer)
        let bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer)
        let width = CVPixelBufferGetWidth(imageBuffer)
        let height = CVPixelBufferGetHeight(imageBuffer)
        let colorSpace = CGColorSpaceCreateDeviceRGB()
            
        var bitmapInfo: UInt32 = CGBitmapInfo.byteOrder32Little.rawValue
        bitmapInfo |= CGImageAlphaInfo.premultipliedFirst.rawValue & CGBitmapInfo.alphaInfoMask.rawValue
            
        let context = CGContext(data: baseAddress, width: width, height: height, bitsPerComponent: 8, bytesPerRow: bytesPerRow, space: colorSpace, bitmapInfo: bitmapInfo)
            
        guard let quartzImage = context?.makeImage() else { return }
            
        CVPixelBufferUnlockBaseAddress(imageBuffer, CVPixelBufferLockFlags.readOnly)
            
        // Detect face
        let image = UIImage(cgImage: quartzImage)
        let detectFrame = FaceDetectBridge.detectFace(image)
        DispatchQueue.main.async {
            self.imageView.image = detectFrame
        }
    }
}
