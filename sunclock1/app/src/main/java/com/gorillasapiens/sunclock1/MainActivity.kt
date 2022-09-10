package com.gorillasapiens.sunclock1

//import android.R
//import android.R

//import net.iakovlev.timeshape.TimeZoneEngine
import android.Manifest
import android.annotation.SuppressLint
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
import com.wozniakconsulting.sunclock1.R
import java.time.Duration
import java.time.LocalDateTime
import kotlin.math.*


class MainActivity : AppCompatActivity() {
    private var mDrawableInitialized = false
    private val mPermissionID = 44
    private var mSunclockDrawable: SunclockDrawable? = null
    var mHasFocus = false
    var mLastTime = (LocalDateTime.now() - Duration.ofDays(1))!!
    var mLastLocation : Location? = null
    var mLastLastLocation : Location? = null
    private var mImageView : ImageView? = null
    var mNeedUpdate : Boolean = true

    private var mOtlDown : Boolean = false
    private var mOtlChanged : Boolean = false
    private var mOtlX : Float = 0.0f
    private var mOtlY : Float = 0.0f
    private var mOtlSpinBase: Float = 0.0f
    private var mOtlLat : Double = 0.0
    private var mOtlLon : Double = 0.0
    private var mOtlSpin : Double = 0.0

    private var mLocationManager : LocationManager? = null
    var mProviderName : String? = null
    private var mNeedDisable : Boolean = false

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
                val something = doGlobe(
                    mLastLocation?.latitude ?: -181.0,
                    mLastLocation?.longitude ?: -181.0,
                    mLastLocation?.altitude ?: 0.0,
                    min(mImageView?.width ?: 1024, mImageView?.height ?: 1024)
                )
                mSunclockDrawable?.setThing(something)
                mImageView?.invalidate()

