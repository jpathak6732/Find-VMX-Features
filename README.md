# Find-VMX-Features
To create a Linux kernel module that will query various MSRs to determine virtualization features available in a CPU. 
This module will report (via the system message log) the features it discovers.

### Functionality Implemented
- Determines if the CPU supports VMX true controls
- Based on the above, read various MSRs to ascertain support capabilities/features - Entry / Exit / Procbased / Secondary Procbased / Pinbased controls
- For each group of controls above, interpret and output the values read from the MSR to the system via printk(..), including if the value can be set or cleared.

The following procedure describes the steps followed to develop and test the kernel module:

1)	Clone the kernel sources from the master linux git repository:
>> git clone https://github.com/torvalds/linux.git

This clones the linux kernel sources to a directory named "linux".
2)	Change to the cloned directory:
>> cd linux

### Module development
3)	Create a new directory named “cmpe283” in the previously cloned “linux” source folder.
>> mkdir cmpe283

4)	Copy the template “Makefile” provided to the cmpe283 directory.

5)	The functionality to query all the other MSRs as explained in the assignment description is added to cmpe283-1.c.
•	By referring SDM, created structures with name (description) and bit positions for procbased, secondary procbased, entry and exit controls.
•	In order to detect and print VMX capabilities of CPU, the function report_capability ( ) is called with appropriate parameters passed in order to print pinbased, procbased, entry and exit controls.
•	In order to determine if true controls are available, a new function has been written to check the 55th  bit of IA32_VMX_BASIC MSR. If this bit is set, then true controls are available and another function will be called to print the corresponding true VMX capabilities.
•	In order to determine if Secondary Procbased controls are available, , a new function has been written to check 63rd bit of IA32_VMX_PROCBASED MSR. If this bit is set, then secondary procbased controls are available and another function will be called to print the corresponding secondary procbased capabilities.

6)	Change to the cmpe283 directory:
>> cd cmpe283

7)	Build the module using the following command inside the cmpe283 directory:
>> make all

8)	Load and unload the module using the following commands:
When a module is inserted into the kernel, the module_init macro will be invoked, which will call the function init_module. 
Similarly, when the module is removed with rmmod, module_exit macro will be invoked, which will call the cleanup_module.
>> sudo insmod ./cmpe283­1.ko

>> sudo rmmod ./cmpe283­1.ko

9)	The VMX features must now be logged in the kernel log and can be verified using the dmesg command:
>> dmesg
