package com.wozniakconsulting.sunclock1
import android.graphics.ColorFilter
import android.graphics.PixelFormat
import android.graphics.RectF
import android.graphics.Paint
import android.graphics.Canvas
import android.graphics.Paint.Style
import android.graphics.drawable.Drawable


class SunclockDrawable : Drawable() {
    private val mPaint: Paint
    private val mRect: RectF
    override fun draw(canvas: Canvas) {
        // Set the correct values in the Paint
        mPaint.setARGB(255, 255, 0, 0)
        mPaint.setStrokeWidth(2.0f)
        mPaint.setStyle(Style.FILL)

        // Adjust the rect
        mRect.left = 15.0f
        mRect.top = 50.0f
        mRect.right = 55.0f
        mRect.bottom = 75.0f

        // Draw it
        canvas.drawRoundRect(mRect, 0.5f, 0.5f, mPaint)
    }

    override fun getOpacity(): Int {
        return PixelFormat.OPAQUE
    }

    override fun setAlpha(arg0: Int) {}
    override fun setColorFilter(arg0: ColorFilter?) {}

    init {
        mPaint = Paint()
        mRect = RectF()
    }
}