//Reed Taylor -- CloudControl (UCSB Senior Capstone Project) This app is a remote for the GCS and is being developed for credit in ECE 150.
// TCP codes:
/*
X0: start recording
X1: stop recording
02: uplink status
03: failed packets
04: tablet connected
15: tablet disconnecting
first digit: 1 for tablet, 0 for drone
 */

package com.cloudcontrol.remote;

import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Color;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;

public class MainActivity extends AppCompatActivity {
    private static final int RECORDER_SAMPLERATE = 12000;
    private static final int RECORDER_CHANNELS = AudioFormat.CHANNEL_IN_MONO;
    private static final int RECORDER_AUDIO_ENCODING = AudioFormat.ENCODING_PCM_16BIT;
    private AudioRecord recorder = null;
    private Thread recordingThread = null;
    private Thread receivingThread = null;
    private boolean recording = false;
    public static final String SERVER_IP = "192.168.42.1"; //server IP address
    public static final int SERVER_PORT = 12001;
    private Socket socket = null;
    private DataInputStream dataIn;
    private DataOutputStream dataOut;
    private Button recordingButton;
    private TextView gcsUplink;
    private TextView droneUplink;
    private TextView failedPackets;

    private final Context mContext = this;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        recordingButton = this.findViewById(R.id.button);
        gcsUplink = this.findViewById(R.id.textView);
        gcsUplink.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                connectToWifi();
            }
        });
        droneUplink = this. findViewById(R.id.textView2);
        failedPackets = this. findViewById(R.id.textView3);
        recordingButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!recording)
                    startRecording(true);
                else
                    stopRecording(true);
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        //Connecting Tablet to GCS network
        connectToWifi();
    }

    @Override
    protected void onPause() {
        super.onPause();
        //Stop recording nd send disconnect signal
        stopRecording(true);
        try {
            gcsUplink.setText("GCS Uplink: Down");
            gcsUplink.setTextColor(Color.RED);
            socket.getOutputStream().write("15".getBytes());
            socket.close();
        }
        catch (IOException e) {}
    }

    private void startRecording(Boolean tablet) {
        // Changes UI for recording and sends recording signal if originating from tablet
        //TODO: implement using microphone
        try{
            if (tablet)
                socket.getOutputStream().write("10".getBytes());
            recording = true;
            recordingButton.post(new Runnable() {
                @Override
                public void run() {
                    recordingButton.setText("Stop Recording");
                    recordingButton.setCompoundDrawablesWithIntrinsicBounds(null, mContext.getResources().getDrawable(R.drawable.square), null, null);
                }
            });
        }
        catch (IOException e) {}
    }

    private void stopRecording(Boolean tablet) {
        // Changes UI for recording and sends recording signal if originating from tablet
        try{
            if (tablet)
                socket.getOutputStream().write("11".getBytes());
            recording = false;
            recordingButton.post(new Runnable() {
                @Override
                public void run() {
                    recordingButton.setText("Start Recording");
                    recordingButton.setCompoundDrawablesWithIntrinsicBounds(null, mContext.getResources().getDrawable(R.drawable.circle), null, null);
                }
            });
        }
        catch (IOException e) {}
    }

    private void setDroneUplink(final Boolean up) {
        //Update UI for Drone uplink status
        droneUplink.post(new Runnable() {
            @Override
            public void run() {
                if (up) {
                    droneUplink.setText("Drone Uplink: Up");
                    droneUplink.setTextColor(Color.rgb(0,100,0));
                }
                else {
                    droneUplink.setText("Drone Uplink: Down");
                    droneUplink.setTextColor(Color.RED);
                }
            }
        });
    }

    private void setGcsUplink(final Boolean up) {
        //Update UI for GCS uplink status
        gcsUplink.post(new Runnable() {
            @Override
            public void run() {
                if (up) {
                    gcsUplink.setText("GCS Uplink: Up");
                    gcsUplink.setTextColor(Color.rgb(0,100,0));
                }
                else {
                    gcsUplink.setText("GCS Uplink: Down");
                    gcsUplink.setTextColor(Color.RED);
                }
            }
        });
    }

    private void setFailedPackets(final String count) {
        //Update UI for failed packet count
        failedPackets.post(new Runnable() {
            @Override
            public void run() {
                failedPackets.setText("Failed Packets: " + count);
            }
        });
    }

    private void receiveUpdates() {
        //Listener for receiving messages from GCS
        while (true) {
            try {
                dataIn = new DataInputStream(socket.getInputStream());
                String serverMessage = dataIn.readLine();
                switch (serverMessage.substring(0,2)) {
                    case "00":
                        startRecording(false);
                        break;
                    case "01":
                        stopRecording(false);
                        break;
                    case "02": //update drone status
                        if (serverMessage.charAt(2) == '1')
                            setDroneUplink(true);
                        else
                            setDroneUplink(false);
                        break;
                    case "03":
                        setFailedPackets(serverMessage.substring(2, serverMessage.length()-1));
                        break;
                    case "04":
                        Log.d("WIFI", "Connected to GCS!!");
                        setGcsUplink(true);
                        break;
                    default:
                        throw new IOException();
                }
            }
            catch (Exception e) {
                Log.d("WIFI", "couldn't receive packet");
                setGcsUplink(false);
                setDroneUplink(false);
                break;
            }
        }
        try {
            Log.d("WIFI", "closing");
            setGcsUplink(false);
            setDroneUplink(false);
            socket.close();
        }
        catch (Exception e) {}
        return;
    }

    private void showAlert(final String title, final String message) {
        //Show failed alert that restarts wifi setup on continue
        MainActivity.this.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                new AlertDialog.Builder(mContext)
                    .setTitle(title)
                    .setMessage(message)
                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) { connectToWifi(); }})
                    .show();
            }
        });
    }

    private void connectToWifi() {
        // Logic for connecting to WIFI network on RPi
        WifiManager wifiManager = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        WifiConfiguration cgsConfig = new WifiConfiguration();
        if (wifiManager.getConnectionInfo().getSSID().equals("\"CloudControlGCS\"") && wifiManager.getConnectionInfo().getSupplicantState().toString() == "COMPLETED") {
            startTCP();
        }
        else {
            cgsConfig.SSID = "\"CloudControlGCS\"";
            cgsConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
            int netId = wifiManager.addNetwork(cgsConfig);
            wifiManager.disconnect();
            wifiManager.enableNetwork(netId, true);
            wifiManager.reconnect();
            int retryCount = 0;
            while (!(wifiManager.getConnectionInfo().getSSID().equals("\"CloudControlGCS\"")) || wifiManager.getConnectionInfo().getRssi() == -200) {
                if (retryCount++ > 13000) {
                    showAlert("Cannot Connect to CGS WiFi", "Make sure CGS is turned on.");
                    break;
                }
            }
            if (wifiManager.getConnectionInfo().getSSID().equals("\"CloudControlGCS\"")) {
                Log.d("WIFI", "Strength:" + wifiManager.getConnectionInfo().getRssi());
                Log.d("WIFI", "Connected to CloudControlGCS");
                retryCount = 0;
                while (wifiManager.getConnectionInfo().getSupplicantState().toString() != "COMPLETED") {
                    if (retryCount++ > 10000) {
                        showAlert("Cannot Connect to CGS WiFi", "Unable to reach completed connection state.");
                        return;
                    }
                }
                Log.d("WIFI", wifiManager.getConnectionInfo().getSupplicantState().toString());
                Handler delayHandler = new Handler();
                Runnable r = new Runnable() {
                    @Override
                    public void run() {
                        startTCP();
                    }
                };
                delayHandler.postDelayed(r, 1000);
            }
            else
                showAlert("Cannot Connect to CGS WiFi", "Did not see network.");
        }
    }

    private void startTCP() {
        //Initialize receiving thread
        receivingThread = new Thread(new Runnable() {
            public void run() {
                int connectTries = 0;
                while (true) {
                    try {
                        socket = new Socket(SERVER_IP, SERVER_PORT);
                        Log.d("WIFI", "Connected to RPI");
                        setGcsUplink(true);
                        break;
                    } catch (IOException e) {
                        if (connectTries++ > 10) {
                            Log.d("WIFI", "Reached TCP try limit");
                            showAlert("TCP Connection failed", "Reached TCP try limit");
                            break;
                        }
                    }
                }
                receiveUpdates();
            }
        }, "Receiving Thread");
        receivingThread.start();
    }

}
