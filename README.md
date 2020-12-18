# Sada-DLIndex
This is the full-text index of Sadakane [1] to solve DL. In this we use the FMI (from sdsl-library of Simon Gog) and our RMQ structure.

Authors: Hector Ferrada and Gonzalo Navarro hferrada@dcc.uchile.cl, gnavarro@dcc.uchile.cl

Description: This is a compressed full-text index to solve document listing. The implementation is based on the method of Sadakane [1].  
In that paper he uses his CSA to index the input collection. As we did not found a 64-bit version for his CSA, we have used a 64-bit version of a FMI to replace it. The FMI is a free code, included in the sdsl-library of S. Gog (https://github.com/simongog/sdsl-lite).  
Then it is necessary to install sdsl-library to run this index.  
In order to compute RMQs we use our own implementation, which is based on the method of Fischer and Heun [2]. For the tree representation needed in [2] we use a light version of the Range Min-Max Tree of Sadakane and Navarro [3].  
This RMQ structure and also other basic functions needed in the code are in our small library called DRF_Utils64. Then you have to download it and compile it to run the index (https://github.com/hferrada/DRF_Utils64.git)  

COMPILE, MAKE and LINKING
=========================

1.- Install sdsl-library. Download it from https://github.com/simongog/sdsl-lite and follow the specifications given there.  
2.- Install DRF_Utils64 library. Download it from https://github.com/hferrada/DRF_Utils64 and follow the specifications given there.  
3.- Edit the Makefile file included in the folder 'Sada-DLIndex' and modify the next lines:  
In the line INCLUDES, put the path of the 'include' folder, where the header files "divsufsort.h" and "divsufsort.h" and the directory sdsl are. For example:  
INCLUDES=-I/home/hferrada/include/ -I/home/hferrada/drf/dir64/DRF_Utils64/includes/  
In the line LIB, indicate the files generated by the libraries: libsdsl.a, libdivsufsort.a, libdivsufsort64.a and drflib64.a (include absolute paths). For example:  
LIB=/home/hferrada/lib/libsdsl.a /home/hferrada/lib/libdivsufsort.a /home/hferrada/lib/libdivsufsort64.a /home/hferrada/drf/dir64/DRF_Utils64/drflib64.a  

4.- Make: To make the index just execute the command 'make' and this will create the lib: 'SadaDL.a'.  
5.- Use the index in your program linking the 'SadaDL.a' file and include the header "SadaDocList64.h" in your sorce code.  
  * Execute: "make build_binary" to generate a binary file "buildDL_Sada" from the source file "buildDL_Sada.cpp" (included here as example)  
    buildDL_Sada.cpp creates the index given the parameters included in a config file, which is indicated in the call.  
  * Execute: "make load_binary" to generate a binary file "loadDL_Sada" from the source file "loadDL_Sada.cpp" (included here as example)  
  * loadDL_Sada.cpp. This loads the index given the parameters included in the config file, which is indicated in the call.  

Config File example  
===================  

It is not necesary uses the files buildDL_Sada.cpp and loadDL_Sada.cpp to generate the binaries files. These are only examples.  
If you use them, then you need to modify the parameters in the config file. This is the format:  

--------------------------------------------------------------------
[GLOBALS]  

TRACE = 0	   # 0: do not list the trace, 1: list the trace  
TEST = 1	    # 0: do not run the test, 1: run the test  
N_REP = 100 	# number of test and repetitions for experiments  
RUN_EXP = 1	 # 0: do not run the experiments, 1: run the experiments  
MAX_M = 10	  # The experiments run with initial lenght pattern, m=6, and increments this value in  
            # 4 until to reach to MAX_M. The test run from length attern m=1 to m=MAX_M and for  
            # each m value run N_REP repetitions   

[DL]  
inputFile = /home/hferrada/text_data/cluewiki.txt  # a unique input file (filesInList = 0) or a list of files (filesInList = 1)
filesInList = 1		# 1: list of files (one line for file), 0: Unique file with all the documents of the collection  
boundSymbol = 1		# original symbol delimiter of documents when we read all documents in one file.  
cutDoc = 1		    # new symbol to separate documents  
dirStore = /home/hferrada/drf/upSiteTest/Sada-DLIndex/cluewiki/  # the directory to save/load the data structure (files *.dls)  
dirResult = /home/hferrada/drf/upSiteTest/Sada-DLIndex/cluewiki/cluewiki_  # the directory to save the results of the experiments as summary files  

  

References: Please, if you want to include this tool as part of your experiments, in your references include the two papers above. Later, it will appear another publication to replace these ones.  

[1] K. Sadakane. Space-efficient data structures for flexible text retrieval systems.   
    In Proc. 13th International Conference on Algorithms and Computation (ISAAC), pages 14–24, 2002.  
[2] J. Fischer and V. Heun. Space-efficient preprocessing schemes for range minimum queries on static arrays.  
    SIAM Journal on Computing, 40(2):465–492, 2011.  
[3] K. Sadakane. Fully-Functional Static and Dynamic Succinct Trees.  
    ACM Transactions on Algorithms 10(3):article 16, 2014
