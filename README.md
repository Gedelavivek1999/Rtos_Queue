**Mitigating Wireless Link Latency Spikes and Packet Loss**

Issue: Telemetry data from the flight controller (such as position, altitude, and orientation)
is transmitted via UART protocols. Potential packet loss or transmission blocking can interrupt data flow,
compounded by priority inversion issues between concurrent tasks.

Method: Implemented an asynchronous FreeRTOS queue architecture running at a deterministic 1.5ms cycle to buffer 
and stream data. If transient packet loss occurs, non-blocking queue operations allow control tasks to pass resources 
seamlessly without causing system stalls or disruptions.

Resolves: Latency spikes, task blocking, and priority inversion issues.

