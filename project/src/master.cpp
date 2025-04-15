//This file contains the code that the master process will execute.

#include <iostream>
#include <mpi.h>
#include <unistd.h>

#include "RayTrace.h"
#include "master.h"

void masterMain(ConfigData* data)
{
    //Depending on the partitioning scheme, different things will happen.
    //You should have a different function for each of the required 
    //schemes that returns some values that you need to handle.
    
    //Allocate space for the image on the master.
    float* pixels = new float[3 * data->width * data->height];
    
    //Execution time will be defined as how long it takes
    //for the given function to execute based on partitioning
    //type.
    double renderTime = 0.0, startTime, stopTime;

    //Print PID (for debugging)
    std::cout << "Master PID: " << getpid() << std::endl;

	//Add the required partitioning methods here in the case statement.
	//You do not need to handle all cases; the default will catch any
	//statements that are not specified. This switch/case statement is the
	//only place that you should be adding code in this function. Make sure
	//that you update the header files with the new functions that will be
	//called.
	//It is suggested that you use the same parameters to your functions as shown
	//in the sequential example below.
    switch (data->partitioningMode)
    {
        case PART_MODE_NONE:
            //Call the function that will handle this.
            startTime = MPI_Wtime();
            masterSequential(data, pixels);
            stopTime = MPI_Wtime();
            break;
        case PART_MODE_STATIC_STRIPS_VERTICAL:
            //Call the function that will handle this.
            startTime = MPI_Wtime();
            masterStaticCyclesVertical(data, pixels);
            stopTime = MPI_Wtime();
            break;
        case PART_MODE_STATIC_BLOCKS:
            //Call the function that will handle this.
            startTime = MPI_Wtime();
            masterStaticBlocks(data, pixels);
            stopTime = MPI_Wtime();
            break;
        default:
            std::cout << "This mode (" << data->partitioningMode;
            std::cout << ") is not currently implemented." << std::endl;
            break;
    }

    renderTime = stopTime - startTime;
    std::cout << "Execution Time: " << renderTime << " seconds" << std::endl << std::endl;

    //After this gets done, save the image.
    std::cout << "Image will be save to: ";
    std::string file = "renders/" + generateFileName();
    std::cout << file << std::endl;
    savePixels(file, pixels, data);

    //Delete the pixel data.
    delete[] pixels; 
}

void masterSequential(ConfigData* data, float* pixels)
{
    //Start the computation time timer.
    double computationStart = MPI_Wtime();

    //Render the scene.
    for( int i = 0; i < data->height; i++ )
    {
        for( int j = 0; j < data->width; j++ )
        {
            int row = i;
            int column = j;

            //Calculate the index into the array.
            int baseIndex = 3 * ( row * data->width + column );

            //Call the function to shade the pixel.
            shadePixel(&(pixels[baseIndex]),row,j,data);
        }
    }

    //Stop the comp. timer
    double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;

    //After receiving from all processes, the communication time will
    //be obtained.
    double communicationTime = 0.0;

    //Print the times and the c-to-c ratio
	//This section of printing, IN THIS ORDER, needs to be included in all of the
	//functions that you write at the end of the function.
    std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
    std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    double c2cRatio = communicationTime / computationTime;
    std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
}

