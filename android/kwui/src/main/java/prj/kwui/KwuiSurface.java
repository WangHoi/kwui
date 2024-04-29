package prj.kwui;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import androidx.annotation.NonNull;

public class KwuiSurface extends SurfaceView implements SurfaceHolder.Callback2 {
    public String mId;
    private GestureDetector mDetector;

    @SuppressLint("ClickableViewAccessibility")
    public KwuiSurface(Context context, String id) {
        super(context);
        mId = id;
        getHolder().addCallback(this);
        mDetector = new GestureDetector(this.getContext(), new MyGestureListener(mId));
        setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                // Native.nHandleTouchEvent(aNative, event.getAction(), event.getX(), event.getY());
                // Log.i(TAG, String.format("onTouch action=%d pos=%.2f, %.2f", event.getAction(), event.getX(), event.getY()));
                mDetector.onTouchEvent(event);
                return true;
            }
        });
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder surfaceHolder) {
        Log.i("Surface", "SurfaceCreated");
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder surfaceHolder, int format, int width, int height) {
        float density = getResources().getDisplayMetrics().density;
        Native.nSurfaceChanged(KwuiActivity.instance.aNative, mId, surfaceHolder.getSurface(), format, width, height, density);
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder surfaceHolder) {
        Native.nSurfaceDestroyed(KwuiActivity.instance.aNative, mId);
    }

    @Override
    public void surfaceRedrawNeeded(@NonNull SurfaceHolder surfaceHolder) {
        Native.nSurfaceRedrawNeeded(KwuiActivity.instance.aNative, mId);
    }


    static class MyGestureListener extends GestureDetector.SimpleOnGestureListener {
        private static final String DEBUG_TAG = "Gestures";
        private String mId;

        public MyGestureListener(String id)
        {
            mId = id;
        }

        @Override
        public void onShowPress (MotionEvent e)
        {
            Native.nHandleShowPressEvent(KwuiActivity.instance.aNative, mId, e.getX(), e.getY());
        }

        @Override
        public void onLongPress (MotionEvent e)
        {
            Native.nHandleLongPressEvent(KwuiActivity.instance.aNative, mId, e.getX(), e.getY());
        }

        @Override
        public boolean onSingleTapConfirmed(MotionEvent e) {
            Native.nHandleSingleTapConfirmedEvent(KwuiActivity.instance.aNative, mId, e.getX(), e.getY());
            return true;
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
            // Log.i(DEBUG_TAG, String.format("onScroll e1=%.0f,%.0f e2=%.0f,%.0f delta=%.0f,%.0f",
            //         e1.getX(), e1.getY(), e2.getX(), e2.getY(), distanceX, distanceY));
            Native.nHandleScrollEvent(KwuiActivity.instance.aNative, mId, e2.getX(), e2.getY(), distanceX, distanceY);
            return true;
        }
    }

}
