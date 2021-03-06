package com.gorillasapiens.sunclock1

//import android.R
//import android.R

import android.Manifest
import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.location.Location
import android.location.LocationManager
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.provider.Settings
import android.view.View
import android.widget.ImageView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.google.android.gms.location.*
import com.wozniakconsulting.sunclock1.R
import java.time.Duration
import java.time.LocalDateTime

class MainActivity : AppCompatActivity() {
    var mDrawableInitialized = false
    var mFusedLocationClient: FusedLocationProviderClient? = null
    val PERMISSION_ID = 44
    var mSunclockDrawable: SunclockDrawable? = null
    var mHasFocus = false;
    var mLastTime = LocalDateTime.now() - Duration.ofDays(1)
    var mLastRequest = mLastTime
    var mLastLocation : Location? = null
    var mLastLastLocation : Location? = null

    // Create the Handler object (on the main thread by default)
    var mHandler = Handler(Looper.getMainLooper())

    companion object {
      init {
         System.loadLibrary("libnova")
      }
    }

    private val runEverySecond: Runnable = object : Runnable {
        override fun run() {
            if (mHasFocus) {

                val current = LocalDateTime.now();

                if ((current - Duration.ofMinutes(20)) > mLastRequest) {
                    requestNewLocationData()
                    mLastRequest = current;
                }

                if (mLastLastLocation != mLastLocation ||
                    (current - Duration.ofMinutes(1)) > mLastTime ||
                    current.minute != mLastTime.minute) {
                    val imageView: ImageView = findViewById<View>(R.id.imageView) as ImageView

                    if (mLastLocation == null) {
                        mLastLocation = mLastLastLocation;
                    }

                    if (mLastLocation != null) {
                        var something = do_all(mLastLocation!!.latitude, mLastLocation!!.longitude, 0.0,
                            Math.min(imageView.width, imageView.height));
                        mSunclockDrawable?.setThing(something);
                    }
                    else {
                        var something = do_all(-181.0,-181.0, 0.0,
                            Math.min(imageView.width, imageView.height));
                        mSunclockDrawable?.setThing(something);
                    }

                    imageView.invalidate()

                    mLastTime = current
                    mLastLastLocation = mLastLocation
                }
            }

            mHandler.postDelayed(this, 1000)
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            if (!mDrawableInitialized) {
                //val layObj: LinearLayout = findViewById<View>(R.id.parentLay) as LinearLayout
                val imageView: ImageView = findViewById<View>(R.id.imageView) as ImageView
                mSunclockDrawable = SunclockDrawable(imageView.width, imageView.height)
                imageView.setImageDrawable(mSunclockDrawable)
                imageView.invalidate()

                getLastLocation()

                mDrawableInitialized = true
            }
        }
        mHasFocus = hasFocus;
    }

    @SuppressLint("MissingPermission")
    private fun getLastLocation() {
        // check if permissions are given
        if (checkPermissions()) {
            // check if location is enabled
            if (isLocationEnabled()) {
                // getting last
                // location from
                // FusedLocationClient
                // object
                mFusedLocationClient!!.lastLocation.addOnCompleteListener { task ->
                    val location = task.result
                    if (location == null) {
                        requestNewLocationData()
                    }
                }
            } else {
                Toast.makeText(this, "Please turn on" + " your location...", Toast.LENGTH_LONG)
                    .show()
                val intent = Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS)
                startActivity(intent)
            }
        } else {
            // if permissions aren't available,
            // request for permissions
            requestPermissions()
        }
    }
    @SuppressLint("MissingPermission")
    private fun requestNewLocationData() {

        // Initializing LocationRequest
        // object with appropriate methods
        val mLocationRequest = LocationRequest()
        mLocationRequest.setPriority(LocationRequest.PRIORITY_HIGH_ACCURACY)
        mLocationRequest.setInterval(5)
        mLocationRequest.setFastestInterval(0)
        mLocationRequest.setNumUpdates(1)

        // setting LocationRequest
        // on FusedLocationClient
        mFusedLocationClient = LocationServices.getFusedLocationProviderClient(this)
        mFusedLocationClient?.requestLocationUpdates(
            mLocationRequest,
            mLocationCallback,
            Looper.myLooper()
        )
    }

    private val mLocationCallback: LocationCallback = object : LocationCallback() {
        override fun onLocationResult(locationResult: LocationResult) {
            mLastLocation = locationResult.lastLocation
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
                getLastLocation()
            }
        }
    }

    override fun onResume() {
        super.onResume()
        if (checkPermissions()) {
            getLastLocation()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val manager = this.packageManager
        val info = manager.getPackageInfo(this.packageName, PackageManager.GET_ACTIVITIES)

         actionBar?.setTitle("?????? clock v" + info.versionName);
         supportActionBar?.setTitle("?????? clock v" + info.versionName);
        //actionBar?.hide();
        //supportActionBar?.hide();

        mFusedLocationClient = LocationServices.getFusedLocationProviderClient(this);
        getLastLocation();

        mHandler.post(runEverySecond);
    }

    external fun do_all(lat:Double, lng:Double, offset:Double, width:Int) : IntArray
}