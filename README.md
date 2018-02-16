# AES_DATAFLOW_SDSOC

Implements DATAFLOW between all the processing blocks of the AES encryption algorithm. The following results are based on SDSOC V2017.4.

ZYNQ

Performance on the ZED board with a Zynq 7020 is ~ 192Mbytes per s with a 100 MHz clock. This is 595x faster than the performance of the AES C version on the Cortex A9 processor (600MHz) present in the Zynq SOC. 

The microarchitecture consists of a single compute unit that achieves an initiation interval (II) of 2. Higher initiation inverval doubles the memory requirements and it does not fit in the Zynq 7020. Utilization in the device by the AES core is 57% BlockRAM, 16% FF and 86% LUT.    

The core uses 1 HP/AFI port with a 64-bit width. This means that 2 thansfers are needed to fetch 1 16-byte AES block which is balanced with the II of 2 of the processing pipeline.

ZYNQ ULTRASCALE

The Zynq Ultrascale version uses a ZCU102 board. This device has enough resources to achieve an initiation inverval of 1 and it also fits 4 compute units working in parallel. Performance increases to 2190Mbytes per second with a 300 MHz clock. This is 4385x faster than the speed of the Cortex A53 (1.2 GHz) present in the SoC.  

The Zynq ultrascale uses the 4 HP/AFI ports available with a width of 128-bit so that each CU uses its own HP/AFI port. The optimal width is 128-bit since then the whole 16 byte block encryted by AES can be input into the pipeline per clock cycle.

NOTE: Notice that a Zynq Ultrascale device with a 300 MHz hardware clock and a II of 1 should be able to process a 16-byte block per clock cycle with a theoretical troughput of 300*16 = 4800 MBytes/second per CU. The theoretical troughput for 4 CU is therefore 19200 MBytes/second. The lower speed of the real hardware is probably due to memory bandwidth limitations since the data must be brought from DDR.     