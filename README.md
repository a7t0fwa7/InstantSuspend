# InstantSuspend

Instantly suspend processes to bypass kernel driver protections.

Some programs communicate with protection kernel drivers after startup to prevent tampering. A common approach is for a kernel driver to register a callback, ensuring that other programs cannot obtain the correct process handle using `OpenProcess`. This simple tool addresses this by either creating the process in a suspended state or suspending it immediately upon creation, preventing it from communicating with the driver. This allows a handle to be obtained before the process becomes protected.

> [!IMPORTANT]
> It is important to note that some kernel drivers may still block access after the process is resumed. However, for certain programs, the opened handle remains valid and continues to allow memory read/write operations even after the process is resumed.

## Usage

Either start a process suspended with `InstantSuspend.exe start .\Path\to\Example.exe` or wait for a process to start then instantly suspend it with `InstantSuspend.exe await Example.exe`. The process will be in a suspended state. You can now attach to the process to obtain a handle and view/modify its code:

![image](https://github.com/user-attachments/assets/6a7c6a59-28a1-456b-9643-c0a1e46b145e)

We can try to change this instruction:

![image](https://github.com/user-attachments/assets/39990eb0-078f-4ed2-9dd7-cfa3019871d9)

...and it is changed successfully:

![image](https://github.com/user-attachments/assets/e56cc2b7-dffb-483c-8932-c269249b0021)

After we resume the process, this memory region becomes inaccessible since it has been protected by the kernel driver with black magic:

![image](https://github.com/user-attachments/assets/0966c7e4-31e1-4ece-99f4-2c6a9d376a33)
