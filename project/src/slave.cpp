//This file contains the code that the master process will execute.

#include <iostream>
#include <mpi.h>
#include <unistd.h>
#include "RayTrace.h"
#include "slave.h"

void slaveMain(ConfigData* data)
{
    //Print PID (for debugging)
    std::cout << "Slave " << data->mpi_rank << " PID: " << getpid() << std::endl;
    //Depending on the partitioning scheme, different things will happen.
    //You should have a different function for each of the required 
    //schemes that returns some values that you need to handle.
    switch (data->partitioningMode)
    {
        case PART_MODE_STATIC_STRIPS_VERTICAL:
            slaveStaticStripsVertical(data);
            break;
        case PART_MODE_STATIC_BLOCKS:
            slaveStaticBlocks(data);
            break;
        case PART_MODE_NONE:
            //The slave will do nothing since this means sequential operation.
            break;
        default:
            std::cout << "This mode (" << data->partitioningMode;
            std::cout << ") is not currently implemented." << std::endl;
            break;
    }
}

void slaveStaticStripsVertical(ConfigData* data)
{
    //Receive the data from the master process
    int width = 0;
    int height = 0;
    //Receive the width and height of the strip from the master process
    MPI_Recv(&width, sizeof(int), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&height, sizeof(int), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    int offset = 0;
    MPI_Recv(&offset, sizeof(int), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    std::cout << "Slave " << data->mpi_rank << " offset " << offset << std::endl;
    //Allocate space for the image on the slave
    //float pixels[data->width * data->height];
    float pixels[3 * width * height];
    //Receive the pixels from the master process
    MPI_Recv(pixels, 3 * width * height, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    std::cout << "Slave " << data->mpi_rank << " received strip of size " << width << " x " << height << std::endl;
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;
    //Render the scene
    for( int i = 0; i < height; ++i )
    {
        for( int j = 0; j < width; ++j )
        {
            int row = i;
            int column = j;

            //Calculate the index into the array.
            int baseIndex = 3 * ( row * width + column );

            //Call the function to shade the pixel.
            shadePixel(&(pixels[baseIndex]),row,column + offset,data);
        }
    }
    //Send the pixels back to the master process
    MPI_Send(pixels, 3 * width * height, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    std::cout << "Slave " << data->mpi_rank << " sent strip back to master" << std::endl;
}

void slaveStaticBlocks(ConfigData* data)
{
    //Receive the data from the master process
    ConfigData blockData;
    MPI_Recv(&blockData, sizeof(ConfigData), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //Allocate space for the image on the slave
    float pixels[3 * blockData.width * blockData.height];
    //Receive the pixels from the master process
    MPI_Recv(pixels, 3 * blockData.width * blockData.height, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    std::cout << "Slave " << data->mpi_rank << " received block of size " << blockData.width << " x " << blockData.height << std::endl;
    //Render the scene
    for( int i = 0; i < blockData.height; ++i )
    {
        //std::cout << "Slave " << data->mpi_rank << " rendering row " << i << std::endl;
        for( int j = 0; j < blockData.width; ++j )
        {
            int row = i;
            int column = j;

            //Calculate the index into the array.
            int baseIndex = 3 * ( row * blockData.width + column );
            if(j > blockData.width - 2)
            {
                std::cout << "Slave " << data->mpi_rank << " rendering row " << i << " column " << j << std::endl;
                std::cout << "Base index: " << baseIndex << std::endl;
                std::cout << "Width: " << blockData.width << std::endl;
            }
            //Call the function to shade the pixel.
            shadePixel(&(pixels[baseIndex]),row,column,data);
        }
    }
    //Send the pixels back to the master process
    MPI_Send(&pixels, 3 * blockData.width * blockData.height, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    std::cout << "Slave " << data->mpi_rank << " sent block back to master" << std::endl;
}