void masterStaticBlocks(ConfigData* data, float* pixels)
{
    //Start the computation time timer.
    double computationStart = MPI_Wtime();

    //Calculate the number of pixels per block (X direction)
    int pixelsPerBlockX = data->width / data->mpi_procs;
    int pixelsPerBlockY = data->height / data->mpi_procs;
    //Need to handle remainder in case of odd sized widths or processor numbers
    int pixelsPerBlockRemainderX = data->width % data->mpi_procs;
    int pixelsPerBlockRemainderY = data->height % data->mpi_procs;

    //Distribute the parameters to each process
    ConfigData* blockData = new ConfigData[data->mpi_procs];
    for (int i = 0; i < data->mpi_procs; i++)
    {
        blockData[i] = *data;
        blockData[i].width = pixelsPerBlockX;
        blockData[i].height = pixelsPerBlockY;
        blockData[i].partitioningMode = PART_MODE_STATIC_BLOCKS;
        blockData[i].cycleSize = data->cycleSize;
        blockData[i].dynamicBlockWidth = data->dynamicBlockWidth;
        blockData[i].dynamicBlockHeight = data->dynamicBlockHeight;

        std::cout << "Process " << i << " will handle a block of size " << blockData[i].width << " x " << blockData[i].height << std::endl;

        //If there is a remainder, add the remainder to the width of the last process
        if (i == data->mpi_procs - 1 && pixelsPerBlockRemainderX > 0)
        {
            blockData[i].width += pixelsPerBlockRemainderX;
        }
        //If there is a remainder, add the remainder to the height of the last process
        if (i == data->mpi_procs - 1 && pixelsPerBlockRemainderY > 0)
        {
            blockData[i].height += pixelsPerBlockRemainderY;
        }
    }
    //Send the data to each process
    for (int i = 1; i < data->mpi_procs; i++)
    {
        MPI_Send(&blockData[i], sizeof(ConfigData), MPI_BYTE, i, 0, MPI_COMM_WORLD);
        MPI_Send(pixels, 3 * blockData[i].width * blockData[i].height, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
    }

    //The master process will handle the first strip
    //Render the scene.
    for( int i = 0; i < blockData[0].height; i++ )
    {
        for( int j = 0; j < blockData[0].width; j++ )
        {
            int row = i;
            int column = j;

            //Calculate the index into the array
            int baseIndex = 3 * ( row * data->width + column );
            //Call the function to shade the pixel.
            shadePixel(&(pixels[baseIndex]),row,j,data);
        }
    }
    //Receive the data from each process
    for (int i = 1; i < data->mpi_procs; i++)
    {
        std::cout << "Waiting for strip " << i << std::endl;
        //Receive the pixels from each process
        float blockPixels[3 * blockData[i].width * blockData[i].height];
        MPI_Recv(blockPixels, 3 * blockData[i].width * blockData[i].height, MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Received strip " << i << std::endl;

        //Consolidate the strips into one image, stored in the master process
        for (int row = (pixelsPerBlockY*(i+1)); row < blockData[i].height; row++)
        {   
            for (int column = (pixelsPerBlockX*(i+1)); column < blockData[i].width; column++)
            {
                int baseIndex = 3 * (row * blockData->width + column);
                pixels[baseIndex] = blockPixels[baseIndex];
            }
        }
    }

    //Stop the comp. timer
    double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;

    //After receiving from all processes, the communication time will
    //be obtained.
    double communicationTime = 0.0;

    //Print the times and the c-to-c ratio
    //This section of printing, IN THIS ORDER, needs to be included in all of the
    //functions that you write at the end of the function.
    std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
    std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    double c2cRatio = communicationTime / computationTime;
    std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
}


void masterStaticCyclesVertical(ConfigData* data, float* pixels)
{
    //Start the computation time timer.
    double computationStart = MPI_Wtime();

    //Calculate the number of pixels per vertical strip
    int pixelsPerStrip = data->width / data->mpi_procs;
    //Need to handle remainder in case of odd sized widths or processor numbers
    int pixelsPerStripRemainder = data->width % data->mpi_procs;

    int width[data->mpi_procs];
    int height[data->mpi_procs];

    //Distribute the parameters to each process
    for (int i = 0; i < data->mpi_procs; i++)
    {
        width[i] = pixelsPerStrip;
        height[i] = data->height;
        std::cout << "Process " << i << " will handle a strip of size " << width[i] << " x " << height[i] << std::endl;

        //If there is a remainder, add the remainder to the width of the last process
        if (i == data->mpi_procs - 1 && pixelsPerStripRemainder > 0)
        {
            width[i] += pixelsPerStripRemainder;
        }
    }
    //Send the data to each process
    for (int i = 1; i < data->mpi_procs; i++)
    {
        MPI_Send(&width[i], sizeof(int), MPI_BYTE, i, 0, MPI_COMM_WORLD);
        MPI_Send(&height[i], sizeof(int), MPI_BYTE, i, 0, MPI_COMM_WORLD);
        int pixels_size = 3 * (width[i] * height[i]);
        MPI_Send(pixels, pixels_size, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
    }

    //The master process will handle the first strip
    //Render the scene.
    for( int i = 0; i < height[0]; i++ )
    {
        for( int j = 0; j < width[0]; j++ )
        {
            int row = i;
            int column = j;

            //Calculate the index into the array.
            int baseIndex = 3 * ( row * data->width + column );
            std::cout << "baseIndex: " << baseIndex << std::endl; 

            //Call the function to shade the pixel.
            shadePixel(&(pixels[baseIndex]),row,j,data);
        }
    }

    //Receive the data from each process
    for (int i = 1; i < data->mpi_procs; i++)
    {
        //Receive the pixels from each process
        float stripPixels[3 * width[i] * height[i]];
        MPI_Recv(stripPixels, 3 * width[i] * height[i], MPI_FLOAT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Received strip " << i << std::endl;

        //Consolidate the strips into one image, stored in the master process
        for (int row = 0; row <= height[i]; row++)
        {   
            //std::cout << "Combining from " << (width[i-1]*i) << " to " << (width[i-1] + width[i]) << std::endl;
            for (int column = (width[i-1]*i); column < (width[i-1] + width[i]); column++)
            {
                //std::cout << "Row: " << row << " Column: " << column << std::endl;
                int stripBaseIndex = 3 * (row * (width[i]) + column);
                int pixelsBaseIndex = 3 * (row * data->width + column);
                pixels[pixelsBaseIndex] = stripPixels[stripBaseIndex];
                std::cout << "stripBaseIndex: " << stripBaseIndex << " pixelsBaseIndex: " << pixelsBaseIndex << std::endl;
            }
        }
    }

    //Stop the comp. timer
    double computationStop = MPI_Wtime();
    double computationTime = computationStop - computationStart;

    //After receiving from all processes, the communication time will
    //be obtained.
    double communicationTime = 0.0;

    //Print the times and the c-to-c ratio
    //This section of printing, IN THIS ORDER, needs to be included in all of the
    //functions that you write at the end of the function.
    std::cout << "Total Computation Time: " << computationTime << " seconds" << std::endl;
    std::cout << "Total Communication Time: " << communicationTime << " seconds" << std::endl;
    double c2cRatio = communicationTime / computationTime;
    std::cout << "C-to-C Ratio: " << c2cRatio << std::endl;
}
