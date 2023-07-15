package com.zeromake.onscripter;


import android.media.AudioManager;
import android.os.Bundle;
import android.widget.Toast;

import com.hjq.permissions.OnPermissionCallback;

import org.libsdl.app.SDLActivity;

import java.util.List;


public class MainActivity extends SDLActivity implements OnPermissionCallback {
    private final String[] arguments = new String[]{};

    @Override
    protected String[] getLibraries() {
        return new String[]{
                "sdl2",
                "example_gles"
        };
    }

    @Override
    protected String[] getArguments() {
        return arguments;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
//        externalStoragePermission();
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        setWindowStyle(true);
    }

    protected void onDestroy() {
        super.onDestroy();
        Globals.CurrentGameRunning = false;
    }

    @Override
    public void onGranted(List<String> permissions, boolean all) {
        Toast.makeText(MainActivity.this, "权限申请成功", Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onDenied(List<String> permissions, boolean never) {
        Toast.makeText(MainActivity.this, "权限申请失败", Toast.LENGTH_SHORT).show();
    }
}
