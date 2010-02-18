from numpy import * 

def read_stdout_file(file):
	# read registration output
	f = open(file)
	lines = f.readlines()

	# clean up registration output
	for line_index, line in enumerate(lines):
		cleaned_line = []
		line = line.rsplit()
		for word_index, word in enumerate(line):
			word = word.lstrip('[:=')
			word = word.rstrip(':=,]')
			if not len(word) == 0:
				cleaned_line.append(float(word))
		lines[line_index] = cleaned_line

	# extract data from lines
	lines = asarray(lines)
	params = {}
	params['iteration']          = lines[:,0]
	params['metric_value']       = lines[:,1]
	params['versor_params']      = lines[:,2:5]
	params['translation_params'] = lines[:,5:8]
	
	return params