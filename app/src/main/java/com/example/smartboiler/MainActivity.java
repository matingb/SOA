package com.example.smartboiler;

import static java.lang.Thread.sleep;

import android.os.Bundle;
import androidx.core.splashscreen.SplashScreen;

import androidx.appcompat.app.AppCompatActivity;
public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        SplashScreen splashScreen = SplashScreen.installSplashScreen(this);

        //solo porque no pude demorar la pantalla de carga, para que
        // al menos se vea algo lo deje asi a modo de prueba. despues lo borramos
        try {
            sleep(2000);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        splashScreen.setKeepOnScreenCondition(() -> false);
    }
}