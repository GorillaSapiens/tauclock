package com.gorillasapiens.sunclock1

import android.R.id
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.content.SharedPreferences
import android.provider.AlarmClock
import android.util.Log
import androidx.preference.PreferenceManager
import java.util.*


class AlarmStorage(context: Context) {
    companion object {
        val fields = arrayOf("label", "description", "observer", "category", "type", "delay")
    }

    val mContext = context
    val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(context)

    private fun getTuple(i:Int, s:String) : String? {
        val key = java.lang.String.format("%d_%s", i, s)
        return sharedPreferences.getString(key, "")
    }

    private fun putTuple(editor:SharedPreferences.Editor, i:Int, s:String, value:String?) {
        val key = java.lang.String.format("%d_%s", i, s)
        editor.putString(key, value)
    }

    private fun removeTuple(editor:SharedPreferences.Editor, i:Int, s:String) {
        val key = java.lang.String.format("%d_%s", i, s)
        editor.remove(key)
    }

    fun getCount(): Int {
        return sharedPreferences.getInt("alarms", 0)
    }

    fun putLedger(label: String?, hour: Int, minute: Int, time: Long) {
        val editor: SharedPreferences.Editor = sharedPreferences.edit()
        val value = hour.toString() + ":" + minute.toString() + ":" + time.toString()
        editor.putString(label, value)
        editor.commit()
    }

    fun getLedger(label: String?) : Array<Long> {
        val time = sharedPreferences.getString(label, "0:0:0")
        return time!!.split(":").map { it.toLong() }.toTypedArray()
    }

    fun deleteLedger(label: String?) {
        val editor: SharedPreferences.Editor = sharedPreferences.edit()
        editor.remove(label)
        editor.commit()
    }

    fun deleteSet(i:Int) {
        if (i < 0 || i >= getCount()) {
            throw Exception("size mismatch")
        }

        val max = getCount();
        val editor: SharedPreferences.Editor = sharedPreferences.edit()

        for (j in i..(max-2)) {
            for (member in fields) {
                val value = getTuple(j + 1, member)
                putTuple(editor, j, member, value)
            }
        }
        for (member in fields) {
            removeTuple(editor, max - 1, member)
        }
        editor.putInt("alarms", max-1)
        editor.commit()
    }

    fun getSet(i:Int) : Array<String?> {
        if (i < 0 || i >= getCount()) {
            throw Exception("size mismatch")
        }

        var ret : Array<String?> = emptyArray()

        for (member in fields) {
            val value = getTuple(i, member)
            ret += value;
        }

        return ret;
    }

    fun putSet(n: Int, values: Array<String?>) {
        val editor: SharedPreferences.Editor = sharedPreferences.edit()
        if (values.size != fields.size) {
            throw Exception("size mismatch")
        }
        var spot = n
        if (n < 0 || n > getCount()) {
            spot = getCount()
        }
        for (i in 0..(fields.size-1)) {
            putTuple(editor, spot, fields[i], values[i])
        }
        if (spot == getCount()) {
            editor.putInt("alarms", spot + 1)
        }
        editor.commit()
    }

    fun ponderAdd(values: Array<String?>, inAlarm: Boolean = false) {
        //var wakeup_clock = System.currentTimeMillis() / 1000 + 24*60*60 // 24 hours from now
        //val alarmStorage = AlarmStorage(this)
        val categorynames = arrayOf(
            "SOLAR", "CIVIL", "NAUTICAL", "ASTRONOMICAL",
            "LUNAR",                   // moon and planets from here on
            "MERCURY", "VENUS", "MARS", "JUPITER", "SATURN",
            "ARIES")
        val typenames = arrayOf("RISE","TRANSIT","SET")

        val label = values[0]
        val description = values[1]
        val observer = values[2]
        val category = values[3]
        val type = values[4]
        val delay = values[5]

        val message = String.format("%s\n%s\n%s %s %s minutes at %s",
            label,
            description,
            categorynames[category!!.toInt()], typenames[type!!.toInt()],
            delayString(delay),
            observer)

        val latlon = observer?.split(",")
        val lat = latlon?.get(0)!!.toDouble()
        val lon = latlon.get(1).toDouble()

        val pondering_s = doWhenIsIt(lat, lon,
            category!!.toInt(),type!!.toInt() + 2, delay!!.toInt())

        if (pondering_s > 0) {
            val calendar = Calendar.getInstance()
            calendar.add(Calendar.SECOND, pondering_s)

            putLedger(label,
                calendar.get(Calendar.HOUR_OF_DAY),
                calendar.get(Calendar.MINUTE),
                calendar.getTimeInMillis())

            val intent = Intent(AlarmClock.ACTION_SET_ALARM)
            intent.putExtra(AlarmClock.EXTRA_HOUR, calendar.get(Calendar.HOUR_OF_DAY)) // Integer
            intent.putExtra(AlarmClock.EXTRA_MINUTES, calendar.get(Calendar.MINUTE)) // Integer
            intent.putExtra(AlarmClock.EXTRA_MESSAGE, message)
            intent.putExtra(AlarmClock.EXTRA_SKIP_UI, true)
            if (inAlarm) {
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                intent.addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK)
            }
            Log.d("starting intent", "before " + inAlarm.toString())
            mContext.startActivity(intent)
            Log.d("starting intent", "after " + inAlarm.toString())
        }
    }

    fun delayString(delay: String?): String {
        var value = 0
        try {
            value = delay!!.toInt()
        }
        catch (e: Exception) {
            // basically ignore it
        }
        if (value < 0) {
            return value.toString()
        }
        else {
            return "+" + value.toString()
        }
    }

    fun ponderEdit(values: Array<String?>) {
        val hourminute = getLedger(values[0])
        deleteLedger(values[0])
        ponderAdd(values)
        ponderDelete(values[0], hourminute[0].toInt(), hourminute[1].toInt())
    }

    fun ponderDelete(label: String?, hour: Int, minute: Int) {
        val intent = Intent(AlarmClock.ACTION_DISMISS_ALARM)
        intent.putExtra(AlarmClock.EXTRA_ALARM_SEARCH_MODE, AlarmClock.ALARM_SEARCH_MODE_TIME)
        intent.putExtra(AlarmClock.EXTRA_HOUR, hour)
        intent.putExtra(AlarmClock.EXTRA_MINUTES, minute)
        intent.putExtra(AlarmClock.EXTRA_SKIP_UI, true)
        mContext.startActivity(intent)
    }

    fun refresh() {
        for (i in 0..(getCount()-1)) {
            val values = getSet(i)
            val ledger = getLedger(values[0])
            if (ledger == null || ledger.size != 3 || ledger[2] < Calendar.getInstance().timeInMillis) {
                // refresh this alarm
                deleteLedger(values[0])
                ponderAdd(values, true)
            }
        }
    }

    private external fun doWhenIsIt(lat:Double, lon:Double, category:Int, type:Int, delayMinutes:Int) : Int
}