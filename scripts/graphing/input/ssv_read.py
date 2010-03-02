from numpy import * 

def construct_params(params_array):
	params = {}
	params['level']              = params_array[:,0]
	params['iteration']          = params_array[:,1]
	params['metric_value']       = params_array[:,2]
	params['versor_params']      = params_array[:,3:6]
	params['translation_params'] = params_array[:,6:9]
	return params
	
def read_file(filename):
	# read registration output
	f = open(filename)
	lines = f.readlines()

	# clean up registration output
	lines = [line.split() for line in lines]
	
	# extract parameters from lines and return result
	return construct_params( asarray(lines) )