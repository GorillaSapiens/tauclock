package com.gorillasapiens.sunclock1

//import android.R
//import android.R

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.location.Criteria
import android.location.Location
import android.location.LocationListener
import android.location.LocationManager
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.MotionEvent
import android.view.View
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.wozniakconsulting.sunclock1.R
//import net.iakovlev.timeshape.TimeZoneEngine
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
    var mNeedUpdate : Boolean = true

    var mOtlDown : Boolean = false
    var mOtlChanged : Boolean = false
    var mOtlX : Float = 0.0f
    var mOtlY : Float = 0.0f
    var mOtlLat : Double = 0.0
    var mOtlLon : Double = 0.0

    var mLocationManager : LocationManager? = null
    var mProviderName : String? = null
    var mNeedDisable : Boolean = false

    //var engine: TimeZoneEngine = TimeZoneEngine.initialize()

    // Create the Handler object (on the main thread by default)
    var mHandler = Handler(Looper.getMainLooper())

    companion object {
      init {
         System.loadLibrary("libnova")
      }
    }

    private fun updateDrawing() {
        if (!mDrawableInitialized) {
            return
        }

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
                mProviderName ?: "<null>")
            mSunclockDrawable?.setThing(something)
            mImageView?.invalidate()
        }

        mImageView?.invalidate()
    }

    private val runVeryOften: Runnable = object : Runnable {
        override fun run() {
            if (mHasFocus) {
                val current = LocalDateTime.now();

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

                mDrawableInitialized = true
            }
        }
        mHasFocus = hasFocus;
    }

    private fun _renewLocation() {
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
        if (mProviderName != "manual") {
            try {
                mLocationManager!!.getCurrentLocation(mProviderName ?: "gps",
                    null,
                    ContextCompat.getMainExecutor(this),
                    { location ->
                        if (mProviderName != "manual") {
                            mLastLocation = location
                        };
                    })
            }
            catch (e: Exception) {
                Log.d("EXCEPTION", e.toString())
            }
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
                selectBestProvider()
                startProvider()
            }
        }
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

        selectBestProvider()

        mHandler.post(runVeryOften);
    }

    override fun onStart() {
        super.onStart();
        if (!checkPermissions()) {
            requestPermissions()
        }
    }
    override fun onResume() {
        super.onResume()
        if (checkPermissions()) {
            startProvider()
        }
    }

    override fun onPause() {
        super.onPause()
        if (checkPermissions()) {
            stopProvider()
        }
    }

    private fun otl(view: View, motionEvent: MotionEvent) {
        if (motionEvent.action == MotionEvent.ACTION_DOWN) {
            mOtlX = motionEvent.x
            mOtlY = motionEvent.y
            var width = Math.min(mImageView?.width ?: 1024, mImageView?.height ?: 1024)
            var dcx = motionEvent.x - (mImageView?.width ?: 1024) / 2.0
            var dcy = motionEvent.y - (mImageView?.height ?: 1024) / 2.0

            dcx *= dcx
            dcy *= dcy
            var dc = sqrt(dcx+dcy)

            if (mProviderName == "manual" && dc < width / 2.0) {
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

    class MyLocationListener(mainactivity: MainActivity) : LocationListener {
        var context = mainactivity

        override fun onLocationChanged(location: Location) {
            context.mLastLocation = location
            context.mNeedUpdate = true
        }

        override fun onFlushComplete(requestCode: Int) {
        }

        override fun onProviderDisabled(provider: String) {
            var location = Location(context.mProviderName)
            location.latitude = 91.0
            location.longitude = 361.0
            context.mLastLocation = location
            context.mNeedUpdate = true
        }

        override fun onProviderEnabled(provider: String) {
        }

        override fun onStatusChanged(provider: String?, status: Int, extras: Bundle?) {
        }
    }
    private val mLocationListener = MyLocationListener(this@MainActivity)

    private fun startProvider() {
        mNeedDisable = false;
        if (mProviderName != null && mProviderName != "manual") {
            if (!(mLocationManager?.isProviderEnabled(mProviderName ?: "gps") ?: true)) {
                //val settingsIntent = Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS)
                //startActivity(settingsIntent)
            }

            if (checkPermissions()) {
                mLocationManager?.requestLocationUpdates(
                    mProviderName ?: "gps",
                    10000,          // 10-second interval.
                    10.0f,             // 10 meters.
                    mLocationListener
                );
            }
        }
    }

    private fun stopProvider() {
        if (mProviderName != null && mProviderName != "manual") {
            mLocationManager?.removeUpdates(mLocationListener);
        }
    }

    private fun chooseNewProvider() {
        stopProvider();

        var allProviders = mLocationManager!!.getProviders(true)
        allProviders.add("manual")
        var n = 0
        for (s in allProviders) {
            if (s == mProviderName) {
                break;
            }
            n++;
        }
        n++;
        n %= allProviders.size;
        mProviderName = allProviders[n]

        startProvider()
        mNeedUpdate = true
    }

    private fun selectBestProvider() {
        var allProviders = mLocationManager!!.getAllProviders()
        allProviders.add("manual")
        val criteria = Criteria()
        mProviderName = mLocationManager!!.getBestProvider(criteria,false)
        if (mProviderName == null) {
            mProviderName = "manual"
        }
    }

    external fun do_all(lat:Double, lng:Double, offset:Double, width:Int, provider:String) : IntArray
    external fun do_globe(lat:Double, lng:Double, width:Int) : IntArray
}