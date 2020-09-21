package darko.watermark;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.FileProvider;

import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.DisplayMetrics;
import android.widget.ImageView;
import android.view.View;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.util.Log;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.opencv.core.Mat;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    static {
        System.loadLibrary("native-lib");
//        System.load("opencv-java3");
    }

    static final int REQUEST_IMAGE_CAPTURE = 1;
    static final String TAG = "Watermark";

    private MainActivity context;
    private ImageView imgView;

    private View openCamera;
    private View addWatermarkView1;
    private View addWatermarkView2;
    private View addWatermarkView3;
    private View addWatermarkView4;

    private int imageWidth, imageHeight;

    private String currentPhotoPath;
    private String currentAlgo;

    private Bitmap srcBitmap, dstBitmap;
    private Bitmap watermarkedBitmap;
    private Bitmap watermark;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
    }

    private void initView() {
        context = this;
        DisplayMetrics metrics = getResources().getDisplayMetrics();
        imageWidth = metrics.widthPixels;
        imageHeight = metrics.heightPixels;

        imgView = (ImageView) findViewById(R.id.img);
        openCamera = findViewById(R.id.camera);
        openCamera.setOnClickListener(this);
        addWatermarkView1 = findViewById(R.id.algo1);
        addWatermarkView1.setOnClickListener(this);
        addWatermarkView2 = findViewById(R.id.algo2);
        addWatermarkView2.setOnClickListener(this);
        addWatermarkView3 = findViewById(R.id.algo3);
        addWatermarkView3.setOnClickListener(this);
        addWatermarkView4 = findViewById(R.id.algo4);
        addWatermarkView4.setOnClickListener(this);

        watermark = BitmapFactory.decodeResource(
                this.getApplication().getResources(), R.drawable.dfm);
//                this.getApplication().getResources(), R.mipmap.DFM);
    }

    @Override
    protected void onResume() {
//        super.onResume();
//        // load OpenCV engine and init OpenCV library
//        OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_4_0, getApplicationContext(), mLoaderCallback);
//        Log.i("Watermark", "onResume success loaded OpenCV...");
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d("OpenCV", "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_4_0, this, mLoaderCallback);
        } else {
            Log.d("OpenCV", "OpenCV library found inside package. Using it!");
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case BaseLoaderCallback.SUCCESS:
                    Log.i(TAG, "Successfully Loaded!");
                    break;
                default:
                    super.onManagerConnected(status);
                    Log.i(TAG, "Loading Failed!");
                    break;
            }
        }
    };

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.camera:
                takePhotoClicked();
                break;
            case R.id.algo1:
                currentAlgo = "algo1";
                addWatermarkClicked();
                break;
            case R.id.algo2:
                currentAlgo = "algo2";
                addWatermarkClicked();
                break;
            case R.id.algo3:
                currentAlgo = "algo3";
                addWatermarkClicked();
                break;
            case R.id.algo4:
                currentAlgo = "algo4";
                addWatermarkClicked();

        }
    }

    protected void takePhotoClicked() {
        System.out.println("take photo");

        // open system camera
        dispatchTakePictureIntent();
    }

