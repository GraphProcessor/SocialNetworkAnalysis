#!/bin/bash
#$1 inputfile path
#$2 outputfile dir
#$3 reduce or sequential
#$4 is the path
function run_with_different_thread_count(){
  for thread_count in $( seq 1 32 )
  do
	      outputfile=$2"demon_"${thread_count}
        inputfile=$1
        echo $inputfile
        $4/Daemon_Test $thread_count $inputfile $3> $outputfile
        outputfile=$2"cis_"${thread_count}
        $4/CISTest $thread_count $inputfile $3> $outputfile
        let thread_count+=1
  done
}
