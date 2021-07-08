# NI-DAQ plugin

## 

The NI-DAQ plugin for the Open Ephys GUI adds support for analog and digital input signals from USB/PXI NI-DAQ devices. The plugin has been tested with the following devices:

* PCIe-6321, PXI-6133, PXIe-6341
  * BNC-2090A
  * BNC-2110
  
* USB-6001 

##### Important Note: Syncing analog and digital channels only works with NI devices that support correlated (hardware-timed) digital I/O.

The plugin is limited to support 8 analog inputs and 8 digital inputs maximum. If your experiment requires more inputs, please send me an e-mail at pavel@open-ephys.org for a custom version. 

If you are using a different NI-DAQ device in your setup and can confirm it works and/or has issues, please let us know!
 
The full module documentation can be found [here](https://open-ephys.github.io/gui-docs/User-Manual/Plugins/NIDAQmx.html).
