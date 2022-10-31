package com.gorillasapiens.sunclock1

//import android.R

import android.Manifest
import android.annotation.SuppressLint
import android.app.AlarmManager
import android.app.AlarmManager.AlarmClockInfo
import android.app.PendingIntent
import android.content.DialogInterface
import android.content.Intent
import android.content.SharedPreferences
import android.content.pm.PackageManager
import android.location.Criteria
import android.location.Location
import android.location.LocationListener
import android.location.LocationManager
import android.media.Ringtone
import android.media.RingtoneManager
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.MotionEvent
import android.view.View
import android.widget.*
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.preference.PreferenceManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import net.iakovlev.timeshape.TimeZoneEngine
import java.time.Duration
import java.time.LocalDateTime
import java.util.*
import java.util.regex.Matcher
import java.util.regex.Pattern
import kotlin.math.*


class MainActivity : AppCompatActivity() {
    private var mSeenAlarmTime_clock: Long = 0
    private var mDrawableInitialized = false
    private val mPermissionID = 44
    private var mSunClockDrawable: SunclockDrawable? = null
    private var mHasFocus = false
    private var mLastTime = (LocalDateTime.now() - Duration.ofDays(1))!!
    private var mLastLocation : Location? = null
    private var mLastLastLocation : Location? = null
    private var mImageView : ImageView? = null
    private var mNeedUpdate : Boolean = true

    private var mOtlDown : Boolean = false
    private var mOtlChanged : Boolean = false
    private var mOtlX : Float = 0.0f
    private var mOtlY : Float = 0.0f
    private var mOtlSpinBase: Float = 0.0f
    private var mOtlLat : Double = 0.0
    private var mOtlLon : Double = 0.0
    private var mOtlSpin : Double = 0.0

    private var mTimeZoneEngine: TimeZoneEngine? = null
    private var mEngineDone = false

    private var mLocationManager : LocationManager? = null
    private var mProviderName: String = "best"
    private var mRealProviderName : String = "gps"

    private var mTimeZoneProvider : String = "system"
    private var mManualTimeZone : String = "Etc/GMT"

    private var mOffset: String = "none"
    private var mManualOffset : String = "0"

