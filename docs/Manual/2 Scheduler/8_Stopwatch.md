# Stopwatch

OS/90 supports busy-waiting and asynchronous-dispatched time signals. As usual, the time quantum is always one milisecond. A 64-bit uptime counter is maintained by the kernel and is used for timing purposes.

As stated previously, OS/90 is not real-time so the API described here should be considered approximate in most cases.


1. VOID GetUptime(PQWORD out)
2. STAT AcquireStopwatch

Store the stopwatch dispatch routines in a fixed-length list. If a timer is unavailable, thread must wait. Maybe order it for performance?