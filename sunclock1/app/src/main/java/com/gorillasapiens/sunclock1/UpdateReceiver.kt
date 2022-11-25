package com.gorillasapiens.sunclock1

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent

class UpdateReceiver : BroadcastReceiver() {

    override fun onReceive(context: Context, intent: Intent) {
        // This method is called when the BroadcastReceiver is receiving an Intent broadcast.
        val alarmStorage = AlarmStorage(context)
        alarmStorage.refresh()
    }
}