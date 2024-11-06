package com.example.smartboiler;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.core.app.ActivityCompat;
import androidx.core.splashscreen.SplashScreen;

import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;


import androidx.appcompat.app.AppCompatActivity;

import com.example.smartboiler.servicios.ShakeEventListener;

public class MainActivity extends AppCompatActivity {
    private static final int TEMPERATURA_CAFE = 90;
    private static final int TEMPERATURA_MATE = 75;
    private ShakeEventListener shakeEventListener;




    @RequiresApi(api = Build.VERSION_CODES.S)
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        SplashScreen splashScreen = SplashScreen.installSplashScreen(this);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        splashScreen.setKeepOnScreenCondition(() -> false);

        SeekBar seekBar = findViewById(R.id.seekBar);
        TextView temperatureText = findViewById(R.id.accion_encender).findViewById(R.id.temperature);

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                temperatureText.setText(getString(R.string.formato_accion_temperatura, progress));
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        configurarAccion(R.id.accion_encender, R.drawable.custom, "Encender", seekBar.getProgress(), v -> {
            int temperaturaSlider = seekBar.getProgress();
            lanzarSegundaActividad(temperaturaSlider);
        });
        configurarAccion(R.id.accion_cafe, R.drawable.cafe, "Café", TEMPERATURA_CAFE, v -> lanzarSegundaActividad(TEMPERATURA_CAFE));
        configurarAccion(R.id.accion_mate, R.drawable.mate, "Mate", TEMPERATURA_MATE, v -> lanzarSegundaActividad(TEMPERATURA_MATE));
    }

    private void configurarAccion(int accionId, int iconResId, String texto, int temperaturaInicial, View.OnClickListener onClickListener) {
        ConstraintLayout accionLayout = findViewById(accionId);
        ImageView icono = accionLayout.findViewById(R.id.icon);
        TextView textoView = accionLayout.findViewById(R.id.label);
        TextView temperaturaView = accionLayout.findViewById(R.id.temperature);
        ImageView botonPlay = accionLayout.findViewById(R.id.button);

        icono.setImageResource(iconResId);
        textoView.setText(texto);
        temperaturaView.setText(getString(R.string.formato_accion_temperatura, temperaturaInicial));

        botonPlay.setOnClickListener(onClickListener);
    }

    private void lanzarSegundaActividad(int temperatura) {
        Intent intent = new Intent(MainActivity.this, SecondActivity.class);
        intent.putExtra(SecondActivity.TEMPERATURA_DESEADA, temperatura);
        startActivity(intent);
    }

    @Override
    protected void onResume() {
        super.onResume();
        shakeEventListener = new ShakeEventListener(this);
    }

    @Override
    protected void onPause() {
        super.onPause();
        shakeEventListener.unregister();
    }
}
