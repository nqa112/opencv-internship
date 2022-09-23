package com.example.facedetectionx

import android.Manifest
import android.content.pm.PackageManager
import android.content.res.Resources
import android.graphics.*
import android.graphics.drawable.BitmapDrawable
import android.graphics.drawable.ShapeDrawable
import android.graphics.drawable.shapes.RectShape
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.DisplayMetrics
import android.util.Log
import android.util.Size
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.example.facedetectionx.databinding.ActivityMainBinding
import org.opencv.core.Core
import org.opencv.core.CvType.CV_8UC1
import org.opencv.core.Mat
import java.io.File
import java.nio.ByteBuffer
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors

class MainActivity : AppCompatActivity() {
    private lateinit var viewBinding: ActivityMainBinding
    private lateinit var cameraExecutor: ExecutorService

    // Declare device display
    private val displayMetrics: DisplayMetrics = Resources.getSystem().displayMetrics
    // Set target resolution
    private val screenW = 720
    private val screenH = 1280

    companion object {
        // Declare default temporary file location in Android
        var tmpDir: String? = System.getProperty("java.io.tmpdir")
        private const val TAG = "Face Detection"
        private const val REQUEST_CODE_PERMISSIONS = 10
        private val REQUIRED_PERMISSIONS = arrayOf(Manifest.permission.CAMERA)
        init {
            System.loadLibrary("facedetectionx")
            System.loadLibrary("opencv_java4")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {

        super.onCreate(savedInstanceState)
        viewBinding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(viewBinding.root)

        // Cannot load Haar Cascade file because path to OpenCV resources cannot be accessed directly(Android security)
        // So put model file manually in "/res/raw" resource folder
        val haarRelativePath = "/res/raw/haarcascade_frontalface_alt2.xml"
        // then read file as input stream and copy stream to accessible temporary file
        val haarCascadeIns = javaClass.getResourceAsStream(haarRelativePath)
        // JNI now can access temporary file and utilize model
        val tmpFile =  File(tmpDir, "haarcascade_frontalface.xml")
        haarCascadeIns.use { input ->
            tmpFile.outputStream().use { output ->
                input?.copyTo(output)
            }
        }

        // Draw faceRect onto a bitmap
        // and update bitmap after each interval
        Handler(Looper.getMainLooper()).postDelayed(object : Runnable {
            // Camera X Surface View is mirrored
            // So face bounding box should be mirrored too
            private fun Bitmap.flip(x: Float, y: Float, cx: Float, cy: Float): Bitmap {
                val matrix = Matrix().apply { postScale(x, y, cx, cy) }
                return Bitmap.createBitmap(this, 0, 0, width, height, matrix, true)
            }

            override fun run() {
                val bitmap: Bitmap = Bitmap.createBitmap(displayMetrics, screenW, screenH, Bitmap.Config.ARGB_8888)
                val canvas = Canvas(bitmap)
                var shapeDrawable: ShapeDrawable

                // get bounding box coordinates from created temp file
                val faceInfoFile = File(tmpDir, "faceCoordinates.txt")
                if (faceInfoFile.exists()) {
                    faceInfoFile.forEachLine { it ->
                        val faceCoordinates: Array<Int> = arrayOf(*it.split(",").toTypedArray()).map{ it.toInt() }.toTypedArray()
                        if (faceCoordinates.size == 4) {
                            val left = faceCoordinates[0]
                            val top = faceCoordinates[1]
                            val right = faceCoordinates[2]
                            val bottom = faceCoordinates[3]

                            // draw rectangle shape to canvas
                            shapeDrawable = ShapeDrawable(RectShape())
                            shapeDrawable.setBounds( left, top, right, bottom)
                            shapeDrawable.paint.setColor(Color.parseColor("#3cd184"))
                            shapeDrawable.paint.setStyle(Paint.Style.STROKE)
                            shapeDrawable.paint.setStrokeWidth(6f)
                            shapeDrawable.draw(canvas)

                            // Flip faceRect bitmap horizontally
                            val cx = bitmap.width / 2f
                            val cy = bitmap.height / 2f
                            val flippedBitmap = bitmap.flip(-1f, 1f, cx, cy)

                            // now bitmap holds the updated pixels
                            // set bitmap as background to ImageView
                            viewBinding.faceRect.background = BitmapDrawable(resources, flippedBitmap)
                        }
                    }
                    faceInfoFile.delete()
                }
                else {
                    viewBinding.faceRect.background = BitmapDrawable(resources, bitmap)
                }
                Handler(Looper.getMainLooper()).postDelayed(this, 50)
                return
            }
        }, 500)

        // Request camera permissions
        if (allPermissionsGranted()) {
            startCamera()
        } else {
            ActivityCompat.requestPermissions(
                this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS)
        }
        cameraExecutor = Executors.newSingleThreadExecutor()
    }


    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<String>, grantResults:
        IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == REQUEST_CODE_PERMISSIONS) {
            if (allPermissionsGranted()) {
                startCamera()
            } else {
                Toast.makeText(this,
                    "Permissions not granted by the user.",
                    Toast.LENGTH_SHORT).show()
                finish()
            }
        }
    }


