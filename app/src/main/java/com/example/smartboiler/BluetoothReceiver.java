package com.example.smartboiler;

import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

public class BluetoothReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();

        if (BluetoothAdapter.ACTION_STATE_CHANGED.equals(action)) {
            int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);

            switch (state) {
                case BluetoothAdapter.STATE_OFF:
                    Toast.makeText(context, "Bluetooth apagado", Toast.LENGTH_SHORT).show();
                    break;

                case BluetoothAdapter.STATE_TURNING_OFF:
                    Toast.makeText(context, "Bluetooth apagándose", Toast.LENGTH_SHORT).show();
                    break;

                case BluetoothAdapter.STATE_ON:
                    Toast.makeText(context, "Bluetooth encendido", Toast.LENGTH_SHORT).show();
                    break;

                case BluetoothAdapter.STATE_TURNING_ON:
                    Toast.makeText(context, "Bluetooth encendiéndose", Toast.LENGTH_SHORT).show();
                    break;

                default:
                    Toast.makeText(context, "Estado desconocido", Toast.LENGTH_SHORT).show();
                    break;
            }
        }
    }
}
