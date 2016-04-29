import os, sys, re, collections

dir = '/home/cheyulin/gitrepos/SocialNetworkAnalysis/Codes-Yche/demo_output_files/'
list = os.listdir(dir)
pattern_cis = re.compile(r'demo.*cis.*')
pattern_demon = re.compile(r'demo.*demon.*')
dict = collections.OrderedDict()

for file in list:
    if pattern_cis.match(file):
        filename = str(file) + ' \t'
        index = file.split('_')[3]
        dict[index] = filename

test = sorted(dict.iteritems(), key=lambda e: int(e[0]), reverse=False)

for key in test:
    print key

print '\n'

for file in list:
    if pattern_demon.match(file):
        print str(file)
