package prj.kwui;

import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import android.annotation.SuppressLint;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.view.Surface;
import android.view.View;

import java.util.Objects;

import prj.kwui.databinding.ActivityKwuiBinding;

public class KwuiActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    public long aNative;

    private ActivityKwuiBinding binding;

    static public KwuiActivity instance;

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        instance = this;

        aNative = Native.nCreate(getAssets(), ":/entry.js");

        binding = ActivityKwuiBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
    }

    public void createDialog(String id) {
        KwuiActivity.instance.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final ConstraintLayout cl_main = findViewById(R.id.cl_main);
                KwuiSurface surface = new KwuiSurface(KwuiActivity.instance, id);
                ConstraintLayout.LayoutParams params = new ConstraintLayout.LayoutParams(
                        ConstraintLayout.LayoutParams.MATCH_PARENT,
                        ConstraintLayout.LayoutParams.MATCH_PARENT);
                surface.setLayoutParams(params);
                cl_main.addView(surface);
            }
        });
    }

    public void releaseDialog(String id) {
        KwuiActivity.instance.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final ConstraintLayout cl_main = findViewById(R.id.cl_main);
                int n = cl_main.getChildCount();
                for (int i = 0; i < n; ++i) {
                    View v = cl_main.getChildAt(i);
                    if (v instanceof KwuiSurface) {
                        KwuiSurface surface = (KwuiSurface) v;
                        if (Objects.equals(surface.mId, id)) {
                            cl_main.removeView(v);
                            break;
                        }
                    }
                }
            }
        });
    }

    protected String getMainSharedObject() {
        return "libmain.so";
    }

    protected String getMainFunction() {
        return "kwui_main";
    }
}

class Native {
    static {
        // The runtime will add "lib" on the front and ".o" on the end of
        // the name supplied to loadLibrary.
        System.loadLibrary("kwui");
    }

    static native long nCreate(AssetManager am, String entry_js);

    static native void nDestroy(long obj);

    static native void nSurfaceChanged(long obj, String id, Surface surface, int format, int width, int height, float density);

    static native void nSurfaceDestroyed(long obj, String id);

    static native void nSurfaceRedrawNeeded(long obj, String id);

    static native void nHandleTouchEvent(long obj, String id, int action, float x, float y);

    static native void nHandleScrollEvent(long obj, String id, float x, float y, float dx, float dy);

    static native void nHandleShowPressEvent(long obj, String id, float x, float y);

    static native void nHandleLongPressEvent(long obj, String id, float x, float y);

    static native void nHandleSingleTapConfirmedEvent(long obj, String id, float x, float y);

    public static void jCreateDialog(String id) {
        KwuiActivity.instance.createDialog(id);
    }

    public static void jReleaseDialog(String id) {
        KwuiActivity.instance.releaseDialog(id);
    }

    public static String jGetMainSharedObject() {
        return KwuiActivity.instance.getMainSharedObject();
    }

    public static String jGetMainFunction() {
        return KwuiActivity.instance.getMainFunction();
    }
}