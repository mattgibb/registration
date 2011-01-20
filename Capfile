require 'railsless-deploy'
# load    'config/deploy'

set :application, "registration"
set :repository, "git://github.com/mattgibb/registration.git"
set :scm, :git

role :clpcs, "work", "heart"
role :orac, "orac"
role :clusters, "sal"

set :remote_dir, "/users/matg/imaging/registration"
set :local_dir, "/Users/matthewgibb/Code/imaging/registration"

desc "Compile all the itk programs on the servers"
namespace :compile do
  task :default do
    orac
  end
  
  desc "Compile on orac"
  task :orac, :roles => :orac do
    run "cd registration/itk_build; make"
  end
end

desc "Pull changes to servers"
namespace :pull do
  task :default do
    orac
  end
  
  task :orac, :roles => :orac do
    run "cd registration; git pull"
  end
end

desc "Push changes to remote servers and hard reset the remote project"
namespace :push do
  task :default do
    heart
  end
  
  desc "push to heart server"
  task :heart do
    system "cd #{local_dir}; git push -f heart"
    run " cd #{remote_dir}; git reset --hard"
  end
  
  desc "push to clpc404"
  task :work do
    system "cd #{local_dir}; git push -f heart"
    run "cd #{remote_dir}; git reset --hard"
  end
  
end

# Extra stuff from Capistrano documentation
# perhaps run 'capify' and put code in config/deploy.rb...?
# set :deploy_to, "/var/www/#{application}"
# set :application, "set your application name here"
# set :repository,  "set your repository location here"
# 
# set :scm, :git
# 
# role :web, "your web-server here"                          # Your HTTP server, Apache/etc
# role :app, "your app-server here"                          # This may be the same as your `Web` server
# role :db,  "your primary db-server here", :primary => true # This is where Rails migrations will run
# role :db,  "your slave db-server here"

# namespace :deploy do
  # task :start, :roles => :app do ; end
# end
