package com.android.server.securesim;

/**
 * Interface defining the Binder IPC protocol between SystemUI/App and the Core Service.
 */
interface ISecureSimManager {
    boolean triggerHardwareEject();
}