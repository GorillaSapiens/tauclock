package com.wozniakconsulting.sunclock1
import android.graphics.*
import android.graphics.Paint.Style
import android.graphics.drawable.Drawable


class SunclockDrawable(width: Int, height: Int) : Drawable() {
    private val mPaint: Paint
    private val mWidth: Int
    private val mHeight: Int

    override fun draw(canvas: Canvas) {
        // Set the correct values in the Paint
        mPaint.setARGB(255, 255, 0, 0)
        mPaint.setStrokeWidth(2.0f)
        mPaint.setStyle(Style.FILL)

        // Draw it
        var cr : Float
        if (mWidth > mHeight) {
            cr = mHeight / 2.0f
        }
        else {
            cr = mWidth / 2.0f
        }
        cr=1000.0f
        canvas.drawArc(-cr,-cr,cr,cr,0f,180f,false, mPaint)
    }

    override fun getOpacity(): Int {
        return PixelFormat.OPAQUE
    }

    override fun setAlpha(arg0: Int) {}
    override fun setColorFilter(arg0: ColorFilter?) {}

    init {
        mPaint = Paint()
        mWidth = width
        mHeight = height
    }
}