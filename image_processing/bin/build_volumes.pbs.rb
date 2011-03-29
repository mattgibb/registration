#!/usr/bin/env ruby
require 'erb'

slice = ARGV[0]

template = ERB.new %Q{#!/bin/bash
#PBS -V
#PBS -l walltime=24:00:00
#PBS -l select=1:mpiprocs=8
#PBS -N build_volumes
#PBS -p 1023

cd $PBS_O_WORKDIR
~/registration/itk_build_sal/BuildVolumes Rat24 BuildVolumes <%= slice %>
}
    
puts template.result(binding)
