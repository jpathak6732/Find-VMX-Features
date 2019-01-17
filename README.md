# Find-VMX-Features
To create a Linux kernel module that will query various MSRs to determine virtualization features available in a CPU. 
This module will report (via the system message log) the features it discovers.

### Functionality Implemented
• Determines if the CPU supports VMX true controls
• Based on the above, read various MSRs to ascertain support capabilities/features
  ◦ Entry / Exit / Procbased / Secondary Procbased / Pinbased controls
• For each group of controls above, interpret and output the values read from the MSR to the system via printk(..), including if the value can be set or cleared.
