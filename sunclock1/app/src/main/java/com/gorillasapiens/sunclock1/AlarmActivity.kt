package com.gorillasapiens.sunclock1

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.MenuItem
import android.widget.Button

class AlarmActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_alarm)

        val backButton: Button = findViewById(R.id.backButton)
        backButton.setOnClickListener { v ->
            finish()
        }
    }

//    override fun onOptionsItemSelected(item: MenuItem): Boolean {
//        when (item.getItemId()) {
//            android.R.id.home -> {
//                super.onBackPressed()
//                return true
//            }
//        }
//       return super.onOptionsItemSelected(item)
//   }
}