package com.example.sphota.accelerometer_test;

import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        new LongOperation().execute();

        super.onCreate(savedInstanceState);

    }

    private class LongOperation extends AsyncTask<String, Void, String> {
        @Override
        protected String doInBackground(String... params) {
            float[][] accel = init();
            for (int i = 0; i < 100; i++) {
                Log.d("XYZ", accel[i][0] + " " + accel[i][1] + " " + accel[i][2] + " " + accel[i][3]);
            }
            return null;
        }
    }

    public native float[][] init();
}
