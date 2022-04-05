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
        val cx = mWidth / 2.0f
        val cy = mHeight / 2.0f
        var cr : Float
        if (mWidth > mHeight) {
            cr = cy
        }
        else {
            cr = cx
        }
        canvas.drawArc(cx-cr,cy-cr, cx + cr,cy +cr,20f,140f,true, mPaint)
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