#include <jni.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <android/log.h>

#define LOG_TAG "SecureSIM_HAL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define SIM_DRIVER_PATH "/dev/secure_sim_latch"
#define SIM_IOCTL_CMD_EJECT 0x101

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_android_server_securesim_SecureSimManagerService_nativeEjectSimTray(JNIEnv *env, jobject thiz) {
    LOGI("Native HAL wrapper invoked via system IPC callback.");
    
    int fd = open(SIM_DRIVER_PATH, O_WRONLY);
    if (fd < 0) {
        LOGE("System Critical Error: Cannot interface with /dev/secure_sim_latch. Driver unavailable.");
        return JNI_FALSE;
    }

    // BUG FIX: Passed explicit '0' as the 3rd argument to prevent Undefined Behavior (UB) in kernel space
    unsigned long dummy_arg = 0;
    int result = ioctl(fd, SIM_IOCTL_CMD_EJECT, dummy_arg);
    close(fd);

    if (result < 0) {
        LOGE("Hardware Error: IOCTL execution rejected inside Kernel Space.");
        return JNI_FALSE;
    }

    LOGI("Hardware Ejection command parsed and piped to driver subsystem successfully.");
    return JNI_TRUE;
}

}