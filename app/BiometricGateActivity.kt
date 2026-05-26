package com.android.systemui.securesim

import android.app.Activity
import android.content.Context
import android.hardware.biometrics.BiometricPrompt // TYPO FIXED
import android.os.Bundle
import android.os.CancellationSignal
import android.os.ServiceManager
import android.util.Log
import android.widget.Toast
import com.android.server.securesim.ISecureSimManager
import java.util.concurrent.Executor
import java.util.concurrent.Executors

class BiometricGateActivity : Activity() {
    private val TAG = "SecureSIM_UI"
    private val executor: Executor = Executors.newSingleThreadExecutor()
    private val cancellationSignal = CancellationSignal()
    private var secureSimService: ISecureSimManager? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.setBackgroundDrawableResource(android.R.color.transparent)
        
        // INTERPROCESS COMMUNICATION FIXED: Bind to the System Service via the ServiceManager token
        val binder = ServiceManager.getService("secure_sim")
        if (binder != null) {
            secureSimService = ISecureSimManager.Stub.asInterface(binder)
            triggerBiometricVerification()
        } else {
            Log.e(TAG, "Critical IPC Failure: SecureSimManagerService daemon not found in ServiceManager registry.")
            Toast.makeText(this, "System Framework Error", Toast.LENGTH_SHORT).show()
            finish()
        }
    }

    private fun triggerBiometricVerification() {
        val biometricPrompt = BiometricPrompt.Builder(this)
            .setTitle("Identity Verification Required")
            .setSubtitle("Confirm device ownership to release/eject physical SIM tray mechanism.")
            .setNegativeButton("Cancel", executor) { _, _ -> finish() }
            .build()

        biometricPrompt.authenticate(cancellationSignal, executor, object : BiometricPrompt.AuthenticationCallback() {
            override fun onAuthenticationError(errorCode: Int, errString: CharSequence?) {
                super.onAuthenticationError(errorCode, errString)
                runOnUiThread {
                    Toast.makeText(applicationContext, "Auth Blocked: $errString", Toast.LENGTH_SHORT).show()
                    finish()
                }
            }

            override fun onAuthenticationSucceeded(result: BiometricPrompt.AuthenticationResult?) {
                super.onAuthenticationSucceeded(result)
                runOnUiThread {
                    val status = callSystemEjectService()
                    if (status) {
                        Toast.makeText(applicationContext, "Latch Released Successfully.", Toast.LENGTH_LONG).show()
                    } else {
                        Toast.makeText(applicationContext, "Hardware Error: Latch deployment failed.", Toast.LENGTH_LONG).show()
                    }
                    finish()
                }
            }
        })
    }

    private fun callSystemEjectService(): Boolean {
        return try {
            Log.d(TAG, "Invoking AIDL interface pipeline mapping...")
            secureSimService?.triggerHardwareEject() ?: false
        } catch (e: Exception) {
            Log.e(TAG, "IPC Transaction dropped during secure service validation.", e)
            false
        }
    }
}