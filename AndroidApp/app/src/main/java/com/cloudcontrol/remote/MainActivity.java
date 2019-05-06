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

import android.app.Activity;
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
    private Thread sendingThread = null;
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
                if (!recording) {
                    recordingButton.setText("Stop Recording");
                    recordingButton.setCompoundDrawablesWithIntrinsicBounds(null, mContext.getResources().getDrawable(R.drawable.square), null, null);
                    startRecording();
                }
                else {
                    recordingButton.setText("Start Recording");
                    recordingButton.setCompoundDrawablesWithIntrinsicBounds(null, mContext.getResources().getDrawable(R.drawable.circle), null, null);
                    stopRecording();
                }
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
        final Button recordingButton = this.findViewById(R.id.button);
        recordingButton.setText("Start Recording");
        stopRecording();
        try {
            gcsUplink.setText("GCS Uplink: Down");
            gcsUplink.setTextColor(Color.RED);
            socket.getOutputStream().write("15".getBytes());
            socket.close();
        }
        catch (IOException e) {}
    }

    private void startRecording() {
        //TODO: implement using microphone
        recording = true;
        try{
            socket.getOutputStream().write("10".getBytes());
        }
        catch (IOException e) {}
    }

    private void stopRecording() {
        recording = false;
        try{
            socket.getOutputStream().write("11".getBytes());
        }
        catch (IOException e) {}
    }

    private void receiveUpdates() {
        while (true) {
            try {
                dataIn = new DataInputStream(socket.getInputStream());
                String serverMessage = dataIn.readLine();
                switch (serverMessage.substring(0,2)) {
                    case "00":
                        recording = true;
                        recordingButton.post(new Runnable() {
                            @Override
                            public void run() {
                                recordingButton.setText("Stop Recording");
                                recordingButton.setCompoundDrawablesWithIntrinsicBounds(null, mContext.getResources().getDrawable(R.drawable.square), null, null);
                            }
                        });
                        break;
                    case "01":
                        recording = false;
                        recordingButton.post(new Runnable() {
                            @Override
                            public void run() {
                                recordingButton.setText("Start Recording");
                                recordingButton.setCompoundDrawablesWithIntrinsicBounds(null, mContext.getResources().getDrawable(R.drawable.circle), null, null);
                            }
                        });
                        break;
                    case "02":
                        if (serverMessage.charAt(2) == '1') {
                            droneUplink.post(new Runnable() {
                                @Override
                                public void run() {
                                    droneUplink.setText("Drone Uplink: Up");
                                    droneUplink.setTextColor(Color.rgb(0,100,0));
                                }
                            });
                        }
                        else {
                            droneUplink.post(new Runnable() {
                                @Override
                                public void run() {
                                    droneUplink.setText("Drone Uplink: Down");
                                    droneUplink.setTextColor(Color.RED);
                                }
                            });
                        }
                        break;
                    case "03":
                        final String count = serverMessage.substring(2, serverMessage.length()-1);
                        failedPackets.post(new Runnable() {
                            @Override
                            public void run() {
                                failedPackets.setText("Failed Packets: " + count);
                            }
                        });
                        break;
                    case "04":
                        Log.d("WIFI", "Connected to GCS!!");
                        gcsUplink.post(new Runnable() {
                            @Override
                            public void run() {
                                gcsUplink.setText("GCS Uplink: Up");
                                gcsUplink.setTextColor(Color.rgb(0,100,0));
                            }
                        });
                        break;
                    default:
                        throw new IOException();
                }
//                Log.d("WIFI", "Message received: " + serverMessage);
            }
            catch (Exception e) {
                Log.d("WIFI", "couldn't receive packet");
                gcsUplink.post(new Runnable() {
                    @Override
                    public void run() {
                        gcsUplink.setText("GCS Uplink: Down");
                        gcsUplink.setTextColor(Color.RED);
                    }
                });
                droneUplink.post(new Runnable() {
                    @Override
                    public void run() {
                        droneUplink.setText("Drone Uplink: Down");
                        droneUplink.setTextColor(Color.RED);
                    }
                });
                break;
            }
        }
        try {
            Log.d("WIFI", "closing");
            gcsUplink.post(new Runnable() {
                @Override
                public void run() {
                    gcsUplink.setText("GCS Uplink: Down");
                    gcsUplink.setTextColor(Color.RED);
                }
            });
            droneUplink.post(new Runnable() {
                @Override
                public void run() {
                    droneUplink.setText("Drone Uplink: Down");
                    droneUplink.setTextColor(Color.RED);
                }
            });
            socket.close();
        }
        catch (Exception e) {}
        return;
    }

    private void showAlert(final String title, final String message) {
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
        receivingThread = new Thread(new Runnable() {
            public void run() {
                int connectTries = 0;
                while (true) {
                    try {
                        socket = new Socket(SERVER_IP, SERVER_PORT);
                        Log.d("WIFI", "Connected to RPI");
                        gcsUplink.post(new Runnable() {
                            @Override
                            public void run() {
                                gcsUplink.setText("GCS Uplink: Up");
                                gcsUplink.setTextColor(Color.rgb(0,100,0));
                            }
                        });
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