//    private void dispatchTakePictureIntent() {
//        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
//        if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
//            startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
//        }
//    }

    private void dispatchTakePictureIntent() {
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        // Ensure that there's a camera activity to handle the intent
        if (takePictureIntent.resolveActivity((getPackageManager())) != null) {
            // create the File where the photo should go
            File photoFile = null;
            try {
                photoFile = createImageFile();
            } catch (IOException ex) {
                System.out.println("Error occurred while creating the File");
            }
            // Continue only if the File was successfully created
            if (photoFile != null) {
                Uri photoURI = FileProvider.getUriForFile(this,
                        "com.example.android.fileprovider",
                        photoFile);
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, photoURI);
                startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
            }
        }
    }

    // Get the captured picture thumbnail
    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_IMAGE_CAPTURE && resultCode == RESULT_OK) {
//            Bundle extras = data.getExtras();
//            Bitmap imageBitmap = (Bitmap) extras.get("data");
//            imgView.setImageBitmap(imageBitmap);
//            galleryAddPic();
            setPic();
        }
    }

    private File createImageFile() throws IOException {
        // Create an image file name
        String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        String imageFileName = "JPEG_" + timestamp + "_";
        File storageDir = getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        File image = File.createTempFile(imageFileName, ".jpg", storageDir);
        // save a file: path for use with ACTION_VIEW intents
        currentPhotoPath = image.getAbsolutePath();

        return image;
    }

    private void galleryAddPic() {
        Intent mediaScanIntent = new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE);
        File f = new File(currentPhotoPath);
        Uri contentUri = Uri.fromFile(f);
        mediaScanIntent.setData(contentUri);
        this.sendBroadcast(mediaScanIntent);
    }

    private void setPic() {
        // Get the dimensions of the View
        int targetW = imgView.getWidth();
        int targetH = imgView.getHeight();

        // Get the dimensions of the bitmap
        BitmapFactory.Options bmOptions = new BitmapFactory.Options();
        bmOptions.inJustDecodeBounds = true;

        BitmapFactory.decodeFile(currentPhotoPath, bmOptions);

        int photoW = bmOptions.outWidth;
        int photoH = bmOptions.outHeight;

        // Determine how much to scale down the image
        int scaleFactor = Math.max(1, Math.min(photoW/targetW, photoH/targetH));

        // Decode the image file into a Bitmap sized to fill the View
        bmOptions.inJustDecodeBounds = false;
        bmOptions.inSampleSize = scaleFactor;
        bmOptions.inPurgeable = true;

        Bitmap bitmap = BitmapFactory.decodeFile(currentPhotoPath, bmOptions);
        srcBitmap = bitmap;
        imgView.setImageBitmap(bitmap);
    }

    protected void addWatermarkClicked() {
        System.out.println("adding watermark...");

        // take the captured image and add watermark on it.
        // JNI to use C++ watermark algorithm
        addWatermarkFromJNI();
        imgView.setImageBitmap(dstBitmap);
    }

    protected void addWatermarkFromJNI() {

        String algo = currentAlgo;

        // image to cv::Mat
        Mat mat_img_src = new Mat();
        Mat mat_img_dst = new Mat();
        Mat mat_mark = new Mat();
//        srcBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.orig);
        watermarkedBitmap = Bitmap.createBitmap(srcBitmap.getWidth(),
                                                srcBitmap.getHeight(),
                                                Bitmap.Config.RGB_565);
        Utils.bitmapToMat(srcBitmap, mat_img_src);
        Utils.bitmapToMat(watermark, mat_mark);
//        Imgproc.cvtColor(mat_img_src, mat_img_dst, Imgproc.COLOR_RGB2GRAY);
//        addWatermark(mat_img_src.getNativeObjAddr(), mat_mark.getNativeObjAddr(), algo);
//        addWatermark(mat_img_src, mat_mark, mat_img_dst, algo);
        addWatermark(srcBitmap, watermark, watermarkedBitmap, algo);
//        Utils.matToBitmap(mat_img_dst, watermarkedBitmap);
        dstBitmap = watermarkedBitmap;
        Log.i(TAG, "watermark added!");
    }

    public native void addWatermark(Bitmap jobject_img_src, Bitmap jobject_watermark, Bitmap jobject_img_dst,
                                    String jalgo);

    public static void mat2Bitmap(Mat mat, Bitmap bitmap) {
        nmat2Bitmap(mat.nativeObj, bitmap);
    }

    private static native void nmat2Bitmap(long mnativePtr, Bitmap bitmap);

    public static void bitmap2Mat(Bitmap bitmap, Mat mat) {
        nbitmap2Mat(bitmap, mat.nativeObj);
    }

    private static native void nbitmap2Mat(Bitmap bitmap, long nativePtr);

}