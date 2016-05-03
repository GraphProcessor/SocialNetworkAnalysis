#Current Work
- Reimplement Algorithm [*Connected Iterative Scan*](./test_cis.cpp)
- Reimplement Algorithm [*Demon*](./test_daemon.cpp)
- Implement [Parallelizer](./parallelizer.h) for local-based overlapping community detection algorithms
- [Util](./util) *Draw Figures*  (e.g as facebook output handling)

```zsh
python get_statistics.py ~/Gitrepos/SocialNetworkAnalysis/Codes-Yche/demo_output_files/  facebook
```
#Pending Work
- Reimplement Algorithm *GCE*

#Build
- Build with Cmake 3.3, Gcc 5.3, C++ 14
- Dependencies : Boost, Pthreads

#Attention Please
- Pthread_Create, void * have to make the input arguments existing until they are joined  
- Pay attention to dead lock  

#Potential Improvement Points
- For CIS algorithm, the merge operation takes a lot of time for the dataset of collaboration, which is sequential,sequential 4s, parallel 2.4s    
- For Demon algorithm, not exist such problem, and the execution time is very long, 13s for parallel and 24s for sequential,  
the merge operation do not take a lot of time

#Environment(CPU Checking)
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

#Experiment
- Merge operation just sequentially execute  
- Overlap some merge with Local computation, not significant  

##Collaboration 20 thousand edges
![CisExp](./images/collaboration_cis_v1.png)  
![DemonExp](./images/collaboration_demon_v1.png)  

##Facebook 80 thousand edges
![CisExp_Facebook](./images/facebook_cis_v1.png)  
![DemonExp_Facebook](./images/facebook_demon_v1.png)  
