package com.example.smartboiler;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
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
    public static final String TEMPERATURA_DESEADA = "temperatura-deseada";


    Handler bluetoothIn;
    final int handlerState = 0; //used to identify handler message

    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private final StringBuilder recDataString = new StringBuilder();

    private ConnectedThread mConnectedThread;

    private static final UUID BTMODULEUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    private static final int REQUEST_BLUETOOTH_PERMISSION = 1;
    private ShakeEventListener shakeEventListener;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        setContentView(R.layout.second_layout);

        Button botonApagar = findViewById(R.id.boton_apagar);

        btAdapter = BluetoothAdapter.getDefaultAdapter();
        bluetoothIn = Handler_Msg_Hilo_Principal();
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
            if(mConnectedThread != null) {
                mConnectedThread.interrupt();
            }
        } catch (RuntimeException e) {
            Log.e("ERROR", "Error al interrumpir el thread: " + e);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        shakeEventListener = new ShakeEventListener(this, this::apagar);
        String macAddress = "00:22:06:01:8C:86";
        BluetoothDevice device = btAdapter.getRemoteDevice(macAddress);

        try {
            btSocket = crearSocketBluetooth(device);
            contectarPorBluethoot();
            mConnectedThread = new ConnectedThread(btSocket);
            mConnectedThread.start();
            mConnectedThread.write(COMANDO_ENCENDER);
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

    private void contectarPorBluethoot() throws IOException {
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
        return device.createRfcommSocketToServiceRecord(BTMODULEUUID);
    }

    //Handler que permite mostrar datos en el Layout al hilo secundario
    private Handler Handler_Msg_Hilo_Principal() {
        return new Handler(Looper.getMainLooper()) {
            public void handleMessage(@NonNull android.os.Message msg) {
                if (existeMensaje(msg)) {
                    String mensaje = (String) msg.obj;
                    recDataString.append(mensaje);
                    int finDeLinea = recDataString.indexOf("\r\n");

                    if (finDeLinea > 0) {
                        String dataInPrint = recDataString.substring(0, finDeLinea);
                        TextView textTemperatura = findViewById(R.id.temperatura_actual);

                        if (dataInPrint.equals(MENSAJE_AGUA_INSUFICIENTE) || dataInPrint.equals(MENSAJE_OFF)) {
                            apagar();
                        }
                        else
                            textTemperatura.setText(dataInPrint + "°");
                        recDataString.delete(0, recDataString.length());
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
        if(mConnectedThread != null) {
            mConnectedThread.write(COMANDO_APAGAR);
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
        mConnectedThread.write(COMANDO_CAMBIAR_TEMPERATURA_DESEADA + temperaturaDeseada);
    }

    //******************************************** Hilo secundario del Activity**************************************
    //*************************************** recibe los datos enviados por el HC05**********************************

    private class ConnectedThread extends Thread {
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
            }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        //metodo run del hilo, que va a entrar en una espera activa para recibir los msjs del HC05
        public void run() {
            byte[] buffer = new byte[256];
            int bytes;

            while (true) {
                try {
                    bytes = mmInStream.read(buffer);
                    String readMessage = new String(buffer, 0, bytes);
                    bluetoothIn.obtainMessage(handlerState, bytes, -1, readMessage).sendToTarget();
                } catch (IOException e) {
                    break;
                }
            }
        }

        public void write(String input) {
            byte[] msgBuffer = input.getBytes();
            try {
                mmOutStream.write(msgBuffer);
            } catch (IOException e) {
                mostrarToast("Ocurrio un error inesperado");
                finish();
            }
        }
    }
}