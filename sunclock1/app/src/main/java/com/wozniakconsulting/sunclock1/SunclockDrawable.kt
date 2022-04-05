package com.wozniakconsulting.sunclock1
import android.graphics.*
import android.graphics.Paint.Style
import android.graphics.drawable.Drawable
import android.location.Location


class SunclockDrawable(width: Int, height: Int) : Drawable() {
    private val mPaint: Paint
    private val mWidth: Int
    private val mHeight: Int
    private var mThing: IntArray

    //private native void do_all(double, double,double);

    fun setThing(something: IntArray) {
        mThing = something;
    }

    override fun draw(canvas: Canvas) {
        synchronized(mThing) {
            if (mThing.size > 0) {
                val w = mThing[0];
                val h = mThing[1];

                val x = (mWidth / 2.0f) - (w / 2.0f)
                val y = (mHeight / 2.0f) - (h / 2.0f)

//                val image = mThing.slice(2 .. mThing.size)
                canvas.drawBitmap(mThing, 2, w.toInt(), x.toInt(), y.toInt(), w.toInt(), h.toInt(), true, null)

                mPaint.setARGB(255, 0, 0, 0)
                mPaint.setStrokeWidth(2.0f)
                mPaint.setStyle(Style.FILL)

                var r : Rect;
                // above
                r = Rect(0,0,mWidth.toInt(),y.toInt());
                canvas.drawRect(r, mPaint);
                // below
                r = Rect(0,(y+h).toInt(),mWidth.toInt(), mHeight.toInt());
                canvas.drawRect(r, mPaint);
                // left
                r = Rect(0,y.toInt(),x.toInt(), (y+h).toInt());
                canvas.drawRect(r, mPaint);
                // right
                r = Rect((x+w).toInt(),y.toInt(),mWidth.toInt(), (y+h).toInt());
                canvas.drawRect(r, mPaint);
            } else {
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
        }
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
        mThing = IntArray(0)
    }
}