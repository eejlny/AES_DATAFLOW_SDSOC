# AES_DATAFLOW_SDSOC

Implements DATAFLOW between all the processing blocks of the AES encryption algorithm. 

Performance on the ZED board with a Zynq 7020 is ~ 192K bytes per ms with a 100 MHz clock. This is 595x faster than the performance of the C version on the Cortex A9 processor present in the Zynq SOC. 

The microarchitecture consists of a single compute unit that achieves an initiation interval of 2. Higher initiation inverval doubles the memory requirements and it does not fit in the Zynq 7020. Utilization in the device is 57% BlockRAM, 16% FF and 86% LUT.    

The Zynq Ultrascale version uses a ZCU102 board. This device achieves an initiation inverval of 1 and fits 4 compute units. Performance increases to ...

