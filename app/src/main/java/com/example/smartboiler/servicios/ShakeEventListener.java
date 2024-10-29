package com.example.smartboiler.servicios;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.widget.Toast;

public class ShakeEventListener implements SensorEventListener {

    private float mAccel;
    private float mAccelCurrent;
    private float mAccelLast;
    private final SensorManager mSensorManager;
    private final Context context;

    public ShakeEventListener(Context context) {
        this.context = context;
        mAccel = 10f;
        mAccelCurrent = SensorManager.GRAVITY_EARTH;
        mAccelLast = SensorManager.GRAVITY_EARTH;

        mSensorManager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);

        if (mSensorManager != null) {
            mSensorManager.registerListener(this,
                    mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                    SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        float x = event.values[0];
        float y = event.values[1];
        float z = event.values[2];
        mAccelLast = mAccelCurrent;
        mAccelCurrent = (float) Math.sqrt((x * x + y * y + z * z));
        float delta = mAccelCurrent - mAccelLast;
        mAccel = mAccel * 0.9f + delta;

        if (mAccel > 12) {
            Toast.makeText(context, "Shake event detected", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }

    public void unregister() {
        if (mSensorManager != null) {
            mSensorManager.unregisterListener(this);
        }
    }
}
