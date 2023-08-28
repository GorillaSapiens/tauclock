package com.gorillasapiens.sunclock1

//import android.R

import android.Manifest
import android.annotation.SuppressLint
import android.annotation.TargetApi
import android.app.ActivityManager
import android.app.AlarmManager
import android.app.PendingIntent
import android.content.Context
import android.content.DialogInterface
import android.content.Intent
import android.content.SharedPreferences
import android.content.pm.PackageManager
import android.content.res.Configuration
import android.location.Criteria
import android.location.Location
import android.location.LocationListener
import android.location.LocationManager
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.provider.Settings
import android.util.Log
import android.view.MotionEvent
import android.view.View
import android.view.WindowManager
import android.widget.*
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.preference.PreferenceManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import net.iakovlev.timeshape.TimeZoneEngine
import java.text.DateFormatSymbols
import java.time.Duration
import java.time.LocalDateTime
import java.time.temporal.ChronoUnit
import java.util.*
import java.util.regex.Matcher
import java.util.regex.Pattern
import kotlin.math.*


class MainActivity : AppCompatActivity() {
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

    private var mLocationManager : LocationManager? = null
    private var mProviderName: String = "best"
    private var mRealProviderName : String = "gps"
    private var mManualLocation : String? = null

    private var mTimeZoneProvider : String = "system"
    private var mManualTimeZone : String = "Etc/GMT"

    private var mOffset: String = "none"
    private var mManualOffset : String = "0"

    private var mLight : Int = 0
    private var mDark : Int = 0

    // Create the Handler object (on the main thread by default)
    private var mHandler = Handler(Looper.getMainLooper())

    companion object {
        var mTimeZoneEngine: TimeZoneEngine? = null
        var mEngineDone = false
        var ACTION_MANAGE_OVERLAY_PERMISSION_REQUEST_CODE = 5469

        private val dfs = DateFormatSymbols.getInstance()
        val monthnames = dfs.months
        val weekdaynames = dfs.weekdays

        init {
            System.loadLibrary("tauclock")
        }
    }

    private val mManualOffsetPattern: Pattern = Pattern.compile("^([0-9]+)[-/]([0-9]+)[-/]([0-9]+(\\.[0-9]*)?)$")

    private fun manualOffset2Double() : Double {
        var ret = 0.0
        if (mManualOffset.contains("/") || mManualOffset.indexOf("-") > 0) {
            try {
                val m :Matcher = mManualOffsetPattern.matcher(mManualOffset)
                m.find()
                val year: Int = m.group(1).toInt()
                val month: Int = m.group(2).toInt()
                val day: Int = m.group(3).toDouble().toInt()
                val residual: Double = m.group(3).toDouble() - day.toDouble()

                val now = LocalDateTime.now()
                val then = LocalDateTime.of(year,month,day,0,0)

                ret = now.until(then, ChronoUnit.DAYS).toDouble()
                ret += residual
            }
            catch (e: Exception) {
                // ignore it...
            }
        }
        else {
            ret =mManualOffset.toDouble()
        }
        return ret
    }