    // Create the Handler object (on the main thread by default)
    private var mHandler = Handler(Looper.getMainLooper())

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
            if (mNeedUpdate || mOtlChanged || mLastLocation != mLastLastLocation) {
                if (mRealProviderName != "manual") {
                    mLastLocation?.altitude = 0.0
                }
                var tzname = "(initializing...)"
                try {
                    val zoneId = mTimeZoneEngine!!.query(
                        mLastLocation?.latitude ?: 0.0,
                        mLastLocation?.longitude ?: 0.0
                    )
                    tzname = zoneId?.get()?.toString() ?: ""
                }
                catch (e: Exception) {
                    // ignore it
                }
                val something = doGlobe(
                    mLastLocation?.latitude ?: -181.0,
                    mLastLocation?.longitude ?: -181.0,
                    mLastLocation?.altitude ?: 0.0,
                    min(mImageView?.width ?: 1024, mImageView?.height ?: 1024),
                    tzname
                )
                mSunClockDrawable?.setThing(something)
                mImageView?.invalidate()

                mLastLastLocation = mLastLocation
                mOtlChanged = false
            }
        }
        else {
            var displayProvider = mProviderName
            if (displayProvider == "best") {
                displayProvider += " ("
                displayProvider += mRealProviderName
                displayProvider += ")"
            }
            if (mRealProviderName != "manual" && mLocationManager?.isProviderEnabled(mRealProviderName) == false) {
                displayProvider += " [DISABLED!]"
            }
            var tzname : String = TimeZone.getDefault().toZoneId().toString()
            if (mTimeZoneProvider == "location") {
                tzname = try {
                    val zoneId = mTimeZoneEngine!!.query(
                        mLastLocation?.latitude ?: 0.0,
                        mLastLocation?.longitude ?: 0.0
                    )
                    zoneId?.get()?.toString() ?: ""
                } catch (e: Exception) {
                    // do nothing
                    "(initializing...)"
                }
            }
            else if (mTimeZoneProvider == "manual") {
                tzname = mManualTimeZone
            }
            var offset = 0.0
            if (mOffset == "manual") {
                try {
                    offset = mManualOffset.toDouble()
                }
                catch (e: Exception) {
                    // do nothing
                }
            }
            try {
                val something = doAll(
                    mLastLocation?.latitude ?: -181.0,
                    mLastLocation?.longitude ?: -181.0,
                    offset,
                    min(mImageView?.width ?: 1024, mImageView?.height ?: 1024),
                    displayProvider, mTimeZoneProvider, tzname
                )
                mSunClockDrawable?.setThing(something)
                mImageView?.invalidate()
            }
            catch (e: Exception) {
                Log.d("debug", e.toString())
            }
        }

        mImageView?.invalidate()
    }

    private val runVeryOften: Runnable = object : Runnable {
        override fun run() {
            if (mHasFocus) {
                val current = LocalDateTime.now()

                if (mNeedUpdate ||
                    mLastLastLocation != mLastLocation ||
                    (current - Duration.ofMinutes(1)) > mLastTime ||
                    current.minute != mLastTime.minute) {

                    if (mLastLocation == null) {
                        mLastLocation = mLastLastLocation
                    }

                    updateDrawing()
                    mNeedUpdate = false

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
                mSunClockDrawable = SunclockDrawable(
                    mImageView?.width ?: 1024,
                    mImageView?.height ?: 1024)

                mImageView?.setImageDrawable(mSunClockDrawable)
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
                mProviderName = "best" // TODO FIX get from settings!
                startProvider()
            }
        }
    }

    private fun importSettings() {
        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this /* Activity context */)

        var tmp = sharedPreferences.getString("provider", "")
        if (tmp != null && tmp.isNotEmpty()) {
            mProviderName = tmp
        }
        if (mProviderName == "manual") {
            tmp = sharedPreferences.getString("manual_location", "")
            if (tmp != null && tmp.isNotEmpty()) {
                val pattern : Pattern = Pattern.compile("^([^,]+),(.+)$")
                val matcher : Matcher = pattern.matcher(tmp)
                val isMatch = matcher.matches()

                if (isMatch) {
                    try {
                        mLastLocation = Location("manual")
                        mLastLocation?.latitude = matcher.group(1)?.toDouble() ?: -91.0
                        mLastLocation?.longitude = matcher.group(2)?.toDouble() ?: -181.0
                        mLastLocation?.altitude = 0.0
                    }
                    catch (e: Exception) {
                        // ignore it
                    }
                }
            }
        }

        tmp = sharedPreferences.getString("timezone", "")
        if (tmp != null && tmp.isNotEmpty()) {
            mTimeZoneProvider = tmp
        }
        tmp = sharedPreferences.getString("manual_timezone", "")
        if (tmp != null && tmp.isNotEmpty()) {
            mManualTimeZone = tmp
        }

        tmp = sharedPreferences.getString("offset", "")
        if (tmp != null && tmp.isNotEmpty()) {
            mOffset = tmp
        }
        tmp = sharedPreferences.getString("manual_offset", "")
        if (tmp != null && tmp.isNotEmpty()) {
            mManualOffset = tmp
        }
    }

    private fun exportSettings() {
        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this /* Activity context */)
        val editor: SharedPreferences.Editor = sharedPreferences.edit()

        editor.putString("provider", mProviderName)
        if (mProviderName == "manual") {
            if (mLastLocation != null) {
                editor.putString("manual_location",
                    ((mLastLocation!!.latitude * 10000.0).toInt().toDouble() / 10000.0).toString() +
                    "," +
                    ((mLastLocation!!.longitude * 10000.0).toInt().toDouble() / 10000.0).toString())
            }
        }
        editor.putString("timezone", mTimeZoneProvider)
        editor.putString("manual_timezone", mManualTimeZone)
        editor.putString("offset", mOffset)
        editor.putString("manual_offset", mManualOffset)

        editor.apply()
    }

    private fun doEULA() {
        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this /* Activity context */)
        val yesEULA = sharedPreferences.getInt("EULA", 0)

        if (yesEULA > 0) {
            return
        }

        val checkBox = CheckBox(this)
        checkBox.text = "Agree to terms"
        val linearLayout = LinearLayout(this)
        linearLayout.layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.FILL_PARENT,
            LinearLayout.LayoutParams.FILL_PARENT
        )
        linearLayout.orientation = LinearLayout.VERTICAL
        linearLayout.addView(checkBox)

        val alertDialogBuilder: AlertDialog.Builder = AlertDialog.Builder(this)
        alertDialogBuilder.setView(linearLayout)
        alertDialogBuilder.setTitle("End User License Agreement")
        alertDialogBuilder.setMessage(
            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR " +
                    "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, " +
                    "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL " +
                    "THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER " +
                    "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING " +
                    "FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER " +
                    "DEALINGS IN THE SOFTWARE.")
        alertDialogBuilder.setPositiveButton("Ok",
            DialogInterface.OnClickListener { arg0, arg1 ->
                if (!checkBox.isChecked()) {
                    doEULA()
                }
                else {
                    val editor: SharedPreferences.Editor = sharedPreferences.edit()
                    editor.putInt("EULA", 1)
                    editor.apply()
                }
            })
        alertDialogBuilder.setCancelable(false)
        alertDialogBuilder.show()
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val manager = this.packageManager
        val info = manager.getPackageInfo(this.packageName, PackageManager.GET_ACTIVITIES)
        actionBar?.title = "ταμ clock v" + info.versionName
        supportActionBar?.title = "ταμ clock v" + info.versionName

        doEULA()

        mImageView = findViewById<View>(R.id.imageView) as ImageView
        mImageView?.setOnTouchListener(View.OnTouchListener { view, motionEvent ->
            this.otl(motionEvent)
            view.performClick()
            return@OnTouchListener true
        })

        mLocationManager = this.getSystemService(LOCATION_SERVICE) as LocationManager?
        mHandler.post(runVeryOften)

        //val runtime = Runtime.getRuntime()
        //mUsed = (runtime.totalMemory() - runtime.freeMemory()) / 1048576

        GlobalScope.launch(Dispatchers.IO) {
            mTimeZoneEngine = TimeZoneEngine.initialize()
            launch(Dispatchers.Main) {
                mEngineDone = true
                mNeedUpdate = true
            }
        }

        val locationButton: Button = findViewById(R.id.locationButton)
        locationButton.setOnClickListener { v ->
            chooseNewProvider()
            Toast.makeText(
                v!!.context,
                java.lang.String.format("Location Provider set to '%s'", mProviderName),
                Toast.LENGTH_SHORT
            ).show()

            if (mProviderName != "manual" && mLocationManager?.isProviderEnabled(mRealProviderName) == false) {
                Toast.makeText(
                    v.context,
                    "Go to Android Settings to enable this Location Provider",
                    Toast.LENGTH_LONG
                ).show()
            }

            mNeedUpdate = true
            exportSettings()
        }

        val alarmsButton: Button = findViewById(R.id.alarmsButton)
        alarmsButton.setOnClickListener { v ->
            val switchActivityIntent = Intent(v!!.context, AlarmActivity::class.java)
            val bundle = Bundle()
            try {
                val value = java.lang.String.format("%.4f,%.4f", mLastLocation!!.latitude,mLastLocation!!.longitude )
                bundle.putString("observer", value)
            }
            catch (e: Exception) {
                // ignore it
            }
            switchActivityIntent.putExtras(bundle)
            startActivity(switchActivityIntent)
        }

        val tzButton: Button = findViewById(R.id.tzButton)
        tzButton.setOnClickListener { v ->
            when (mTimeZoneProvider) {
                "system" -> {
                    mTimeZoneProvider = "location"
                }
                "location" -> {
                    mTimeZoneProvider = "manual"
                }
                "manual" -> {
                    mTimeZoneProvider = "system"
                }
            }
            Toast.makeText(
                v!!.context,
                java.lang.String.format("Timezone Provider set to '%s'", mTimeZoneProvider),
                Toast.LENGTH_SHORT
            ).show()
            mNeedUpdate = true
            exportSettings()
        }

        val settingsButton: Button = findViewById(R.id.settingsButton)
        settingsButton.setOnClickListener { v ->
            val switchActivityIntent = Intent(v!!.context, SettingsActivity::class.java)
            startActivity(switchActivityIntent)
        }

        val leftButton: Button = findViewById(R.id.leftButton)
        leftButton.setOnClickListener(
            object : View.OnClickListener {
                override fun onClick(v: View?) {
                    if (mOffset != "manual") {
                        Toast.makeText(v!!.context, "Offset Provider setting is not 'manual'", Toast.LENGTH_SHORT).show()
                        return
                    }
                    mManualOffset = (mManualOffset.toDouble() - 1.0).toString()
                    mNeedUpdate = true
                    exportSettings()
                }
            }
        )

        val rightButton: Button = findViewById(R.id.rightButton)
        rightButton.setOnClickListener(
            object : View.OnClickListener {
                override fun onClick(v: View?) {
                    if (mOffset != "manual") {
                        Toast.makeText(v!!.context, "Offset Provider settting is not 'manual'", Toast.LENGTH_SHORT).show()
                        return
                    }
                    mManualOffset = (mManualOffset.toDouble() + 1.0).toString()
                    mNeedUpdate = true
                    exportSettings()
                }
            }
        )
    }

    override fun onStart() {
        super.onStart()
        if (!checkPermissions()) {
            requestPermissions()
        }
    }

    private fun ponderAlarms() {
        var wakeup_clock = System.currentTimeMillis() / 1000 + 24*60*60 // 24 hours from now
        val alarmStorage = AlarmStorage(this)
        val categorynames = arrayOf(
            "SOLAR", "CIVIL", "NAUTICAL", "ASTRONOMICAL",
            "LUNAR",                   // moon and planets from here on
            "MERCURY", "VENUS", "MARS", "JUPITER", "SATURN",
            "ARIES")
        val typenames = arrayOf("RISE","TRANSIT","SET")

        for (i in 0..alarmStorage.getCount()-1) {
            val values = alarmStorage.getSet(i)
            val name = values[0]
            val observer = values[1]
            val category = values[2]
            val type = values[3]
            val offset = values[4]

            val latlon = observer?.split(",")
            val lat = latlon?.get(0)!!.toDouble()
            val lon = latlon.get(1).toDouble()

            val pondering_s = doWhenIsIt(lat, lon,
                category!!.toInt(),type!!.toInt() + 2, offset!!.toInt())

            val actual_clock = System.currentTimeMillis() / 1000 + pondering_s

            if (pondering_s < 60) {
                if (actual_clock > mSeenAlarmTime_clock) {


                    val alertDialogBuilder: AlertDialog.Builder = AlertDialog.Builder(this)
                    alertDialogBuilder.setTitle("ALARM: " + name)
                    val plusminus = if (offset.toInt() >= 0) {
                        "+"
                    } else {
                        ""
                    }
                    alertDialogBuilder.setMessage(observer + "\n" + categorynames[category.toInt()] + " " + typenames[type.toInt()] + " " + plusminus + offset + " minutes")
                    alertDialogBuilder.setPositiveButton("Ok",
                        DialogInterface.OnClickListener { arg0, arg1 ->
                            mSeenAlarmTime_clock = actual_clock
                        })
                    alertDialogBuilder.setCancelable(false)
                    alertDialogBuilder.show()
                }
            }
            else if (actual_clock < wakeup_clock) {
                wakeup_clock = actual_clock
            }
        }

        var sec = (wakeup_clock - (System.currentTimeMillis() / 1000))
        val min = (sec / 60) % 60
        val hour = sec / 3600
        sec %= 60

        Toast.makeText(
            this,
            java.lang.String.format("Next alarm check in %dh%02dm%02ds", hour, min, sec),
            Toast.LENGTH_SHORT
        ).show()
    }

    override fun onResume() {
        super.onResume()

        importSettings()

        if (checkPermissions()) {
            startProvider()
        }

        mNeedUpdate = true

 //       ponderAlarms()
    }

    override fun onPause() {
        super.onPause()

        exportSettings()

        if (checkPermissions()) {
            stopProvider()
        }
    }

    private fun isCloseToXY(motionEvent: MotionEvent, x: Double, y: Double, d: Double) : Boolean {
        val dcx = motionEvent.x - x
        val dcy = motionEvent.y - y
        val dc = sqrt(dcx*dcx+dcy*dcy)
        return dc < d
    }

    private fun isCloseToCenter(motionEvent: MotionEvent) : Boolean {
        val width = min(mImageView?.width ?: 1024, mImageView?.height ?: 1024)

        return isCloseToXY(motionEvent,
            (mImageView?.width ?: 1024) / 2.0,
            (mImageView?.height ?: 1024) / 2.0,
            (width/2).toDouble()
        )
    }

    private fun otl(motionEvent: MotionEvent) {
        val action = motionEvent.action and MotionEvent.ACTION_MASK

        if (mRealProviderName == "manual") {
            if (action == MotionEvent.ACTION_DOWN) {
                if (isCloseToCenter(motionEvent)) {
                    mOtlX = motionEvent.x
                    mOtlY = motionEvent.y

                    mOtlLat = mLastLocation?.latitude ?: 0.0
                    mOtlLon = mLastLocation?.longitude ?: 0.0
                    mOtlSpin = if (mLastLocation?.provider == "manual") {
                        mLastLocation!!.altitude
                    } else {
                        0.0
                    }
                    mOtlDown = true
                    mOtlChanged = true
                    mNeedUpdate = true
                    return
                }
            }

            if (!mOtlDown) {
                return
            }

            when (action) {
                MotionEvent.ACTION_POINTER_DOWN -> {
                    val x0 = motionEvent.getX(0)
                    val y0 = motionEvent.getY(0)
                    val x1 = motionEvent.getX(1)
                    val y1 = motionEvent.getY(1)
                    mOtlSpinBase = atan2(y1 - y0, x1 - x0)
                }
                MotionEvent.ACTION_UP -> {
                    mOtlDown = false
                    mNeedUpdate = true
                    exportSettings()
                }
                MotionEvent.ACTION_POINTER_UP -> {
                    mOtlDown = false
                    mNeedUpdate = true
                }
                MotionEvent.ACTION_MOVE -> {
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

                        while (proposedLocation.altitude < -180.0) {
                            proposedLocation.altitude += 360.0
                        }
                        while (proposedLocation.altitude > 180.0) {
                            proposedLocation.altitude -= 360.0
                        }

                    } else {
                        val width = min(mImageView?.width ?: 1024,mImageView?.height ?: 1024)
                        val deltaX = motionEvent.x - mOtlX
                        val deltaY = motionEvent.y - mOtlY

                        // TODO FIX the signs in the next 2 lines seem off.  it works as desired, but why?
                        proposedLocation.latitude = mOtlLat +  90.0 * (-deltaX * sin(mOtlSpin * PI / 180.0) + deltaY * cos(mOtlSpin * PI / 180.0)) / width
                        proposedLocation.longitude = mOtlLon + 90.0 * (-deltaY * sin(mOtlSpin * PI / 180.0) - deltaX * cos(mOtlSpin * PI / 180.0)) / (width)
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
                    mNeedUpdate = true
                }
            }
        }
    }

    class MyLocationListener(mainActivity: MainActivity) : LocationListener {
        private var context = mainActivity

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
        mRealProviderName = mProviderName
        if (mProviderName == "best") {
            val criteria = Criteria()
            mRealProviderName = mLocationManager?.getBestProvider(criteria,true) ?: "manual"
        }
        if (mRealProviderName != "manual") {
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
                        mRealProviderName,
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
        if (mProviderName != "manual") {
            mLocationManager?.removeUpdates(mLocationListener)
        }
    }

    private fun chooseNewProvider() {
        stopProvider()

        val allProviders = mLocationManager!!.getProviders(false)
        allProviders.sort()
        allProviders.add(0, "best")
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

    private external fun doAll(lat:Double, lon:Double, offset:Double, width:Int, provider:String, tzprovider:String, tz:String) : IntArray
    private external fun doGlobe(lat:Double, lon:Double, spin:Double, width:Int, tzname:String) : IntArray
    private external fun doWhenIsIt(lat:Double, lon:Double, category:Int, type:Int, offset_minutes:Int) : Int
}
