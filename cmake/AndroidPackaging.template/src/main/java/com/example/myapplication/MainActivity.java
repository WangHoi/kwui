package com.example.myapplication;

import android.os.Bundle;

import prj.kwui.KwuiActivity;

import java.util.Objects;

public class MainActivity extends KwuiActivity {
    private static final String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected String getMainSharedObject() {
        return "libmyapplication.so";
    }
}
