package com.example.smartboiler.servicios;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

import java.time.Duration;
import java.time.LocalDateTime;

public class ShakeEventListener implements SensorEventListener {

    private float aceleracionFiltrada;
    private float aceleracionTotal;
    private float aceleracionAnterior;
    private final SensorManager sensorManager;
    private final Runnable onShakeAction;
    private LocalDateTime fechaUltimoShake = LocalDateTime.now();
    private final float VALOR_PARTIDA_ACELERACION_FILTRADA = 10f;
    private final float FACTOR_SUAVIZADOR_FILTRO = 0.9f;
    private final float UMBRAL_ACELERACION_MINIMA = 12;
    private final long TIEMPO_MINIMO_ENTRE_SHAKES = 2;

    public ShakeEventListener(Context context, Runnable onShakeAction) {
        this.onShakeAction = onShakeAction;
        aceleracionFiltrada = VALOR_PARTIDA_ACELERACION_FILTRADA;
        aceleracionTotal = SensorManager.GRAVITY_EARTH;
        aceleracionAnterior = SensorManager.GRAVITY_EARTH;

        sensorManager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);

        if (sensorManager != null) {
            sensorManager.registerListener(this,
                    sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
                    SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        float x = event.values[0];
        float y = event.values[1];
        float z = event.values[2];
        aceleracionAnterior = aceleracionTotal;
        aceleracionTotal = (float) Math.sqrt((x * x + y * y + z * z));
        float diferenciaAceleraciones = aceleracionTotal - aceleracionAnterior;
        aceleracionFiltrada = aceleracionFiltrada * FACTOR_SUAVIZADOR_FILTRO + diferenciaAceleraciones;

        long diferenciaSegundos = Duration.between(fechaUltimoShake, LocalDateTime.now()).getSeconds();

        if (aceleracionFiltrada > UMBRAL_ACELERACION_MINIMA && diferenciaSegundos >= TIEMPO_MINIMO_ENTRE_SHAKES) {
            onShakeAction.run();
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }

    public void unregister() {
        if (sensorManager != null) {
            sensorManager.unregisterListener(this);
        }
    }
}
