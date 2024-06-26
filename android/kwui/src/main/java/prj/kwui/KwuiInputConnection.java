package prj.kwui;

import android.content.*;
import android.os.Build;
import android.text.Editable;
import android.util.Log;
import android.view.*;
import android.view.inputmethod.BaseInputConnection;
import android.widget.EditText;

public class KwuiInputConnection extends BaseInputConnection
{
    private static final String TAG = "InputConnection";
    protected EditText mEditText;
    protected String mCommittedText = "";

    public KwuiInputConnection(View targetView, boolean fullEditor) {
        super(targetView, fullEditor);
        mEditText = new EditText(KwuiActivity.instance);
    }

    @Override
    public Editable getEditable() {
        return mEditText.getEditableText();
    }

    @Override
    public boolean sendKeyEvent(KeyEvent event) {
        /*
         * This used to handle the keycodes from soft keyboard (and IME-translated input from hardkeyboard)
         * However, as of Ice Cream Sandwich and later, almost all soft keyboard doesn't generate key presses
         * and so we need to generate them ourselves in commitText.  To avoid duplicates on the handful of keys
         * that still do, we empty this out.
         */

        /*
         * Return DOES still generate a key event, however.  So rather than using it as the 'click a button' key
         * as we do with physical keyboards, let's just use it to hide the keyboard.
         */
        Log.i(TAG, String.format("sendKeyEvent %s %d", event.toString(), event.getKeyCode()));
        if (event.getKeyCode() == KeyEvent.KEYCODE_ENTER) {
            if (Native.nSoftReturnKey()) {
                return true;
            }
        }

        return super.sendKeyEvent(event);
    }

    @Override
    public boolean commitText(CharSequence text, int newCursorPosition) {
        Log.i(TAG, String.format("commitText %s %d", text.toString(), newCursorPosition));
        if (!super.commitText(text, newCursorPosition)) {
            return false;
        }
        updateText(newCursorPosition);
        return true;
    }

    @Override
    public boolean setComposingText(CharSequence text, int newCursorPosition) {
        Log.i(TAG, String.format("setComposingText %s %d", text.toString(), newCursorPosition));
        if (!super.setComposingText(text, newCursorPosition)) {
            return false;
        }
        updateText(newCursorPosition);
        return true;
    }

    @Override
    public boolean deleteSurroundingText(int beforeLength, int afterLength) {
        Log.i(TAG, String.format("TODO: deleteSurroundingText %d %d", beforeLength, afterLength));
        return true;
        /*
        if (Build.VERSION.SDK_INT <= 29) { // Android 10.0 (Q)
            // Workaround to capture backspace key. Ref: http://stackoverflow.com/questions>/14560344/android-backspace-in-webview-baseinputconnection
            // and https://bugzilla.libsdl.org/show_bug.cgi?id=2265
            if (beforeLength > 0 && afterLength == 0) {
                // backspace(s)
                while (beforeLength-- > 0) {
                    Native.nGenerateScancodeForUnichar('\b');
                }
                return true;
           }
        }
        if (!super.deleteSurroundingText(beforeLength, afterLength)) {
            return false;
        }
        updateText(0);
        return true;
        */
    }

    protected void updateText(int newCursorPosition) {
        final Editable content = getEditable();
        if (content == null) {
            return;
        }

        String text = content.toString();
        content.clear();
        Native.nCommitText(KwuiActivity.instance.aNative, text, newCursorPosition);
        mCommittedText = text;

        int compareLength = Math.min(text.length(), mCommittedText.length());
        int matchLength, offset;

        /* Backspace over characters that are no longer in the string */
        for (matchLength = 0; matchLength < compareLength; ) {
            int codePoint = mCommittedText.codePointAt(matchLength);
            if (codePoint != text.codePointAt(matchLength)) {
                break;
            }
            matchLength += Character.charCount(codePoint);
        }
        /* FIXME: This doesn't handle graphemes, like '🌬️' */
        for (offset = matchLength; offset < mCommittedText.length(); ) {
            int codePoint = mCommittedText.codePointAt(offset);
            Native.nGenerateScancodeForUnichar('\b');
            offset += Character.charCount(codePoint);
        }

        if (matchLength < text.length()) {
            String pendingText = text.subSequence(matchLength, text.length()).toString();
            for (offset = 0; offset < pendingText.length(); ) {
                int codePoint = pendingText.codePointAt(offset);
                if (codePoint == '\n') {
                    if (Native.nSoftReturnKey()) {
                        return;
                    }
                }
                /* Higher code points don't generate simulated scancodes */
                if (codePoint < 128) {
                    Native.nGenerateScancodeForUnichar((char)codePoint);
                }
                offset += Character.charCount(codePoint);
            }
            Native.nCommitText(KwuiActivity.instance.aNative, pendingText, 0);
        }
        mCommittedText = text;
    }
}

