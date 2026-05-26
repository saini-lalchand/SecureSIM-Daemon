package com.android.server.securesim;

import android.content.Context;
import android.os.Binder;
import android.os.RemoteException;
import android.util.Log;

/**
 * Fully compliant Android System Service implementing the AIDL protocol.
 * To integrate with AOSP: Register via ServiceManager.addService("secure_sim", new SecureSimManagerService(context)) inside SystemServer.java.
 */
public class SecureSimManagerService extends ISecureSimManager.Stub {
    private static final String TAG = "SecureSimManagerService";
    private static final String PERMISSION_CHECK = "android.permission.MANAGE_SECURE_SIM";
    private final Context mContext;

    static {
        try {
            System.loadLibrary("securesim_hal");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Critical Security Error: Native HAL binaries failed to load.", e);
        }
    }

    public SecureSimManagerService(Context context) {
        this.mContext = context;
        Log.i(TAG, "SecureSimManagerService Core Subsystem Mounted.");
    }

    @Override
    public boolean triggerHardwareEject() throws RemoteException {
        // Enforce system-level Signature|Privileged permissions to prevent unauthenticated apps
        mContext.enforceCallingOrSelfPermission(PERMISSION_CHECK, "Access Denied: Process lacks MANAGE_SECURE_SIM permission.");
        
        Log.d(TAG, "Binder IPC Authentication Verified. Dispatching request to Native JNI Layer...");
        
        try {
            return nativeEjectSimTray();
        } catch (Exception e) {
            Log.e(TAG, "Failsafe Intercepted: Runtime exception inside JNI/HAL layer.", e);
            return false;
        }
    }

    private native boolean nativeEjectSimTray();
}