    private fun allPermissionsGranted() = REQUIRED_PERMISSIONS.all {
        ContextCompat.checkSelfPermission(
            baseContext, it) == PackageManager.PERMISSION_GRANTED
    }


    private fun startCamera() {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(this)

        cameraProviderFuture.addListener({
            // Used to bind the lifecycle of cameras to the lifecycle owner
            val cameraProvider: ProcessCameraProvider = cameraProviderFuture.get()

            // Preview
            val preview = Preview.Builder()
                // CameraX output will be produced to match the aspect ratios requested as closely as the device supports
                // If there is no exact-match resolution supported, the one that fulfills the most conditions is selected.
                // In this case, I use 16:9 ratio resolution
                .setTargetResolution(Size(screenW, screenH))
                .build()
                .also {
                    it.setSurfaceProvider(viewBinding.viewFinder.surfaceProvider)
                }

            val imageAnalyzer = ImageAnalysis.Builder()
                // CameraX output will be produced to match the aspect ratios requested as closely as the device supports
                // If there is no exact-match resolution supported, the one that fulfills the most conditions is selected.
                // In this case, I use 16:9 ratio resolution
                .setTargetResolution(Size(screenW, screenH))
                .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
                .build()
                .also {
                    it.setAnalyzer(cameraExecutor, FaceAnalyzer())
                }

            // Select front camera as a default
            val cameraSelector = CameraSelector.Builder()
                .requireLensFacing(CameraSelector.LENS_FACING_FRONT)
                .build()

            try {
                // Unbind use cases before rebinding
                cameraProvider.unbindAll()

                // Bind use cases to camera
                cameraProvider.bindToLifecycle(this, cameraSelector, preview, imageAnalyzer)

            } catch(exc: Exception) {
                Log.e(TAG, "Use case binding failed", exc)
            }

        }, ContextCompat.getMainExecutor(this))
    }


    override fun onDestroy() {
        super.onDestroy()
        cameraExecutor.shutdown()
    }


    private class FaceAnalyzer : ImageAnalysis.Analyzer {
        private fun ByteBuffer.toByteArray(): ByteArray {
            rewind()    // Rewind the buffer to zero
            val data = ByteArray(remaining())
            get(data)   // Copy the buffer into a byte array
            return data // Return the byte array
        }

        override fun analyze(image: ImageProxy) {
            // Image format is YUV_420_888
            // Grayscale image is extracted from Y-plane, which is index 0
            val buffer = image.planes[0].buffer
            val data = buffer.toByteArray()
            val mat = Mat(image.height, image.width, CV_8UC1)
            mat.put(0, 0, data)

            // ImageProxy is rotated incorrectly leading to wrong Matrix rotation
            // So we rotate Matrix in WYSIWYG way
            // Note: mat.t() is the transpose
            val rotatedMat = Mat()
            when(image.imageInfo.rotationDegrees) {
                90 -> Core.flip(mat.t(), rotatedMat, 1)
                180 -> Core.flip(mat, rotatedMat, -1)
                270 -> Core.flip(mat.t(), rotatedMat, 0)
            }
            mat.release()

            // Get absolute path to "tmpFile" and pass it to JNI function
            val faceInfo = faceDetect("${tmpDir}/haarcascade_frontalface.xml", rotatedMat.nativeObjAddr)
            rotatedMat.release()
            // Return x, y, h, w of detected face bounding box
            if (faceInfo.size == 4) {
                val x = faceInfo[0].toDouble()
                val y = faceInfo[1].toDouble()
                val w = faceInfo[2].toDouble()
                val h = faceInfo[3].toDouble()

                val tmpFaceFile =  File(tmpDir, "faceCoordinates.txt")
                tmpFaceFile.writeText("${x.toInt()},${y.toInt()},${w.toInt()},${h.toInt()}")
            }

            // Close image after finishing processing
            image.close()
        }

        private external fun faceDetect(model: String, matAdd: Long): Array<Int>
    }
}