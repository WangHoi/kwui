package prj.kwui;

import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.Surface;
import android.view.View;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.widget.RelativeLayout;

import java.util.Objects;

import prj.kwui.databinding.ActivityKwuiBinding;

public class KwuiActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    public long aNative;

    private ActivityKwuiBinding binding;

    static public KwuiDummyEdit mTextEdit;

    static public KwuiActivity instance;
    static boolean mScreenKeyboardShown = false;

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

    public static boolean isTextInputEvent(KeyEvent event) {

        // Key pressed with Ctrl should be sent as SDL_KEYDOWN/SDL_KEYUP and not SDL_TEXTINPUT
        if (event.isCtrlPressed()) {
            return false;
        }

        return event.isPrintingKey() || event.getKeyCode() == KeyEvent.KEYCODE_SPACE;
    }

    public static boolean handleKeyEvent(View v, int keyCode, KeyEvent event, InputConnection ic) {
        int deviceId = event.getDeviceId();
        int source = event.getSource();

        if (source == InputDevice.SOURCE_UNKNOWN) {
            InputDevice device = InputDevice.getDevice(deviceId);
            if (device != null) {
                source = device.getSources();
            }
        }

//        if (event.getAction() == KeyEvent.ACTION_DOWN) {
//            Log.v("SDL", "key down: " + keyCode + ", deviceId = " + deviceId + ", source = " + source);
//        } else if (event.getAction() == KeyEvent.ACTION_UP) {
//            Log.v("SDL", "key up: " + keyCode + ", deviceId = " + deviceId + ", source = " + source);
//        }

        // Dispatch the different events depending on where they come from
        // Some SOURCE_JOYSTICK, SOURCE_DPAD or SOURCE_GAMEPAD are also SOURCE_KEYBOARD
        // So, we try to process them as JOYSTICK/DPAD/GAMEPAD events first, if that fails we try them as KEYBOARD
        //
        // Furthermore, it's possible a game controller has SOURCE_KEYBOARD and
        // SOURCE_JOYSTICK, while its key events arrive from the keyboard source
        // So, retrieve the device itself and check all of its sources
        /*
        if (SDLControllerManager.isDeviceSDLJoystick(deviceId)) {
            // Note that we process events with specific key codes here
            if (event.getAction() == KeyEvent.ACTION_DOWN) {
                if (SDLControllerManager.onNativePadDown(deviceId, keyCode) == 0) {
                    return true;
                }
            } else if (event.getAction() == KeyEvent.ACTION_UP) {
                if (SDLControllerManager.onNativePadUp(deviceId, keyCode) == 0) {
                    return true;
                }
            }
        }
        */

        if ((source & InputDevice.SOURCE_MOUSE) == InputDevice.SOURCE_MOUSE) {
            // on some devices key events are sent for mouse BUTTON_BACK/FORWARD presses
            // they are ignored here because sending them as mouse input to SDL is messy
            if ((keyCode == KeyEvent.KEYCODE_BACK) || (keyCode == KeyEvent.KEYCODE_FORWARD)) {
                switch (event.getAction()) {
                    case KeyEvent.ACTION_DOWN:
                    case KeyEvent.ACTION_UP:
                        // mark the event as handled or it will be handled by system
                        // handling KEYCODE_BACK by system will call onBackPressed()
                        return true;
                }
            }
        }

        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            if (isTextInputEvent(event)) {
                if (ic != null) {
                    ic.commitText(String.valueOf((char) event.getUnicodeChar()), 1);
                } else {
                    Native.nCommitText(KwuiActivity.instance.aNative, String.valueOf((char) event.getUnicodeChar()), 1);
                }
            }
            Native.nHandleKeyDown(keyCode);
            return true;
        } else if (event.getAction() == KeyEvent.ACTION_UP) {
            Native.nHandleKeyUp(keyCode);
            return true;
        }

        return false;
    }
    static class ShowTextInputTask implements Runnable {
        /*
         * This is used to regulate the pan&scan method to have some offset from
         * the bottom edge of the input region and the top edge of an input
         * method (soft keyboard)
         */
        static final int HEIGHT_PADDING = 15;

        public int x, y, w, h;

        public ShowTextInputTask(int x, int y, int w, int h) {
            this.x = x;
            this.y = y;
            this.w = w;
            this.h = h;

            /* Minimum size of 1 pixel, so it takes focus. */
            if (this.w <= 0) {
                this.w = 1;
            }
            if (this.h + HEIGHT_PADDING <= 0) {
                this.h = 1 - HEIGHT_PADDING;
            }
        }

        @Override
        public void run() {
            RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(w, h + HEIGHT_PADDING);
            params.leftMargin = x;
            params.topMargin = y;

            if (mTextEdit == null) {
                mTextEdit = new KwuiDummyEdit(instance);

                final ConstraintLayout cl_main = instance.findViewById(R.id.cl_main);
                cl_main.addView(mTextEdit, params);
            } else {
                mTextEdit.setLayoutParams(params);
            }

            mTextEdit.setVisibility(View.VISIBLE);
            mTextEdit.requestFocus();

            InputMethodManager imm = (InputMethodManager)instance.getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.showSoftInput(mTextEdit, 0);

            mScreenKeyboardShown = true;
        }
    }
    static class HideTextInputTask implements Runnable {
        @Override
        public void run() {
            if (mScreenKeyboardShown) {
                mScreenKeyboardShown = false;
                if (mTextEdit != null) {
                    mTextEdit.setVisibility(View.INVISIBLE);
                }
            }
        }
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

    static native void nKeyboardFocusLost();

    static native void nHandleKeyDown(int keyCode);

    static native void nHandleKeyUp(int keyCode);

    static native boolean nSoftReturnKey();

    static native void nCommitText(long obj, String text, int newCursorPosition);

    static native void nGenerateScancodeForUnichar(char c);

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
    public static void jShowTextInput(int x, int y, int w, int h) {
        // Transfer the task to the main thread as a Runnable
        KwuiActivity.instance.runOnUiThread(new KwuiActivity.ShowTextInputTask(x, y, w, h));
    }
    public static void jHideTextInput() {
        KwuiActivity.instance.runOnUiThread(new KwuiActivity.HideTextInputTask());
    }
}