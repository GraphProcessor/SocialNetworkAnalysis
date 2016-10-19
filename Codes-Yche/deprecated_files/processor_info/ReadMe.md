###Environment(CPU Checking)
- Physical    

  ```zsh
  cat /proc/cpuinfo |grep "physical id"|sort |uniq|wc -l  
  ```
- Logical    

  ```zsh
  cat /proc/cpuinfo |grep "processor"|wc -l
  ```
- Core   

  ```zsh
  cat /proc/cpuinfo |grep "cores"|uniq
  ```

###CPU-Logical-Core
- Cluster 15 : 20 cores
- Cluster 16 : 20 cores
- Cluster 23 : 40 cores
- LabDesktop : 8 cores

###LabDesktop      
Type| Param
-------------- | ---------------  
Architecture:      |     x86_64           
CPU op-mode(s):     |    32-bit, 64-bit          
Byte Order:         |    Little Endian             
CPU(s):             |    8              
On-line CPU(s) list: |    0-7         
Thread(s) per core:   |  2            
Core(s) per socket:   |  4             
Socket(s):           |   1          
NUMA node(s):       |    1    
Vendor ID:          |    GenuineIntel   
CPU family:         |    6    
Model:              |    58   
Model name:         |    Intel(R) Core(TM) i7-3770 CPU @ 3.40GHz    
Stepping:            |   9    
CPU MHz:             |   1613.007   
CPU max MHz:         |   3900.0000    
CPU min MHz:         |   1600.0000    
BogoMIPS:           |    6807.03    
Virtualization:     |    VT-x   
L1d cache:          |    32K    
L1i cache:           |   32K    
L2 cache:            |   256K   
L3 cache:            |   8192K    
NUMA node0 CPU(s):   |   0-7    
Flags:                 fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx rdtscp lm constant_tsc arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc aperfmperf eagerfpu pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm ida arat epb pln pts dtherm tpr_shadow vnmi flexpriority ept vpid fsgsbase smep erms xsaveopt   

###Cluster 15
Type| Param
-------------- | ---------------
Architecture:    |      x86_64   
CPU op-mode(s):    |    32-bit, 64-bit   
Byte Order:        |    Little Endian    
CPU(s):            |    20   
On-line CPU(s) list: |  0-19   
Thread(s) per core:  |  1    
Core(s) per socket:  |  10   
Socket(s):        |     2    
NUMA node(s):      |    2    
Vendor ID:         |    GenuineIntel   
CPU family:        |    6    
Model:             |    62   
Stepping:          |    4    
CPU MHz:           |    2194.436   
BogoMIPS:           |   4388.77    
Virtualization:      |  VT-x   
L1d cache:           |  32K    
L1i cache:           |  32K    
L2 cache:            |  256K   
L3 cache:            |  25600K   
NUMA node0 CPU(s):   |  0-9    
NUMA node1 CPU(s):   |  10-19    

###Cluster 23
Type| Param
-------------- | ---------------
Architecture:       |   x86_64   
CPU op-mode(s):     |   32-bit, 64-bit   
Byte Order:        |    Little Endian    
CPU(s):            |    40   
On-line CPU(s) list:  | 0-39   
Thread(s) per core:  |  2    
Core(s) per socket: |   10   
Socket(s):          |   2    
NUMA node(s):      |    2    
Vendor ID:         |    GenuineIntel   
CPU family:        |    6    
Model:             |    63   
Stepping:          |    2    
CPU MHz:            |   2299.861   
BogoMIPS:           |   4599.35    
Virtualization:    |    VT-x   
L1d cache:         |    32K    
L1i cache:          |   32K    
L2 cache:          |    256K   
L3 cache:          |    25600K   
NUMA node0 CPU(s):  |   0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38   
NUMA node1 CPU(s):  |   1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39   
