role :servers, "heart"

set :remote_dir, "/users/matg/imaging/downsampler"

desc "Compile all the itk programs on the servers"
task :compile do
  run "cd #{remote_dir}/itk; make"
end

desc "Hard reset the remote project"
task :reset do
  run " cd #{remote_dir}; git reset --hard"
end
