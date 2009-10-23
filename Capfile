role :servers, "heart"

set :root_dir, "/users/matg/imaging/downsampler"

desc "Compile all the itk programs on the servers"
task :compile do
  run "cd #{root_dir}/itk; make"
end

desc "Hard reset the remote project"
task :reset do
  run " cd #{root_dir}; git reset --hard"
end
