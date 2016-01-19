#Community Detection Related (Mainly Social Network)
##Possible Research Points(Especiallly on Directed Graph)
1. Evaluate prior detection methods(non-overlapping, overlapping, hierarchical) on metric and scalability (**WWW, ASONAM**)
2. Implement userful algorithms on **GraphX(Scala), PowerGraph(C++), Giraph(Java)** and estimate efficiency and analyze hwo to transparent sequential one into parallel environment (**SC, IPDPS**)  
3. Implement algorithms with GPU/MIC acceleration
2. Study fast local community search/query (**VLDB, SIGMOD**)  
3. Study index technique to help combine several algorithms for social network analysis(**My Imagination**)  
2. Study Sampling large scale graph for approximated structure(**KDD**)  

##Influential Researcher
- Foreign
    - **[Andrea Lancichinetti](https://sites.google.com/site/andrealancichinetti/) (Umeå University)**  
    - **[Jure Leskovec](http://cs.stanford.edu/people/jure/) (Stanford)**  
- China
    - **[Jie Tang](http://keg.cs.tsinghua.edu.cn/jietang/) (Tsinghua)**  
    - **[Jianyong Wang](http://dbgroup.cs.tsinghua.edu.cn/wangjy/) (Tsinghua)**  

## Good Survey
1. **[Overall Survey](http://lab41.github.io/survey-community-detection/)** (Home Page : http://lab41.github.io/Circulo/)  

##Prior Famous Algorithms & Evaluation Benchmark
- Reference  
	- (Refer to [Reihaneh Rabbany](https://scholar.google.com.hk/citations?user=Foh_c-QAAAAJ&hl=zh-CN&oi=ao)'s Github: https://github.com/rabbanyk/CommunityEvaluation)  
	- (Refer to [Andrea Lancichinetti](https://sites.google.com/site/andrealancichinetti/)'s Implementation)  

- Algorithms  
	- 2004, **Fast Modularity**, *[Aaron Clauset]*, Finding community structure in very large networks  
	- 2005, **Walker Trap**, *[Pascal Pons]*, Computing Communities in Large Networks Using Random Walks  
	- 2005, **Spectral Algorithm**, *[Luca Donetti]*, Improved spectral algorithm for the detection of network communities  
	- 2007, **Label Propogation**, *[Usha Nandini Raghavan]*, Near linear time algorithm to detect community structures in large-scale networks  
	- 2007, **Modularity Opt(Simulated Annealing)**, *[Marta Sales-Pardo]*, Extracting the hierarchical organization of complex systems  
	- 2008, **Louvain**, *[Vincent D Blondel]*, Fast unfolding of communities in large networks  
	- 2008, **Infomap**, *[Martin Rosvall]*, Maps of random walks on complex networks reveal community structure  
	- 2009, **Potts model**, *[Peter Ronhovde]*, Multiresolution community detection for megascale networks by information-based replica correlations  
	- 2010, **Link-Plus**, *[Yong-Yeol Ahn]*, Link communities reveal multiscale complexity in networks  
	- 2010, **Greedy Clique Expansion**, *[Conrad Lee]*, Detecting Highly Overlapping Community Structure by Greedy Clique Expansion  
	- 2010, **MOSES**, *[Aaron McDaid]*, Detecting higly overlapping communities with Model-based Overlapping Seed Expectation  
	- 2010, **Potts Model**, *[Peter Ronhovde]*, Local resolution-limit-free Potts model for community detection  
	- 2010, **COPRA(Label Propogation)**, *[Steve Gregory]*, Finding overlapping communities in networks by label propagation  
	- 2010, **Top Leader**. *[Reihaneh Rabbany Khorasgani]*, Top Leaders Community Detection Approach in Information Networks  
	- 2011, **(State-of-Art) OSOLOM**, *[Andrea Lancichinetti]*, Finding Statistically Significant Communities in Networks  
	- 2011, **Multi-Level-Infomap**, *[Martin Rosvall]*, Multilevel Compression of Random Walks on Networks Reveals Hierarchical Organization in Large Integrated Systems  
	- 2012, **Consensus Clustering**, *[Andrea Lancichinetti]*, Consensus clustering in complex networks  
	- 2012, **Community-Affiliation Graph Model**, *[Jaewon Yang]*, Community-Affiliation Graph Model for Overlapping Network Community Detection  
	- 2013, **(State-of-Art) Large Scale CAG, BigClam**, *[Jaewon Yang]*, Overlapping Community Detection at Scale: A Nonnegative Matrix Factorization Approach  

- Scalable Algorithms
	- 2009, **Propinquity dynamics**, *[Jianyong Wang]*, Parallel Community Detection on Large Networks with Propinquity Dynamics  
	- 2013, **Ensemble**, *[Michael Ovelg¨onne]*, Distributed Community Detection in Web-Scale Networks  
	- 2013, **Fast Algorithm for Modularity-based**, *[Hiroaki Shiokawa]*, Fast Algorithm for Modularity-based Graph Clustering  
	- 2014, **SCD**, *[Arnau Prat-Pérez]*, High Quality, Scalable and Parallel Community Detection for Large Real Graphs
	- 2014, **RelaxMap**,*[Seung-Hee Bae]*, Scalable Flow-Based Community Detection for Large-Scale Network Analysis  
	- 2015, **GossipMap**, *[Seung-Hee Bae]*, GossipMap: A Distributed Community Detection Algorithm for Billion-Edge Directed Graphs  

- Evaluations  
	- 2009, **NMI for overlapping Community Detection**, *[Andrea Lancichinetti]*, Detecting the overlapping and hierarchical community structure in complex networks  
	- 2010, **Snap empirical comparison**, *[Jure Leskovec]*, Empirical Comparison of Algorithms for Network Community Detection  
	- 2011, **NMI for overlapping Community Detection**, *[Aaron F. McDaid]*, Normalized Mutual Information to evaluate overlapping community finding algorithms  
	- 2012, **WCC(a metric based on triangle structures in a community)**, *[Arnau Prat-Pérez]*, Shaping Communities out of Triangles  
	- 2013, **Summarize LFR and RC benchmark**, *[Rodrigo Aldecoa]*, Exploring the limits of community detection strategies in complex networks (**Notes: Evaluate 17 algorithms**)  
	- 2013, **Effective Evaluation**, *[Conrad Lee]*, Community detection: effective evaluation on large social networks  
	- 2015, **rNMI**, *[Pan Zhang]*, Evaluating accuracy of community detection using the relative normalized mutual information  

- Optimization Techniques
	- 2014, **General Optimization**, *[Stanislav Sobolevsky]*, General optimization technique for high-quality community detection in complex networks  

## Tools
- General Library
    1. **[Some Recommendations From CppReference](http://en.cppreference.com/w/cpp/links/libs)** (Boost.Graph, LEMON, OGDF, NGraph)  
    1. **Networkx** has implemented **[many graph algorithms](http://networkx.github.io/documentation/networkx-1.10/reference/algorithms.html)**  
    2. **[Snap](https://github.com/snap-stanford/snap)** (Infomap, Fast Newman, BIGCLAM, CESNA, CoDA, RoIX)  
    3. **[igraph](https://github.com/igraph/igraph)** (Infomap, WalkTrap, Leading Eginvector)  
		- **Implementations**: edge_betweenness, leading_eigenvector, spinglass, fastgreedy , leading_eigenvector_naive, walktrap, infomap, multilevel, label_propagation, optimal_modularity  
		- **Usage Explaination**: https://github.com/rabbanyk/CommunityEvaluation/blob/master/src/algorithms/communityMining/external_methods/iGraph/communityMinerInterface.py  
- BLAS
	5. **[Boost BLAS](http://www.boost.org/doc/libs/1_60_0/libs/numeric/ublas/doc/index.html)** (Boost Library)  
    2. **[Egien](http://eigen.tuxfamily.org/index.php?title=Main_Page)** (Cpp template library for linear algebra related algorithms)  
    6. **[ArmAdillo](http://arma.sourceforge.net/)**(Primarily developed at Data61 (Australia) by Conrad Sanderson)  


## Codes
- C/C++:
	- Benchmark  
        - [2009 Lancichi Benchmark](https://sites.google.com/site/santofortunato/inthepress2) (from GoogleSite)  
        - [2013 Conrad Lee Benchmark](https://github.com/conradlee/network-community-benchmark) (from Github)  
    - Algorithm  
    	- [2004 Clauset](https://github.com/ddvlamin/CommunityDetectionC) (from Github)  
    	- [2010 linkcomm](https://github.com/bagrow/linkcomm) (from Github)  
    	- [2010 GCE](https://sites.google.com/site/greedycliqueexpansion/) (from GoogleSite)  
    	- [2011 SLPA](https://sites.google.com/site/communitydetectionslpa/) (from GoogleSite)  
    	- [2011 Dense Subgraph Extraction](https://github.com/sranshous/Graph-Community-Detection) (from Github)  
    	- [2011 Marvelot](http://www.elemartelot.org/index.php/programming/cd-code) (from Website)  
    	- [2012 Combo](http://senseable.mit.edu/community_detection/combo.zip) (from Martelot's Homepage), [Combo And Others](https://github.com/sina-khorami/AI-community-detection) (from Github)  
    	- [2014 SCD](https://github.com/DAMA-UPC/SCD) (from Github), [SCD GPU implementation](https://github.com/Het-SCD/Het-SCD) (from Github)  
    	- [2015 GossipMap](https://github.com/uwescience/GossipMap) (from Github), [2013 RelaxMap](https://github.com/uwescience/RelaxMap) (from Github)  

    6. [Community-Detection-Betweenness](https://github.com/sidrakesh/Community-Detection-Betweenness) ( modified version of Brandes algorithm(BC))  
    9. [Par-CD](https://github.com/stijnh/Par-CD) (Multi-core CPU and GPU implementation)  
    10. [paco](https://github.com/CarloNicolini/paco) (Benchmark Refer to Physics Review 2009)  
    12. [k-clique Percolation](https://sites.google.com/site/cliqueperccomp/) (k-clqiue Percolation 2012)  

- Java:
    1. [distributed-graph-analytics](https://github.com/Sotera/distributed-graph-analytics) (BSP impl,e.g., High Betweenness Set Extraction, Weakly Connected Components, Page Rank, Leaf Compression, and Louvain Modularity)  
    2. [Reihaneh Rabbany](https://github.com/rabbanyk/CommunityEvaluation) (5th-year Phd Student, Give integration of many sequential implementations)  
    3. [noesis](https://github.com/sisusisu/noesis)(Some Impls)  

- Python:
    1. [Label-Propagation](https://github.com/liyanghua/Label-Propagation) (Refer to Pyshics 2007)  
    2. [Circulo](https://github.com/Lab41/Circulo) (With Some Implementations and Refer to SNAP)  

## Recent Interesting Published Papers(With Codes)
- Algorithms
	- (**GossipMap, Use InfoTheory**) [GossipMap: a distributed community detection algorithm for billion-edge directed graphs](http://dl.acm.org/citation.cfm?id=2807668) (SC 2015)  
	- (**Scalable Community Detection**) [High quality, scalable and parallel community detection for large real graphs](http://www.dama.upc.edu/publications/fp546prat.pdf) (WWW 2014)  
	- (**Marvelot**) [Fast Multi-Scale Detection of Relevant Communities in Large-Scale Networks](http://comjnl.oxfordjournals.org/content/early/2013/01/22/comjnl.bxt002.full.pdf+html)(The Computer Journal 2013)  
	- (**SLPA**) [Towards Linear Time Overlapping Community Detection in Social Networks](http://arxiv.org/pdf/1202.2465.pdf) (Advances in Knowledge Discovery and Data Mining 2012)  
	- (**Dense Subgraph Extraction**) [Dense Subgraph Extraction with Applicationto Community Detection](http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=5677532) (TKDE 2012)  
- Evaluation
	- [Benchmarks for testing community detection algorithms on directed and weighted graphs with overlapping communities](http://journals.aps.org/pre/pdf/10.1103/PhysRevE.80.016118) (Physics Review 2009)  
	- [Community detection: effective evaluation on large social networks](http://comnet.oxfordjournals.org/content/2/1/19.full.pdf+html) (Journal of Complex Networks 2014)  

- Optimization Techniques
	- [General optimization technique for high-quality community detection in complex networks](http://journals.aps.org/pre/pdf/10.1103/PhysRevE.90.012811) (Physics Review 2014)  




