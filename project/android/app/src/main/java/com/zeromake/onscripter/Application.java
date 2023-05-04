package com.zeromake.onscripter;

public class Application extends android.app.Application {
    private static Application _app;
    public static android.app.Application getInstance() {
        return _app;
    }
    @Override
    public void onCreate() {
        super.onCreate();
        _app = this;
    }
}