#Algorithm Descriptions
##OVerlapping Community Detection Algorithms
- Cis : Connected Iterative Scan
- Demon: Democratically Estimate of Modular Organization of Network
- Gce: Greedy Clique Expansion
- Seed Expansion

##Points
- multi-threading is quite a interesting thing... race-condition there

##Improving Points
###Cis
- Two memory access, just like the virtual function implementation, function pointer
    - `using MemberInfoMap = std::unordered_map<IndexType, unique_ptr<MemberInfo>>;`, usage of `unique_ptr` is not efficient
    - `unique_ptr<CommunityMemberSet> members_;`, usage of `unique_ptr` is not efficient
    - many parts, need to replace the `unique_ptr` with l-value-ref, and return the l-value-ref, since the allocation is conducted by the std containers, 
    with corresponding allocator
    - actually, in many scenarios, we do not expect the usage of pointers, need to think more 