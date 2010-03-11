role :servers, "heart"

set :remote_dir, "/users/matg/imaging/downsampler"
set :local_dir, "/Users/matthewgibb/Documents/DPhil/code/imaging/downsampler"

desc "Compile all the itk programs on the servers"
task :compile do
  run "cd #{remote_dir}/itk; make"
end

desc "Push changes to remote server and hard reset the remote project"
task :push do
  `cd #{local_dir}; git push heart`
  run " cd #{remote_dir}; git reset --hard"
end