    private fun manualOffsetAdjust(days: Long) {
        if (mManualOffset.contains("/") || mManualOffset.indexOf("-") > 0) {
            try {
                val m :Matcher = mManualOffsetPattern.matcher(mManualOffset)
                m.find()
                val year: Int = m.group(1).toInt()
                val month: Int = m.group(2).toInt()
                val day: Int = m.group(3).toDouble().toInt()
                val residual: Double = m.group(3).toDouble() - day.toDouble()

                val then = LocalDateTime.of(year,month,day,0,0).plusDays(days)

                val sep = if (mManualOffset.contains("/")) "/" else "-"
                mManualOffset =
                    then.year.toString() + sep + then.month.value.toString() + sep +
                            (then.dayOfMonth.toDouble() + residual).toString()
            }
            catch (e: Exception) {
                // ignore it...
            }
        }
        else {
            mManualOffset = (mManualOffset.toDouble() + days.toDouble()).toString()
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
                displayProvider += "\n("
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
                    offset = manualOffset2Double()
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
                    displayProvider, mTimeZoneProvider, tzname,
                    ((mLight shl 8) or mDark), monthnames, weekdaynames
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
    private fun checkLocationPermissions(): Boolean {
        return ActivityCompat.checkSelfPermission(
            this,
            Manifest.permission.ACCESS_COARSE_LOCATION
        ) == PackageManager.PERMISSION_GRANTED && ActivityCompat.checkSelfPermission(
            this,
            Manifest.permission.ACCESS_FINE_LOCATION
        ) == PackageManager.PERMISSION_GRANTED
    }

    // method to request for permissions
    private fun requestLocationPermissions() {
        ActivityCompat.requestPermissions(
            this, arrayOf(
                Manifest.permission.ACCESS_COARSE_LOCATION,
                Manifest.permission.ACCESS_FINE_LOCATION
            ), mPermissionID
        )
    }

    private fun checkSystemAlertPermission() {
        val activityManager = getSystemService(ACTIVITY_SERVICE) as ActivityManager
        if (activityManager.isLowRamDevice()) {
            val alertDialogBuilder: AlertDialog.Builder = AlertDialog.Builder(this)
            alertDialogBuilder.setTitle("Low Ram Device")
            alertDialogBuilder.setMessage(
                "Repeating alarms will not function on low RAM devices.")
            alertDialogBuilder.setPositiveButton("Ok",
                DialogInterface.OnClickListener { arg0, arg1 ->
                })
            alertDialogBuilder.setCancelable(false)
            alertDialogBuilder.show()
        }
        else {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                if (!Settings.canDrawOverlays(this)) {
                    val alertDialogBuilder: AlertDialog.Builder = AlertDialog.Builder(this)
                    alertDialogBuilder.setTitle("Permission Required")
                    alertDialogBuilder.setMessage(
                        "Tauclock also needs permission to draw over other apps.  This is necessary " +
                                "for proper functioning of repeating alarms, and needs to be turned on manually " +
                                "for this app in system settings.")
                    alertDialogBuilder.setPositiveButton("Ok",
                        DialogInterface.OnClickListener { arg0, arg1 ->
                            val intent = Intent(
                                Settings.ACTION_MANAGE_OVERLAY_PERMISSION,
                                Uri.parse("package:$packageName")
                            )
                            startActivityForResult(intent, ACTION_MANAGE_OVERLAY_PERMISSION_REQUEST_CODE)
                        })
                    alertDialogBuilder.setCancelable(false)
                    alertDialogBuilder.show()
                }
            }
        }
    }

    @TargetApi(Build.VERSION_CODES.M)
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == ACTION_MANAGE_OVERLAY_PERMISSION_REQUEST_CODE) {
            if (!Settings.canDrawOverlays(this)) {
                // You don't have permission
                checkSystemAlertPermission()
            } else {
                // Do as per your logic
            }
        }
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

    private fun commitManualLocation() {
        mLastLocation = Location("manual")
        mLastLocation?.latitude = -91.0
        mLastLocation?.longitude = -181.0
        mLastLocation?.altitude = 0.0

        if (mManualLocation != null && mManualLocation!!.isNotEmpty()) {
            val parts = mManualLocation!!.split(",")

            val tmp =
            if (parts.size >= 2) {
                parts[parts.size - 2] + "," + parts[parts.size - 1]
            }
            else {
                ""
            }

            val pattern : Pattern = Pattern.compile("^([^,]+),(.+)$")
            val matcher : Matcher = pattern.matcher(tmp)
            val isMatch = matcher.matches()

            if (isMatch) {
                try {
                    mLastLocation?.latitude = matcher.group(1)?.toDouble() ?: -91.0
                    mLastLocation?.longitude = matcher.group(2)?.toDouble() ?: -181.0
                    mLastLocation?.altitude = 0.0
                }
                catch (e: Exception) {
                }
            }
        }
    }

    private fun importSettings() {
        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this /* Activity context */)

        var tmp = sharedPreferences.getString("provider", "")
        if (tmp != null && tmp.isNotEmpty()) {
            mProviderName = tmp
        }
        mManualLocation = sharedPreferences.getString("manual_location", "")
        if (mProviderName == "manual") {
            commitManualLocation()
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
        if (tmp != null && tmp.replace(" ", "").isNotEmpty()) {
            mManualOffset = tmp.replace(" ", "")
        }

        mLight = sharedPreferences.getInt("light", 0)
        mDark = sharedPreferences.getInt("dark", 0)
    }

    private fun exportSettings() {
        val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this /* Activity context */)
        val editor: SharedPreferences.Editor = sharedPreferences.edit()

