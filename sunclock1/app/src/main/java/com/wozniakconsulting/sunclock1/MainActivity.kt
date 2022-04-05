package com.wozniakconsulting.sunclock1

//import android.R
//import android.R
import android.os.Bundle
import android.view.View
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity


class MainActivity : AppCompatActivity() {
    var drawable_initialized = false

    companion object {
      init {
         System.loadLibrary("libnova")
      }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            if (!drawable_initialized) {
                //val layObj: LinearLayout = findViewById<View>(R.id.parentLay) as LinearLayout
                val imageView: ImageView = findViewById<View>(R.id.imageView) as ImageView
                val myDrawObj = SunclockDrawable(imageView.width, imageView.height)
                imageView.setImageDrawable(myDrawObj)
                imageView.invalidate()

                drawable_initialized = true;
            }
        }
    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }
}