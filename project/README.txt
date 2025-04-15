================================================================================
Description:
  To compile the program, first run the "spack env activate cmpe-655" command,
  and then simply run `make'. The usual options like `make clean' are also valid.
  This will generate 3 programs, called 'raytrace_seq', 'raytrace_mpi', and 
  'png_compare'. The first program is a sequential implementation; this has no 
  MPI dependencies. The second is an MPI capable program. Running the program 
  as follows will print a usage statement:

    ./raytrace_seq -help
	
  THIS IS THE ONLY TIME THAT YOU MAY RUN THIS ON THE LOGIN NODE!!!

  The final program is used to compare two png files. This will be useful for
  you to use to ensure that all of your images are identical for a given
  scene. If this program tells you that there are differences between the two
  inputs, then something is wrong. Make sure that you use an image that came 
  from running the sequential implementation as the reference image.
  
  Usage: ./png_compare <reference_image_path> <image_for_compare_path>
================================================================================  
SLURM
  
  An example script (runner_mpi.sh) has been provided. Change the command at
  the bottom of the script to render the different scenes. Some example commands 
  can be found at the end of this file.
  
  When specifying the number of threads to use, make sure to ONLY change the 
  value on the following line:
    #SBATCH -p kgcoe-mps -n <threads>

Submitting Jobs
  
  sbatch runner_mpi.sh
  
  It is good etiquette to ensure that your program works before submitting 
  many jobs.
  
Deleting Jobs

  scancel -u abc1234
  
  Where abc1234 is your username. This will cancel ALL of your jobs.
  
  If that does not work, try running orte-clean.

  If you wish to cancel a specific job, you can run:
  scancel <jobID>
  
Checking Job Status

  squeue -j <jobID>

  OR

  squeue -p kgcoe-mps
  
Queuing Multiple Items

  When submitting multiple jobs to the queue, the output file names will be 
  unique based on the job IDs. Feel free to change these as you see fit.
  
    # Standard out and Standard Error output files
    #SBATCH -o rt_mpi_<jobID>.out
    #SBATCH -e rt_mpi_<jobID>.err
	
  Note that all render jobs must be run via SLURM.
  
  Monitor your threads via squeue to make sure that
  your threads are not running on an oversubscribed node.
================================================================================
Example Commands:

  Render a 1200x1200 image of Turner Whitted's classic scene using strip
  partitioning and 5 processes:

    srun -n 5 raytrace_mpi -h 1200 -w 1200 -c configs/twhitted.xml -p static_strips_vertical

================================================================================
COMPLEX scene vs. SIMPLE scene:

  To render the complex scene, specify the configs/box.xml configuration file
  to the raytrace program, e.g.

    srun -n 1 raytrace_mpi -h 1000 -w 1000 -c configs/box.xml ...

  To render the simple scene, specify the configs/twhitted.xml configuration
  file, e.g.

    srun -n 1 raytrace_mpi -h 1000 -w 1000 -c configs/twhitted.xml ...

  You may wish to develop two separate job submission scripts for the complex and simple scenes,
  to avoid constantly changing things.

================================================================================
Files of interest:
  + src/main_mpi.cpp

    Contains the main program that is called when the application is launched.
    You will need to add some MPI code here.

  + src/slave.cpp

    Contains the main function for the slave processes.

  + src/master.cpp

    Contains the main function for the master process.

  + include/RayTrace.h

    Contains details about the functions provided to you and information about
    the ConfigData struct that you will need to use.

===============================================================================
Important Notes:
  All of the output has been provided for you. !This should be the only output
  that your program produces.! There are scripts that will be used for
  grading. If our scripts cannot parse your output because it has been
  changed, then points may be deducted for an incorrect implementation.

  There should be no checks in your MPI implementation for the number of 
  processes that are used. All of the implementations should work with an 
  arbitrary number of processes. That is, even the master should be doing work
  as the slaves are doing work. The only time that a check should be performed
  is when dynamic partitioning is used, which requires a minimum of 2
  processes.

===============================================================================
