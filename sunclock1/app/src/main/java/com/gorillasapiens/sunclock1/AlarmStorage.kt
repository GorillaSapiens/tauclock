package com.gorillasapiens.sunclock1

import android.content.Context
import android.content.SharedPreferences
import androidx.preference.PreferenceManager

class AlarmStorage(context: Context) {
    companion object {
        val fields = arrayOf("label", "description", "observer", "category", "type", "delay")
    }

    val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(context);

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

    fun putLedger(label: String?, hour:Int, minute:Int) {
        val editor: SharedPreferences.Editor = sharedPreferences.edit()
        val value = hour.toString() + ":" + minute.toString()
        editor.putString(label, value)
        editor.commit()
    }

    fun getLedger(label: String?) : Array<Int> {
        val time = sharedPreferences.getString(label, "0:0")
        return time!!.split(":").map { it.toInt() }.toTypedArray()
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

}