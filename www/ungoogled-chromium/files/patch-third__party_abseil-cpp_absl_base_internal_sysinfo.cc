--- third_party/abseil-cpp/absl/base/internal/sysinfo.cc.orig	2023-08-18 10:26:52 UTC
+++ third_party/abseil-cpp/absl/base/internal/sysinfo.cc
@@ -30,10 +30,14 @@
 #include <sys/syscall.h>
 #endif
 
-#if defined(__APPLE__) || defined(__FreeBSD__)
+#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
 #include <sys/sysctl.h>
 #endif
 
+#if defined(__FreeBSD__)
+#include <pthread_np.h>
+#endif
+
 #if defined(__myriad2__)
 #include <rtems.h>
 #endif
@@ -186,6 +190,7 @@ static double GetNominalCPUFrequency() {
 
 #else
 
+#if !defined(__OpenBSD__) && !defined(__FreeBSD__)
 // Helper function for reading a long from a file. Returns true if successful
 // and the memory location pointed to by value is set to the value read.
 static bool ReadLongFromFile(const char *file, long *value) {
@@ -212,6 +217,7 @@ static bool ReadLongFromFile(const char *file, long *v
   }
   return ret;
 }
+#endif
 
 #if defined(ABSL_INTERNAL_UNSCALED_CYCLECLOCK_FREQUENCY_IS_CPU_FREQUENCY)
 
@@ -311,9 +317,11 @@ static double GetNominalCPUFrequency() {
   // a new mode (turbo mode). Essentially, those frequencies cannot
   // always be relied upon. The same reasons apply to /proc/cpuinfo as
   // well.
+#if !defined(__OpenBSD__) && !defined(__FreeBSD__) // pledge violation
   if (ReadLongFromFile("/sys/devices/system/cpu/cpu0/tsc_freq_khz", &freq)) {
     return freq * 1e3;  // Value is kHz.
   }
+#endif
 
 #if defined(ABSL_INTERNAL_UNSCALED_CYCLECLOCK_FREQUENCY_IS_CPU_FREQUENCY)
   // On these platforms, the TSC frequency is the nominal CPU
@@ -433,6 +441,18 @@ pid_t GetTID() {
   static_assert(sizeof(pid_t) == sizeof(thread),
                 "In NaCL int expected to be the same size as a pointer");
   return reinterpret_cast<pid_t>(thread);
+}
+
+#elif defined(__OpenBSD__)
+
+pid_t GetTID() {
+  return getthrid();
+}
+
+#elif defined(__FreeBSD__)
+
+pid_t GetTID() {
+  return pthread_getthreadid_np();
 }
 
 #else
