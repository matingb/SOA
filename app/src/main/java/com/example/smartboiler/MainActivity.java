package com.example.smartboiler;

import static java.lang.Thread.sleep;

import android.content.Intent;
import android.os.Bundle;

import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.core.splashscreen.SplashScreen;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import java.util.Objects;

import androidx.appcompat.app.AppCompatActivity;
public class MainActivity extends AppCompatActivity {
    private SensorManager mSensorManager;
    private float mAccel;
    private float mAccelCurrent;
    private float mAccelLast;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        SplashScreen splashScreen = SplashScreen.installSplashScreen(this);

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        splashScreen.setKeepOnScreenCondition(() -> false);
        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        Objects.requireNonNull(mSensorManager).registerListener(mSensorListener, mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                SensorManager.SENSOR_DELAY_NORMAL);
        mAccel = 10f;
        mAccelCurrent = SensorManager.GRAVITY_EARTH;
        mAccelLast = SensorManager.GRAVITY_EARTH;

        configurarAccion(
                findViewById(R.id.accion_encender),
                R.drawable.custom,
                "Encender",
                "50º",
                v -> {
                    Intent intent = new Intent(MainActivity.this, SecondActivity.class);
                    intent.putExtra(SecondActivity.TEMPERATURA_DESEADA, 50);
                    startActivity(intent);
                }
        );

        configurarAccion(
                findViewById(R.id.accion_cafe),
                R.drawable.cafe,
                "Café",
                "90º",
                v -> {
                    Intent intent = new Intent(MainActivity.this, SecondActivity.class);
                    intent.putExtra(SecondActivity.TEMPERATURA_DESEADA, 90);
                    startActivity(intent);
                }
        );

        configurarAccion(
                findViewById(R.id.accion_mate),
                R.drawable.mate,
                "Mate",
                "75º",
                v -> {
                    Intent intent = new Intent(MainActivity.this, SecondActivity.class);
                    intent.putExtra(SecondActivity.TEMPERATURA_DESEADA, 75);
                    startActivity(intent);
                }
        );
    }

    private void configurarAccion(ConstraintLayout accionLayout, int iconResId, String texto, String temperatura, View.OnClickListener listener) {
        ImageView icono = accionLayout.findViewById(R.id.icon);
        TextView textoView = accionLayout.findViewById(R.id.label);
        TextView temperaturaView = accionLayout.findViewById(R.id.temperature);

        icono.setImageResource(iconResId);
        textoView.setText(texto);
        temperaturaView.setText(temperatura);

        ImageView botonPlay = accionLayout.findViewById(R.id.button);
        botonPlay.setOnClickListener(listener);
    }

    private final SensorEventListener mSensorListener = new SensorEventListener() {
        @Override
        public void onSensorChanged(SensorEvent event) {
            float x = event.values[0];
            float y = event.values[1];
            float z = event.values[2];
            mAccelLast = mAccelCurrent;
            mAccelCurrent = (float) Math.sqrt((double) (x * x + y * y + z * z));
            float delta = mAccelCurrent - mAccelLast;
            mAccel = mAccel * 0.9f + delta;
            if (mAccel > 12) {
                Toast.makeText(getApplicationContext(), "Shake event detected", Toast.LENGTH_SHORT).show();
            }
        }
        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {
        }
    };
    @Override
    protected void onResume() {
        mSensorManager.registerListener(mSensorListener, mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),SensorManager.SENSOR_DELAY_NORMAL);
        super.onResume();
    }
    @Override
    protected void onPause() {
        mSensorManager.unregisterListener(mSensorListener);
        super.onPause();
    }
}