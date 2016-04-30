import os, sys, re, collections
import numpy as np
import matplotlib.pyplot as plt

pattern_elapsed_str = re.compile(r'.*Time.*')
pattern_time_str = re.compile(r'[0-9]+\.[0-9]+')


def get_count_time_map(file_pattern, file_dir):
    file_list = os.listdir(file_dir)
    my_dict = {}
    for open_file in file_list:
        if file_pattern.match(open_file):
            filename = str(open_file) + ' \t'
            print filename
            index = open_file.split('_')[2]
            my_dict[index] = filename
    ordered_dict = sorted(my_dict.iteritems(), key=lambda e: int(e[0]), reverse=False)

    # Store Thread_Count And Time Info Tuple
    my_tuple_list = ()
    for key in ordered_dict:
        openfilename = key[1]
        openfilename = openfilename.split(' ')[0]
        my_tuple = (openfilename,)
        with open(file_dir + openfilename) as fin:
            data_lists = fin.readlines()
            for data in data_lists:
                if pattern_elapsed_str.match(data):
                    info = data.strip()
                    time = re.findall(pattern_time_str, info)[0]
                    my_tuple += (time,)
        time_second_phase = float(my_tuple[2]) - float(my_tuple[1])
        my_tuple += ("{:.4f}".format(time_second_phase),)
        my_tuple_list += (my_tuple,)
    return my_tuple_list


def draw_bar(tuple_list, title_name):
    N = 32
    bar_one_tuple = ()
    bar_two_tuple = ()
    for my_tuple in tuple_list:
        bar_one_tuple += (float(my_tuple[1]),)
        bar_two_tuple += (float(my_tuple[3]),)
    menMeans = bar_one_tuple
    womenMeans = bar_two_tuple
    menStd = ()
    womenstd = ()
    for i in range(0, 32):
        menStd += (0,)
        womenstd += (0,)
    ind = np.arange(N)  # the x locationget_statistics.py:56s for the groups
    width = 0.8  # the width of the bars: can also be len(x) sequence

    p1 = plt.bar(ind, menMeans, width, color='red', yerr=menStd)
    p2 = plt.bar(ind, womenMeans, width, color='lime',
                 bottom=menMeans, yerr=womenstd)
    plt.ylabel('RunTime/s')
    plt.title(title_name)
    max_val = max(enumerate(bar_two_tuple), key=lambda x: x[1])[1]
    print max_val
    plt.yticks(np.arange(0, max_val * 3, 1))
    plt.legend((p1[0], p2[0]), ('Parallel Computation', 'Sequential Merge'))

    plt.show()


my_dir = sys.argv[1]
pattern_cis = re.compile(r'.*cis.*')
pattern_demon = re.compile(r'.*demon.*')
get_count_time_map(pattern_cis, my_dir)
print '\n'
my_tuple_list = get_count_time_map(pattern_demon, my_dir)
draw_bar(my_tuple_list, 'Demon Algorithm Parallel Run-Time')
my_tuple_list = get_count_time_map(pattern_cis, my_dir)
draw_bar(my_tuple_list, 'Cis Algorithm Parallel Run-Time')
