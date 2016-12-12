#! /usr/bin/python3

import sys, os, codecs

ph_freq_table = {}

def get_priority(p):
    if p <= 0xff:
        p = p >> 4
    else:
        p = 16 + p >> 8

    if p > 0xff:
        p = 0xff
    elif p == 0:
        p = 1

    return p

def load_freqtable():
    try:
        f_ph  = codecs.open("han_phrase_freq.txt", 'r', "utf-8")
    except IOError:
        sys.stderr.write("The file does not exist \n" )
        f_ph.close()
        sys.exit()

    try:
        for line in f_ph:
             if not line.startswith('#'):
                  line = line.strip()
                  items = line.replace('\t', ' ').split(' ')
                  if len(items) >= 2:
                      ph_freq_table[items[0]] = int(items[1])
                      #print("load_fretable: %s, %d"%(items[0], ph_freq_table[items[0]]))

    finally:
        f_ph.close()

def make_pinyin_file(i_path, o_path):
    try:
        f_input  = codecs.open(i_path, 'r', "utf-8")
        f_output = codecs.open(o_path, 'w', "utf-8")
    except IOError:
        sys.stderr.write("The file(%s) does not exist \n" %(sys.argv[1]))
        f_inpput.close()
        f_output.close()
        sys.exit()

    try:
        for line in f_input:
             if not line.startswith('#'):
                  line = line.strip()
                  items = line.replace('\t', ' ').split(' ')
                  if len(items) == 2:
                      for han in items[1]:
                          freq = ph_freq_table.get(han, 0)
                          if freq > 0:
                              freq = get_priority(freq)
                              print("joni freq%d"%(freq))

                          l = han + ' ' + items[0] + '  ' + str(freq) +  '\n'
                          f_output.write(l)

    finally:
        f_input.close()
        f_output.close()

def make_phrase_file(py_path, phrase_path, py_phrase_path):
    try:
        f_py        = codecs.open(py_path, 'r', "utf-8")
        f_phrase    = codecs.open(phrase_path, 'r', "utf-8")
        f_py_phrase = codecs.open(py_phrase_path, 'w', "utf-8")
        f_py_phrase2 = codecs.open(py_phrase_path + "_2", 'w', "utf-8")
    except IOError:
        sys.stderr.write("The file does not exist \n" )
        f_py.close()
        f_phrase.close()
        f_py_phrase.close()
        sys.exit()

    py_list = {}
    try:
        for line in f_py:
             if not line.startswith('#'):
                  line = line.strip()
                  items = line.replace('\t', ' ').split(' ')
                  if len(items) > 2:
                      pys = py_list.get(items[0],[]);
                      pys.append(items[1])
                      py_list[items[0]] = pys


        for line in f_phrase:
            if not line.startswith('#'):
                line = line.strip()
                if line != "":
                    line2s = []
                    freq = ph_freq_table.get(line, 0)
                    if freq > 0:
                        freq = get_priority(freq)
                        #print("joni freq%d"%(freq))

                    pys = py_list.get(line[0], [])
                    if (len(pys) == 0):
                        print("no py: %s"%(line[0]))		

                    for py in pys:
                        line2s.append(py)

                    for han in line[1:]:
                        pys = py_list.get(han, [])
                        line2s_len = len(line2s)
                        for i in range(line2s_len):
                            l_i = line2s[i]
                            for j in range(len(pys)):
                               l =  l_i + '-' + pys[j]
                               if j == 0:
                                   line2s[i] = l
                               else:
                                   line2s.append(l)

                    for l in line2s:
                        l +=  '\t' + line + '  ' + str(freq)  + '\n'
                        f_py_phrase.write(l)

    finally:
        f_py.close()
        f_phrase.close()
        f_py_phrase.close()

def make_phrase_file_2(py_path, phrase_path, py_phrase_path):
    try:
        f_py        = codecs.open(py_path, 'r', "utf-8")
        f_phrase    = codecs.open(phrase_path, 'r', "utf-8")
        f_py_phrase = codecs.open(py_phrase_path, 'w', "utf-8")
        f_py_phrase2 = codecs.open(py_phrase_path + "_2", 'w', "utf-8")
    except IOError:
        sys.stderr.write("The file does not exist \n" )
        f_py.close()
        f_phrase.close()
        f_py_phrase.close()
        sys.exit()

    py_list = {}
    try:
        for line in f_py:
             if not line.startswith('#'):
                  line = line.strip()
                  items = line.replace('\t', ' ').split(' ')
                  if len(items) > 2:
                      pys = py_list.get(items[0],[]);
                      pys.append(items[1])
                      py_list[items[0]] = pys


        for line in f_phrase:
            if not line.startswith('#'):
                line = line.strip()
                if line != "":
                    line2s = []
                    freq = ph_freq_table.get(line, 0)
                    if freq > 0:
                        freq = get_priority(freq)
                        #print("joni freq%d"%(freq))

                    pys = py_list.get(line[0], [])
                    for py in pys:
                        l =  py  + '-' + line + '\t' + str(freq)  + '\n'
                        f_py_phrase.write(l)

    finally:
        f_py.close()
        f_phrase.close()
        f_py_phrase.close()

if __name__ == '__main__':
    argv = sys.argv
    load_freqtable()
    # tool.py  pinyin-utf8.tdf  han-utf8.tdf.tmp
    #make_pinyin_file(argv[1], argv[2])
    
    # tool.py  phrase_utf8.txt  phrase-utf8.tdf.tmp
    make_phrase_file_2("han-utf8.tdf", argv[1], argv[2])
