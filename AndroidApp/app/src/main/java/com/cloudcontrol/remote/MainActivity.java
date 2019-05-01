package com.cloudcontrol.remote;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity {

    private boolean recording = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //TODO: Connect wifi to RPI network
        final Button recordingButton = this.findViewById(R.id.button);
        recordingButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Open gallery activity on press
                if (recording) {
                    recordingButton.setText("Start Recording");
                    recording = false;
                    //TODO: start recording
                }
                else {
                    recordingButton.setText("Stop Recording");
                    recording = true;
                    //TODO: stop recording
                }
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        //TODO: make sure wifi is connected to RPI network
    }

    @Override
    protected void onPause() {
        super.onPause();
        final Button recordingButton = this.findViewById(R.id.button);
        recordingButton.setText("Start Recording");
        recording = false;
        //TODO: stop recording
    }
}