        editor.putString("provider", mProviderName)
        if (mProviderName == "manual") {
            if (mLastLocation != null) {
                var latitude = mLastLocation!!.latitude
                var longitude = mLastLocation!!.longitude

                if (latitude > 0.0) {
                    latitude += 0.00005
                }
                else {
                    latitude -= 0.00005
                }

                if (longitude > 0.0) {
                    longitude += 0.00005
                }
                else {
                    longitude -= 0.00005
                }

                val tmp = ((latitude * 10000.0).toInt().toDouble() / 10000.0).toString() +
                        "," +
                        ((longitude * 10000.0).toInt().toDouble() / 10000.0).toString()

                if (!mManualLocation!!.contains("," + tmp)) {
                    editor.putString("manual_location", tmp)
                }
            }
        }
        editor.putString("timezone", mTimeZoneProvider)
        editor.putString("manual_timezone", mManualTimeZone)
        editor.putString("offset", mOffset)
        editor.putString("manual_offset", mManualOffset.replace(" ", ""))
        editor.putInt("light", mLight)
        editor.putInt("dark", mDark)

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
                if (!checkBox.isChecked) {
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
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        val manager = this.packageManager
        val info = manager.getPackageInfo(this.packageName, PackageManager.GET_ACTIVITIES)
        actionBar?.title = "ταμ clock v" + info.versionName
        supportActionBar?.title = "ταμ clock v" + info.versionName

        if (getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE) {
            actionBar?.hide()
            supportActionBar?.hide()
        }

        doEULA()

        mImageView = findViewById<View>(R.id.imageView) as ImageView
        mImageView?.setOnTouchListener(View.OnTouchListener { view, motionEvent ->
            this.viewOnTouchListener(motionEvent)
            view.performClick()
            return@OnTouchListener true
        })

        mLocationManager = this.getSystemService(LOCATION_SERVICE) as LocationManager?
        mHandler.post(runVeryOften)

        //val runtime = Runtime.getRuntime()
        //mUsed = (runtime.totalMemory() - runtime.freeMemory()) / 1048576

        GlobalScope.launch(Dispatchers.IO) {
            if (mTimeZoneEngine == null) {
                mTimeZoneEngine = TimeZoneEngine.initialize()
                launch(Dispatchers.Main) {
                    mEngineDone = true
                    mNeedUpdate = true
                }
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

            if (mProviderName == "manual") {
                commitManualLocation()
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
                    manualOffsetAdjust(-1);
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
                    manualOffsetAdjust(1)
                    mNeedUpdate = true
                    exportSettings()
                }
            }
        )

        val alarmIntent = Intent(this, UpdateReceiver::class.java)
        alarmIntent.putExtra("origin", "alarm")
        alarmIntent.putExtra("insert", true)
        val pendingIntent = PendingIntent.getBroadcast(
            this, 0, alarmIntent,
            PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_MUTABLE
        )
        val alarmManager = getSystemService(Context.ALARM_SERVICE) as AlarmManager
        alarmManager.setInexactRepeating(
            AlarmManager.RTC_WAKEUP, Calendar.getInstance().timeInMillis,
            AlarmManager.INTERVAL_HALF_HOUR, pendingIntent
        )

        checkSystemAlertPermission()
    }

    override fun onStart() {
        super.onStart()
        if (!checkLocationPermissions()) {
            requestLocationPermissions()
        }
    }

    override fun onResume() {
        super.onResume()

        importSettings()

        if (checkLocationPermissions()) {
            startProvider()
        }

        mNeedUpdate = true
    }

    override fun onPause() {
        super.onPause()

        exportSettings()

        if (checkLocationPermissions()) {
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

    private fun isUpperLeft(motionEvent: MotionEvent) : Boolean {
        return (motionEvent.x < (mImageView!!.width / 5.0) &&
                motionEvent.y < (mImageView!!.height / 5.0))
    }

    private fun viewOnTouchListener(motionEvent: MotionEvent) {
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
                        proposedLocation.latitude = mOtlLat +  90.0 * (-deltaX * sin(mOtlSpin * PI / 180.0) + deltaY * cos(mOtlSpin * PI / 180.0)) / (width)
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
                    // kludge undo some stuff...
                    mLastLocation = proposedLocation
                    mNeedUpdate = true
                }
            }
        }
        else {
            if (action == MotionEvent.ACTION_DOWN) {
                if (isUpperLeft(motionEvent)) {
                    mOtlLat = mLastLocation?.latitude ?: 0.0
                    mOtlLon = mLastLocation?.longitude ?: 0.0
                    mOtlDown = !mOtlDown
                    mOtlChanged = true
                    mNeedUpdate = true
                    return
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

            if (checkLocationPermissions()) {
                try {
                    mLocationManager?.requestLocationUpdates(
                        mRealProviderName,
                        10000,          // 10-second interval.
                        10.0f,             // 10 meters.
                        mLocationListener
                    )
                }
                catch(e : SecurityException) {
                    requestLocationPermissions()
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

    private external fun doAll(
        lat: Double, lon: Double, offset: Double,
        width: Int, provider: String, tzprovider: String, tz: String,
        lightdark: Int,
        monthnames: Array<String>,
        weekdaynames: Array<String>
    ) : IntArray
    private external fun doGlobe(lat:Double, lon:Double, spin:Double, width:Int, tzname:String) : IntArray
}