                mLastLastLocation = mLastLocation
                mOtlChanged = false
            }
        }
        else {
            val something = doAll(
                mLastLocation?.latitude ?: -181.0,
                mLastLocation?.longitude ?: -181.0,
                0.0,
                min(mImageView?.width ?: 1024, mImageView?.height ?: 1024),
                mProviderName ?: "<null>")
            mSunclockDrawable?.setThing(something)
            mImageView?.invalidate()
        }

        mImageView?.invalidate()
    }

    private val runVeryOften: Runnable = object : Runnable {
        override fun run() {
            if (mHasFocus) {
                val current = LocalDateTime.now()

                if (mLastLastLocation != mLastLocation ||
                    (current - Duration.ofMinutes(1)) > mLastTime ||
                    current.minute != mLastTime.minute) {

                    if (mLastLocation == null) {
                        mLastLocation = mLastLastLocation
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
        mHasFocus = hasFocus
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
            ), mPermissionID
        )
    }

    // If everything is alright then
    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String?>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == mPermissionID) {
            if (grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                selectBestProvider()
                startProvider()
            }
        }
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val manager = this.packageManager
        val info = manager.getPackageInfo(this.packageName, PackageManager.GET_ACTIVITIES)

        actionBar?.title = "ταμ clock v" + info.versionName
        supportActionBar?.title = "ταμ clock v" + info.versionName

        //actionBar?.hide();
        //supportActionBar?.hide();

        mImageView = findViewById<View>(R.id.imageView) as ImageView
        mImageView?.setOnTouchListener(View.OnTouchListener { view, motionEvent ->
            this.otl(motionEvent)
            view.performClick()
            return@OnTouchListener true
        })

        mLocationManager = this.getSystemService(LOCATION_SERVICE) as LocationManager?

        selectBestProvider()

        mHandler.post(runVeryOften)
    }

    override fun onStart() {
        super.onStart()
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

    private fun isCloseToCenter(motionEvent: MotionEvent) : Boolean {
        val width = min(mImageView?.width ?: 1024, mImageView?.height ?: 1024)

        var dcx = motionEvent.x - (mImageView?.width ?: 1024) / 2.0
        var dcy = motionEvent.y - (mImageView?.height ?: 1024) / 2.0

        dcx *= dcx
        dcy *= dcy
        val dc = sqrt(dcx+dcy)

        return (dc < (width / 2))
    }

    private fun isCloseToProvider(motionEvent: MotionEvent) : Boolean {
        val width = min(mImageView?.width ?: 1024, mImageView?.height ?: 1024)

        var dcx = motionEvent.x
        var dcy = motionEvent.y - ((mImageView?.height ?: 1024) / 2.0 - width / 2.0)

        dcx *= dcx
        dcy *= dcy
        val dc = sqrt(dcx+dcy)

        return (dc < (width / 5))
    }

    private fun isCloseToTimeZone(motionEvent: MotionEvent) : Boolean {
        val width = min(mImageView?.width ?: 1024, mImageView?.height ?: 1024)

        var dcx = motionEvent.x
        var dcy = motionEvent.y - ((mImageView?.height ?: 1024) / 2.0 + width / 2.0)

        dcx *= dcx
        dcy *= dcy
        val dc = sqrt(dcx+dcy)

        return (dc < (width / 5))
    }

    private fun otl(motionEvent: MotionEvent) {
        val action = motionEvent.getAction() and MotionEvent.ACTION_MASK

        if (action == MotionEvent.ACTION_DOWN) {
            if (isCloseToProvider(motionEvent)){
                chooseNewProvider()
                updateDrawing()
                return
            }
            else if (isCloseToTimeZone(motionEvent)){
                chooseNewProvider()
                updateDrawing()
                return
            }
        }
        if (mProviderName == "manual") {
            if (action == MotionEvent.ACTION_DOWN) {
                if (isCloseToCenter(motionEvent)) {
                    mOtlX = motionEvent.x
                    mOtlY = motionEvent.y

                    mOtlLat = mLastLocation?.latitude ?: 0.0
                    mOtlLon = mLastLocation?.longitude ?: 0.0
                    if (mLastLocation?.provider == "manual") {
                        mOtlSpin = mLastLocation!!.altitude
                    }
                    else {
                        mOtlSpin = 0.0
                    }
                    mOtlDown = true
                    mOtlChanged = true
                    updateDrawing()
                    return
                }
            }

            if (!mOtlDown) {
                return
            }

            if (action == MotionEvent.ACTION_POINTER_DOWN) {
                val x0 = motionEvent.getX(0)
                val y0 = motionEvent.getY(0)
                val x1 = motionEvent.getX(1)
                val y1 = motionEvent.getY(1)
                mOtlSpinBase = atan2(y1 - y0, x1 - x0)

                Log.d("base", x0.toString() + " " +
                        y0.toString() + " " +
                        x1.toString() + " " +
                        y1.toString() + " " +
                        mOtlSpinBase.toString())
            }
            else if (action == MotionEvent.ACTION_UP){
                mOtlDown = false
                updateDrawing()
            }
            else if (action == MotionEvent.ACTION_MOVE) {
                val proposedLocation = Location("manual")

                if (motionEvent.pointerCount > 1) {
                    val x0 = motionEvent.getX(0)
                    val y0 = motionEvent.getY(0)
                    val x1 = motionEvent.getX(1)
                    val y1 = motionEvent.getY(1)
                    val spin = atan2(y1 - y0, x1 - x0)

                    proposedLocation.latitude = mOtlLat
                    proposedLocation.longitude = mOtlLon
                    proposedLocation.altitude = mOtlSpin + (spin - mOtlSpinBase) * 180.0 / PI

                    Log.d("spin", x0.toString() + " " +
                            y0.toString() + " " +
                            x1.toString() + " " +
                            y1.toString() + " " +
                            spin.toString())
                }
                else {
                    val width = min(mImageView?.width ?: 1024,mImageView?.height ?: 1024)
                    val deltax = motionEvent.x - mOtlX
                    val deltay = motionEvent.y - mOtlY

                    proposedLocation.latitude = mOtlLat +  90.0 * (deltay * cos(mOtlSpin * PI / 180.0) - deltax * sin(mOtlSpin * PI / 180.0)) / width
                    proposedLocation.longitude = mOtlLon + 90.0 * (deltay * sin(mOtlSpin * PI / 180.0) - deltax * cos(mOtlSpin * PI / 180.0)) / (width)
                    proposedLocation.altitude = mOtlSpin

                    if (proposedLocation.latitude > 90.0) {
                        proposedLocation.latitude = 90.0 - (proposedLocation.latitude - 90.0)
                        proposedLocation.longitude += 180.0
                        proposedLocation.altitude += 180.0
                    }
                    if (proposedLocation.latitude < -90.0) {
                        proposedLocation.latitude = -90.0 - (proposedLocation.latitude + 90.0)
                        proposedLocation.longitude += 180.0
                        proposedLocation.altitude += 180.0
                    }
                    while (proposedLocation.longitude < -180.0) {
                        proposedLocation.longitude += 360.0
                    }
                    while (proposedLocation.longitude > 180.0) {
                        proposedLocation.longitude -= 360.0
                    }
                    while (proposedLocation.altitude < -180.0) {
                        proposedLocation.altitude += 360.0
                    }
                    while (proposedLocation.altitude > 180.0) {
                        proposedLocation.altitude -= 360.0
                    }
                }
                mLastLocation = proposedLocation
                updateDrawing()
            }
        }
    }

    class MyLocationListener(mainactivity: MainActivity) : LocationListener {
        private var context = mainactivity

        override fun onLocationChanged(location: Location) {
            context.mLastLocation = location
            context.mNeedUpdate = true
        }

        override fun onFlushComplete(requestCode: Int) {
        }

        override fun onProviderDisabled(provider: String) {
            val location = Location(context.mProviderName)
            location.latitude = 91.0
            location.longitude = 361.0
            context.mLastLocation = location
            context.mNeedUpdate = true
        }

        override fun onProviderEnabled(provider: String) {
        }

        @Deprecated("Deprecated in Java")
        override fun onStatusChanged(provider: String?, status: Int, extras: Bundle?) {
        }
    }
    private val mLocationListener = MyLocationListener(this@MainActivity)

    private fun startProvider() {
        mNeedDisable = false
        if (mProviderName != null && mProviderName != "manual") {

            /*
            if (!(mLocationManager?.isProviderEnabled(mProviderName ?: "gps") ?: true)) {
                //val settingsIntent = Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS)
                //startActivity(settingsIntent)
                // TODO FIX
            }
            */

            if (checkPermissions()) {
                try {
                    mLocationManager?.requestLocationUpdates(
                        mProviderName ?: "gps",
                        10000,          // 10-second interval.
                        10.0f,             // 10 meters.
                        mLocationListener
                    )
                }
                catch(e : SecurityException) {
                    requestPermissions()
                }
            }
        }
    }

    private fun stopProvider() {
        if (mProviderName != null && mProviderName != "manual") {
            mLocationManager?.removeUpdates(mLocationListener)
        }
    }

    private fun chooseNewProvider() {
        stopProvider()

        val allProviders = mLocationManager!!.getProviders(true)
        allProviders.add("manual")
        var n = 0
        for (s in allProviders) {
            if (s == mProviderName) {
                break
            }
            n++
        }
        n++
        n %= allProviders.size
        mProviderName = allProviders[n]

        startProvider()
        mNeedUpdate = true
    }

    private fun selectBestProvider() {
        val allProviders = mLocationManager!!.allProviders
        allProviders.add("manual")
        val criteria = Criteria()
        mProviderName = mLocationManager!!.getBestProvider(criteria,false)
        if (mProviderName == null) {
            mProviderName = "manual"
        }
    }

    private external fun doAll(lat:Double, lon:Double, offset:Double, width:Int, provider:String) : IntArray
    private external fun doGlobe(lat:Double, lon:Double, spin:Double, width:Int) : IntArray
}