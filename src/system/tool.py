import sys, os, codecs

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
                      for hans in items[1]:
                          l = hans + ' ' + items[0] + '  ' + '0' +  '\n'
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
                      py_list[items[0]] = items[1]

        for line in f_phrase:
             if not line.startswith('#'):
                  line = line.strip()
                  if line != "":
                      l = ""
                      for han in line:
                          py = py_list.get(han, "")
                          if py != "":
                              if l == "":
                                 l = py
                              else:
                                 l += '-' + py

                      l += '\t' + line + '\n'
                      f_py_phrase.write(l)

    finally:
        f_py.close()
        f_phrase.close()
        f_py_phrase.close()

if __name__ == '__main__':
    argv = sys.argv
    #make_pinyin_file(argv[1], argv[2])
    make_phrase_file("han-utf8.tdf", argv[1], argv[2])
