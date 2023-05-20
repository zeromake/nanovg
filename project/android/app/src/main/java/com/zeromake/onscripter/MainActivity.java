package com.zeromake.onscripter;


import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.widget.Toast;

import com.hjq.permissions.OnPermissionCallback;
import com.hjq.permissions.Permission;
import com.hjq.permissions.XXPermissions;

import org.libsdl.app.SDLActivity;

import java.util.List;


public class MainActivity extends SDLActivity implements OnPermissionCallback {
    public static String ARGS_KEY = "args";
    private String[] arguments = new String[]{};
    private String rootPath = null;

    // @Override
    // protected boolean initWindowStyle() {
    //     return true;
    // }
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

    private void externalStoragePermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                XXPermissions.with(this).permission(
                        Permission.MANAGE_EXTERNAL_STORAGE
                ).request(this);
            }
        } else {
            XXPermissions.with(this).permission(
                    Permission.READ_EXTERNAL_STORAGE,
                    Permission.WRITE_EXTERNAL_STORAGE
            ).request(this);
        }
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