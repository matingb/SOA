package com.example.smartboiler;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.bluetooth.BluetoothDevice;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import com.example.smartboiler.servicios.ShakeEventListener;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.UUID;

public class SecondActivity extends AppCompatActivity {
    private final static String COMANDO_ENCENDER = "E";
    private final static String COMANDO_APAGAR = "A";
    private final static String COMANDO_CAMBIAR_TEMPERATURA_DESEADA = "T";
    private final static String MENSAJE_AGUA_INSUFICIENTE = "Agua insuficiente";
    private final static String MENSAJE_OFF = "Apagar calendator";
    private final static String MENSAJE_TEMPERATURA_ALCANZADA = "Temperatura deseada alcanzada";
    public static final String TEMPERATURA_DESEADA = "temperatura-deseada";
    private final static int bytesMinimoMensajes = 0;

    Handler handlerBluetoothEmbebido;
    final int handlerState = 0;

    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private final StringBuilder informacionEmbebido = new StringBuilder();

    private ConnectedThread connectedThread;

    private static final UUID identificacionServicioBT = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private static final int REQUEST_BLUETOOTH_PERMISSION = 1;
    private ShakeEventListener shakeEventListener;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        setContentView(R.layout.second_layout);

        Button botonApagar = findViewById(R.id.boton_apagar);

        btAdapter = BluetoothAdapter.getDefaultAdapter();
        handlerBluetoothEmbebido = Handler_Comunicacion_Embebido();
        botonApagar.setOnClickListener(v -> apagar());
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        try {
            if(btSocket != null) {
                btSocket.close();
            }
        } catch (IOException e) {
            Log.e("ERROR", "Error al cerrar el socket: " + e);
        }

        try {
            if(connectedThread != null) {
                connectedThread.interrupt();
            }
        } catch (RuntimeException e) {
            Log.e("ERROR", "Error al interrumpir el thread: " + e);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        shakeEventListener = new ShakeEventListener(this, this::apagar);
        String macAddressEmbebido = "00:22:06:01:8C:86";
        BluetoothDevice deviceEmbebido = btAdapter.getRemoteDevice(macAddressEmbebido);

        try {
            btSocket = crearSocketBluetooth(deviceEmbebido);
            conectarPorBluethoot();
            connectedThread = new ConnectedThread(btSocket);
            connectedThread.start();
            connectedThread.write(COMANDO_ENCENDER);
            setearTemperaturaDeseada();
        } catch (IOException e) {
            mostrarFalloAlEncender();
            volverActivityPrincipal();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        shakeEventListener.unregister();
    }

    private void conectarPorBluethoot() throws IOException {
        String[] permisosNecesarios = {
                android.Manifest.permission.BLUETOOTH_CONNECT,
                android.Manifest.permission.BLUETOOTH,
                android.Manifest.permission.BLUETOOTH_ADMIN,
                android.Manifest.permission.BLUETOOTH_ADVERTISE,
                android.Manifest.permission.BLUETOOTH_SCAN,
                android.Manifest.permission.ACCESS_COARSE_LOCATION
        };

        boolean faltaAlgunPermiso = Arrays.stream(permisosNecesarios)
                .anyMatch(permission -> ActivityCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED);

        if (faltaAlgunPermiso) {
            ActivityCompat.requestPermissions(this, permisosNecesarios, REQUEST_BLUETOOTH_PERMISSION);
        }

        btSocket.connect();
    }

    @SuppressLint("MissingPermission")
    private BluetoothSocket crearSocketBluetooth(BluetoothDevice device) throws IOException {
        return device.createRfcommSocketToServiceRecord(identificacionServicioBT);
    }

    private Handler Handler_Comunicacion_Embebido() {
        return new Handler(Looper.getMainLooper()) {
            public void handleMessage(@NonNull android.os.Message msg) {

                if (existeMensaje(msg)) {
                    String mensaje = (String) msg.obj;
                    informacionEmbebido.append(mensaje);
                    int finDeLinea = informacionEmbebido.indexOf("\r\n");

                    if (finDeLinea > bytesMinimoMensajes) {
                        String mensajeRecibido = informacionEmbebido.substring(0, finDeLinea);
                        TextView textTemperatura = findViewById(R.id.temperatura_actual);

                        if(mensajeRecibido.equals(MENSAJE_AGUA_INSUFICIENTE) || mensajeRecibido.equals(MENSAJE_OFF)) {
                            apagar();
                        } else if(mensajeRecibido.equals(MENSAJE_TEMPERATURA_ALCANZADA)){
                            mostrarToast("Agua lista");
                            apagar();
                        } else {
                            textTemperatura.setText(mensajeRecibido + "°");
                        }
                        informacionEmbebido.delete(0, informacionEmbebido.length());
                    }
                }
            }
        };
    }

    private boolean existeMensaje(@NonNull Message msg) {
        return msg.what == handlerState;
    }

    private void volverActivityPrincipal() {
        Intent intent = new Intent(SecondActivity.this, MainActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
        startActivity(intent);
        finish();
    }

    private void apagar() {
        volverActivityPrincipal();
        if(connectedThread != null) {
            connectedThread.write(COMANDO_APAGAR);
        }
    }

    private void mostrarFalloAlEncender() {
        mostrarToast("Ocurrio un error al conectarse al dispositivo");
    }

    private void mostrarToast(String message) {
        Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
    }

    private void setearTemperaturaDeseada() {
        int temperaturaDeseada = getIntent().getIntExtra(TEMPERATURA_DESEADA, 0);
        connectedThread.write(COMANDO_CAMBIAR_TEMPERATURA_DESEADA + temperaturaDeseada);
    }

    //******************************************** Hilo secundario del Activity**************************************
    private class ConnectedThread extends Thread {
        private final InputStream flujoEntrada;
        private final OutputStream flujoSalida;

        public ConnectedThread(BluetoothSocket socket) {
            InputStream flujoEntradaTemporal = null;
            OutputStream flujoSalidaTemporal = null;

            try {
                flujoEntradaTemporal = socket.getInputStream();
                flujoSalidaTemporal = socket.getOutputStream();
            } catch (IOException e) {
                Log.e("ConnectedThread", "Error al obtener los flujos de entrada y salida", e);
            }

            flujoEntrada = flujoEntradaTemporal;
            flujoSalida = flujoSalidaTemporal;
        }

        public void run() {
            byte[] buffer = new byte[256];
            int bytes;

            while (true) {
                try {
                    bytes = flujoEntrada.read(buffer);
                    if(bytes > 0) {
                        String readMessage = new String(buffer, 0, bytes);
                        handlerBluetoothEmbebido.obtainMessage(handlerState, bytes, -1, readMessage).sendToTarget();
                    }
                } catch (IOException e) {
                    break;
                }
            }
        }

        public void write(String input) {
            byte[] msgBuffer = input.getBytes();
            try {
                flujoSalida.write(msgBuffer);
            } catch (IOException e) {
                mostrarToast("Ocurrio un error inesperado");
                finish();
            }
        }
    }
}