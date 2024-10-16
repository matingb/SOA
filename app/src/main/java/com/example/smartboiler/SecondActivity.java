package com.example.smartboiler;

import static java.lang.Thread.sleep;

import android.content.Context;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.splashscreen.SplashScreen;

import java.util.Objects;

public class SecondActivity extends AppCompatActivity {

    public static final String TEMPERATURA_DESEADA = "temperatura-deseada";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.second_layout);

        int temperaturaDeseada = getIntent().getIntExtra(TEMPERATURA_DESEADA, 0);
        TextView textView = findViewById(R.id.temperatura_actual);
        textView.setText(String.valueOf(temperaturaDeseada));

        Button botonApagar = findViewById(R.id.boton_apagar);

        botonApagar.setOnClickListener(v -> {
            Intent intent = new Intent(SecondActivity.this, MainActivity.class);
            startActivity(intent);
        });
    }
}