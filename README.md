# NI-DAQ plugin

## 

The NI-DAQ plugin for the Open Ephys GUI adds support for analog and digital input signals from USB/PXI NI-DAQ devices. The plugin has been tested with up to 8 single-ended analog and 13 digital inputs with the following devices:

* PCIe-6321, PXI-6133
  * BNC-2090A
  * BNC-2110
* USB-6001

##### Important Note: Syncing analog and digital channels only works with NI devices that support correlated (hardware-timed) digital I/O.

If you are using a different NI-DAQ device in your setup and can confirm it works and/or has issues, please let us know!
 
The full module documentation can be found here: https://open-ephys.atlassian.net/wiki/spaces/OEW/pages/1294630913/NIDAQmx+Source+Module
