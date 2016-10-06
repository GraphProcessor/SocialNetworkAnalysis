cd ..
source ./run_cis_demon.sh
echo 'start'
run_with_different_thread_count demo_input_files/collaboration_edges_input.csv demo_output_files/collaboration_  seq
run_with_different_thread_count demo_input_files/collaboration_edges_input.csv demo_output_files/collaboration_with_reduce_  reduce
run_with_different_thread_count ../Dataset/social_network/facebook_combined.txt demo_output_files/facebook_  seq
run_with_different_thread_count ../Dataset/social_network/facebook_combined.txt demo_output_files/facebook_with_reduce_  reduce
echo 'end'
