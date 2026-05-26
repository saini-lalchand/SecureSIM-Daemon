# SecureSIM Daemon

![AOSP](https://img.shields.io/badge/AOSP-System_Module-3DDC84?logo=android)
![Kernel](https://img.shields.io/badge/Linux_Kernel-5.x-FCC624?logo=linux)
![C](https://img.shields.io/badge/C-00599C?logo=c)
![License](https://img.shields.io/badge/License-MIT-yellow)

A system‑level Android security module that prevents physical SIM removal without biometric authentication. This component is integrated into the Linux kernel layer, Android Hardware Abstraction Layer (HAL), and the core Android Framework.

---

## Problem

During device theft, the physical SIM card is typically ejected immediately using a standard pin tool. This action cuts off cellular tracking, remote locking, and data wiping capabilities before any cloud-based security mechanisms can deploy. Android lacks a low-level framework to intercept or prevent this physical vulnerability.

---

## Solution

SecureSIM Daemon implements an architecture for software-controlled electronic SIM latches to replace traditional mechanical trays. 

When a physical tray trigger event occurs, the system intercepts the signal and enforces a biometric or device credential challenge before releasing the latch. Failure to authenticate triggers an immediate system-level lockdown sequence, an acoustic alarm, and an emergency cellular transmission of the last known coordinates. The entire operation executes below the application space, maintaining operation without an active network connection.

---


## Features

- **Biometric SIM Lock:** Prevents physical SIM tray ejection without valid fingerprint, face, or PIN verification.
- **Offline Execution:** Operates entirely at the OS level; no active internet connection is required.
- **Active Threat Response:** Instantly triggers `DevicePolicyManager.lockNow()` and sounds a persistent alarm on failed attempts.
- **SOS Telemetry:** Silently dispatches the device's last known GPS coordinates to a trusted contact via SMS.
- **Tamper-Proof:** Runs in the kernel and system server layers, making it invisible and inaccessible to standard user applications.

---

## Tech Stack

- **Kernel Space:** C (Linux Character Driver, GPIO/Sysfs integration)
- **Hardware Abstraction (HAL):** C++ (JNI/AIDL Native Bridge)
- **Android Framework:** Java (Custom System Service, Binder IPC)
- **User Interface:** Kotlin (BiometricPrompt UI Overlay)
- **Simulation:** Bash (Hardware interrupt simulation scripts)


## System Architecture

This module is built as a native system patch for the Android Open Source Project (AOSP) tree rather than a standard user-space application.

- Kernel Space (`secure_sim_driver.c`): A Linux character driver regulating a GPIO-based magnetic latch mechanism, restricted to CAP_SYS_ADMIN privileges.
- Hardware Abstraction Layer (`secure_sim_hal.cpp`): A C++ native bridge translating AIDL commands into kernel-level IOCTL sequences.
- Android Framework (`SecureSimManagerService`): A core service executing within the system_server process that enforces signature permissions and manages the biometric validation gate.
- UI Layer (`BiometricGateActivity`): A transparent framework window overlaying the device state to process credential confirmation requests.

---

## How it Work (Execution Sequence)

1. The kernel module initializes during the primary boot sequence and maps the electronic latch control register.
2. An physical ejection attempt generates a hardware interrupt via the platform sensor interface.
3. SecureSimManagerService captures the state change over Binder IPC and invokes the BiometricPrompt runtime dialog.
4. On validation success, the HAL writes an activation signal to release the electronic tray latch.
5. On validation failure, the service calls DevicePolicyManager to execute an immediate hardware lock, fires a persistent audio alarm broadcast, and dispatches location telemetry via SMS.

---

## Repository Structure
```text
SecureSIM-Daemon/
├── drivers/
│   └── secure_sim_driver.c          # Linux kernel driver module
├── hal/
│   └── secure_sim_hal.cpp           # Native C++ interface implementation
├── framework/
│   ├── ISecureSimManager.aidl       # Binder IPC interface definition
│   ├── SecureSimManagerService.java # Primary framework execution daemon
│   ├── manifest.xml                 # Signature permission configuration boundaries
│   └── SystemServer_patch.java      # Boot-time service registry hook
├── app/
│   └── BiometricGateActivity.kt     # Client validation bridge implementation
└── README.md
```

---

## Simulated Testing

Hardware implementation can be verified in simulation by writing directly to the sysfs interface to invoke a simulated hardware interrupt sequence:

```
echo 1 > /sys/class/simtray/eject_request
```

This write action forces the underlying system service to process the biometric prompt. If the authorization loop times out or returns a rejection state, the device shifts into lockdown mode.

---

## Engineering Roadmap

- Migration of the existing JNI layer to a modular, Project Treble compliant AIDL HAL structure.
- Implementation of KeyStore hardware protection to require raw biometric token validation directly inside the HAL layer.
- Linking the driver interface to Android power management states to block hardware latch actuation during active cellular connections.
- Integration of a recessed mechanical bypass configuration to allow physical recovery during critical battery depletion or firmware fault states.

---

## Author

Lalchand Saini
System Architecture & Backend Engineering

---

## License

This architecture blueprint is published under the terms of the MIT License.
