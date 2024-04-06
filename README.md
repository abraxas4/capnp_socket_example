# capnp_socket_example

This repository contains the implementation of a socket example using Cap'n Proto for efficient data serialization. Before proceeding, ensure you have the proper tools and environment set up by referring to the following instructions:

[Setup Guide and Environment](https://blog.naver.com/abraxas4/223404205927)

## Prerequisites

The Cap'n Proto compiler has been used to generate the necessary `.bin`, `.h`, and `.c++` files. To compile `car_data.capnp`, execute the following command:  
`capnpc -oc++ car_data.capnp`

## Getting Started

To use this example, follow the steps below:

### Data Writing with Python

1. Run the `write_data.py` script to write serialized data:  
   `python write_data.py`

### Compiling and Running the C++ QT Application

2. Prepare the build directory:  
   `rm -r build`  
   // Remove the existing build directory

3. Configure the project using `cmake`, setting the appropriate paths for Qt and Cap'n Proto:  
   `cmake -B build . -DCMAKE_PREFIX_PATH="C:\Qt\6.6.0\msvc2019_64;C:\capnp\capnproto-c++-1.0.2\build"`  
   // Ensure to replace the paths with the ones corresponding to your environment

4. Compile the application:  
   `cmake --build build --config Release`

5. Run the executable:  
   `.\build\Release\car_data_display.exe`

Make sure to adjust any paths and versions to fit your local setup.

![image](https://github.com/abraxas4/capnp_socket_example/assets/11579758/f3609cd6-fd46-4e66-a456-542d020b77ac)


## Note to Users

As I am a beginner in using Git and managing repositories, the updates or responses to issues and pull requests may take a bit longer than expected. I appreciate your patience and understanding as I navigate through the learning process. Your contributions and feedback are highly valued and play a significant role in my journey to mastering version control with Git.

Thank you for your support!
