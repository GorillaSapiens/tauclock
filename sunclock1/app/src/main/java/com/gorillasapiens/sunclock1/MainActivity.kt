package com.gorillasapiens.sunclock1

//import android.R
//import android.R

import android.Manifest
import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.location.Criteria
import android.location.Location
import android.location.LocationManager
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.provider.Settings
import android.view.MotionEvent
import android.view.View
import android.widget.ImageView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.core.content.ContextCompat.getMainExecutor
import com.google.android.gms.location.LocationCallback
import com.google.android.gms.location.LocationRequest
import com.google.android.gms.location.LocationResult
import com.wozniakconsulting.sunclock1.R
import java.time.Duration
import java.time.LocalDateTime
import kotlin.math.sqrt

class MainActivity : AppCompatActivity() {
    var mDrawableInitialized = false
    //var mFusedLocationClient: FusedLocationProviderClient? = null
    val PERMISSION_ID = 44
    var mSunclockDrawable: SunclockDrawable? = null
    var mHasFocus = false;
    var mLastTime = LocalDateTime.now() - Duration.ofDays(1)
    var mLastRequest = mLastTime
    var mLastLocation : Location? = null
    var mLastLastLocation : Location? = null
    var mImageView : ImageView? = null

    var mOtlDown : Boolean = false
    var mOtlChanged : Boolean = false
    var mOtlX : Float = 0.0f
    var mOtlY : Float = 0.0f
    var mOtlLat : Double = 0.0
    var mOtlLon : Double = 0.0

    var mLocationManager : LocationManager? = null
    var mProvider : String? = null

    // Create the Handler object (on the main thread by default)
    var mHandler = Handler(Looper.getMainLooper())

    companion object {
      init {
         System.loadLibrary("libnova")
      }
    }

    private fun updateDrawing() {
        if (mOtlDown) {
            if (mOtlChanged || mLastLocation != mLastLastLocation) {
                var something = do_globe(
                    mLastLocation?.latitude ?: -181.0,
                    mLastLocation?.longitude ?: -181.0,
                    Math.min(mImageView?.width ?: 1024, mImageView?.height ?: 1024));
                mSunclockDrawable?.setThing(something)
                mImageView?.invalidate()

                mLastLastLocation = mLastLocation;
                mOtlChanged = false;
            }
        }
        else {
            var something = do_all(
                mLastLocation?.latitude ?: -181.0,
                mLastLocation?.longitude ?: -181.0,
                0.0,
                Math.min(mImageView?.width ?: 1024, mImageView?.height ?: 1024),
                mProvider ?: "<null>")
            mSunclockDrawable?.setThing(something)
            mImageView?.invalidate()
        }

        mImageView?.invalidate()
    }

    private val runVeryOften: Runnable = object : Runnable {
        override fun run() {
            if (mHasFocus) {
                val current = LocalDateTime.now();

                if ((current - Duration.ofMinutes(20)) > mLastRequest) {
                    renewLocation()
                    mLastRequest = current;
                }

                if (mLastLastLocation != mLastLocation ||
                    (current - Duration.ofMinutes(1)) > mLastTime ||
                    current.minute != mLastTime.minute) {

                    if (mLastLocation == null) {
                        mLastLocation = mLastLastLocation;
                    }

                    updateDrawing()

                    mLastTime = current
                    mLastLastLocation = mLastLocation
                }

                if (mLastRequest + Duration.ofMinutes(5) < LocalDateTime.now()) {
                    renewLocation()
                }
            }

            mHandler.postDelayed(this, 50)
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            if (!mDrawableInitialized) {
                mSunclockDrawable = SunclockDrawable(
                    mImageView?.width ?: 1024,
                    mImageView?.height ?: 1024)

                mImageView?.setImageDrawable(mSunclockDrawable)
                mImageView?.invalidate()

                renewLocation()

                mDrawableInitialized = true
            }
        }
        mHasFocus = hasFocus;
    }

