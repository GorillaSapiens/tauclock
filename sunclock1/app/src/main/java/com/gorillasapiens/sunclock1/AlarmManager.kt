package com.gorillasapiens.sunclock1

import android.content.Context
import android.content.SharedPreferences
import androidx.preference.PreferenceManager

class AlarmManager(context: Context, fields: Array<String>) {
    val members = fields // arrayOf("name", "observer", "category", "type", "offset")
    val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(context);

    fun getCount(): Int {
        return sharedPreferences.getInt("alarms", 0)
    }

    private fun getTuple(i:Int, s:String) : String {
        val key = java.lang.String.format("%d_%s", i, s)
        val value = sharedPreferences.getString(key, "")
        if (value == null) {
            return ""
        }
        else {
            return value
        }
    }

    private fun putTuple(editor:SharedPreferences.Editor, i:Int, s:String, value:String?) {
        val key = java.lang.String.format("%d_%s", i, s)
        editor.putString(key, value)
    }

    fun deleteSet(i:Int) {
        val max = getCount();
        val editor: SharedPreferences.Editor = sharedPreferences.edit()

        for (j in i..(max-2)) {
            for (member in members) {
                val value = getTuple(j + 1, member)
                putTuple(editor, j, member, value)
            }
        }
        for (member in members) {
            putTuple(editor, max - 1, member, null)
        }
        editor.putInt("alarms", max-1)
        editor.commit()
    }

    fun getSet(i:Int) : Array<String> {
        val max = getCount()
        var ret : Array<String> = emptyArray()

        for (member in members) {
            val value = getTuple(i, member)
            ret += value;
        }

        return ret;
    }

    fun addSet(values: Array<String>) {
        val editor: SharedPreferences.Editor = sharedPreferences.edit()
        if (values.size != members.size) {
            throw Exception("size mismatch")
        }
        val count = getCount();
        for (i in 0..members.size) {
            putTuple(editor, i, members[i], values[i])
        }
        editor.putInt("alarms", count + 1)
        editor.commit()
    }
}