    private fun renewLocation() {
        mLastRequest = LocalDateTime.now()
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.ACCESS_FINE_LOCATION
            ) != PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.ACCESS_COARSE_LOCATION
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            // TODO: Consider calling
            //    ActivityCompat#requestPermissions
            // here to request the missing permissions, and then overriding
            //   public void onRequestPermissionsResult(int requestCode, String[] permissions,
            //                                          int[] grantResults)
            // to handle the case where the user grants the permission. See the documentation
            // for ActivityCompat#requestPermissions for more details.
            requestPermissions()
            return
        }
        if (mProvider != "manual") {
            mLocationManager!!.getCurrentLocation(mProvider ?: "gps",
                null,
                ContextCompat.getMainExecutor(this),
                { location ->
                    if (mProvider != "manual") {
                        mLastLocation = location
                    };
                })
        }
    }

    // method to check for permissions
    private fun checkPermissions(): Boolean {
        return ActivityCompat.checkSelfPermission(
            this,
            Manifest.permission.ACCESS_COARSE_LOCATION
        ) == PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(
            this,
            Manifest.permission.ACCESS_FINE_LOCATION
        ) == PackageManager.PERMISSION_GRANTED

        // If we want background location
        // on Android 10.0 and higher,
        // use:
        // ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_BACKGROUND_LOCATION) == PackageManager.PERMISSION_GRANTED
    }

    // method to request for permissions
    private fun requestPermissions() {
        ActivityCompat.requestPermissions(
            this, arrayOf(
                Manifest.permission.ACCESS_COARSE_LOCATION,
                Manifest.permission.ACCESS_FINE_LOCATION
            ), PERMISSION_ID
        )
    }

    // method to check
    // if location is enabled
    private fun isLocationEnabled(): Boolean {
        val locationManager = getSystemService(Context.LOCATION_SERVICE) as LocationManager
        return locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER) || locationManager.isProviderEnabled(
            LocationManager.NETWORK_PROVIDER
        )
    }

    // If everything is alright then
    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String?>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == PERMISSION_ID) {
            if (grantResults.size > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                chooseBestProvider()
                renewLocation()
            }
        }
    }

    override fun onResume() {
        super.onResume()
        if (checkPermissions()) {
            renewLocation()
        }
    }

    fun chooseNewProvider() {
        var allProviders = mLocationManager!!.getAllProviders()
        allProviders.add("manual")
        var n = 0
        for (s in allProviders) {
            if (s == mProvider) {
                break;
            }
            n++;
        }
        n++;
        n %= allProviders.size;
        mProvider = allProviders[n]

        var something = arrayOf<Int>().toIntArray()
        mSunclockDrawable?.setThing(something)
        mImageView?.invalidate()

        renewLocation()
    }

    fun otl(view: View, motionEvent: MotionEvent) {
        if (motionEvent.action == MotionEvent.ACTION_DOWN) {

            if (mLastLocation == null) {
                mLastLocation = Location("manual")
                mLastLocation?.latitude = 0.0
                mLastLocation?.longitude = 0.0
            }

            mOtlX = motionEvent.x
            mOtlY = motionEvent.y
            var width = Math.min(mImageView?.width ?: 1024, mImageView?.height ?: 1024)
            var dcx = motionEvent.x - (mImageView?.width ?: 1024) / 2.0
            var dcy = motionEvent.y - (mImageView?.height ?: 1024) / 2.0

            dcx *= dcx
            dcy *= dcy
            var dc = sqrt(dcx+dcy)

            if (mProvider == "manual" && dc < width / 2.0) {
                mOtlLat = mLastLocation?.latitude ?: 0.0
                mOtlLon = mLastLocation?.longitude ?: 0.0
                mOtlDown = true
                mOtlChanged = true
                updateDrawing()
            }
            else {
                dcx = motionEvent.x - 0.0;
                dcx *= dcx;
                dcy = motionEvent.y - ((mImageView?.height ?: 1024) / 2.0 - width / 2.0)
                dcy *= dcy
                dc = sqrt(dcx+dcy)
                if (dc < width / 10) {
                    chooseNewProvider()
                    updateDrawing()
                }
            }
        }
        else if (motionEvent.action == MotionEvent.ACTION_UP){
            mOtlDown = false
            updateDrawing()
        }
        else if (mOtlDown && motionEvent.action ==MotionEvent.ACTION_MOVE) {
            var proposedLocation = Location("manual");
            var deltax = motionEvent.x - mOtlX;
            var deltay = motionEvent.y - mOtlY;
            var width = Math.min(mImageView?.width ?: 1024, mImageView?.height ?: 1024)

            proposedLocation.latitude = mOtlLat + 90.0 * deltay / (width)
            proposedLocation.longitude = mOtlLon - 90.0 * deltax / (width)

            if (proposedLocation.latitude > 90.0) {
                proposedLocation.latitude = 90.0
            }
            if (proposedLocation.latitude < -90.0) {
                proposedLocation.latitude = -90.0
            }
            while (proposedLocation.longitude < -180.0) {
                proposedLocation.longitude += 360.0;
            }
            while (proposedLocation.longitude > 180.0) {
                proposedLocation.longitude -= 360.0;
            }
            mLastLocation = proposedLocation;
        }
    }

    private fun chooseBestProvider() {
        var allProviders = mLocationManager!!.getAllProviders()
        allProviders.add("manual")
        val criteria = Criteria()
        mProvider = mLocationManager!!.getBestProvider(criteria,false)
        if (mProvider == null) {
            mProvider = "manual"
        }

        var something = arrayOf<Int>().toIntArray()
        mSunclockDrawable?.setThing(something)
        mImageView?.invalidate()

        renewLocation()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val manager = this.packageManager
        val info = manager.getPackageInfo(this.packageName, PackageManager.GET_ACTIVITIES)

         actionBar?.setTitle("ταμ clock v" + info.versionName);
         supportActionBar?.setTitle("ταμ clock v" + info.versionName);

         //actionBar?.hide();
         //supportActionBar?.hide();

        mImageView = findViewById<View>(R.id.imageView) as ImageView
        mImageView?.setOnTouchListener(View.OnTouchListener { view, motionEvent ->
            this.otl(view, motionEvent)
            return@OnTouchListener true
        })

        mLocationManager = this.getSystemService(android.content.Context.LOCATION_SERVICE) as LocationManager?

        chooseBestProvider()

        mHandler.post(runVeryOften);
    }

    external fun do_all(lat:Double, lng:Double, offset:Double, width:Int, provider:String) : IntArray
    external fun do_globe(lat:Double, lng:Double, width:Int) : IntArray
}