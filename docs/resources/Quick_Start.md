# AOCL-Cryptography Quick Starter for Linux

AOCL-Cryptography is a powerful and flexible cryptography library from AOCL.  
This guide is meant for people who want to get this library setup without going through all the nitty gritty of cmake, linux and prerequisite setup.

This guide will only focus on Ubuntu-24.04 as its a popular and desktop and server OS. In future this guide might get extended to other operating systems as well.

**Note: Below AOCL-Cryptography will be shortened to the repository name AOCL-Crypto.**


## Building, installing and running examples from source

### Installing Prerequisites
Some packages which are required for AOCL-Cryptography can be installed with below commands

```bash
sudo apt update                      # Sync repository information
sudo apt install build-essential     # Basic packages to support compilation
sudo apt install git                 # To clone github repositories
sudo apt install libssl-dev          # For openssl
sudo apt install make                # Build system
sudo apt install cmake               # Build system generator
sudo apt install p7zip-full          # Re-archive static libs
sudo apt install gcc-13 g++-13       # Compiler
```

What is given above should be sufficient for most users but if your OS did not come with a working install of `sudo`, you can install it by running `apt install sudo` as root. Make sure repository list is upto date.

### Cloning Repositories

There are mainly two repositories which are needed, one being `aocl-crypto` and the other which is a dependency named `aocl-utils`. `AOCL-Utils` provide the means to correctly identify the CPU. This is a necessary dependency to ensure optimal performance of `AOCL-Crypto`. Both of these repositories can be cloned with below command

```bash
git clone https://github.com/amd/aocl-crypto.git -b dev
git clone https://github.com/amd/aocl-utils.git  -b dev
```

Please ensure that you are running the above commands from a directory where you have write access.

### Building AOCL-Utils

Cloned repository has the latest released version of AOCL-Utils. AOCL-Utils is built using CMake build system generator. AOCL-Utils have a lot of configurations which you can enable/disable. Below command has what is minimum necessary to make it work optimally.

```bash
# Save current working directory
pushd .

# Setup build directory
cd aocl-utils
mkdir build
cd build

# Configure AOCL-Utils with make build system.
cmake ../ -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Release

# Build the project with all available threads.
make -j $(nproc --all)

# Install the project into build/install directory
make install

# Revert back to the original working directory.
popd
```

### Building AOCL-Cryptography

Cloned repository has the latest released version of AOCL-Cryptography. AOCL-Cryptography is built using CMake build system generator. AOCL-Utils have a lot of configurations which you can enable/disable. Below command has what is minimum necessary to make it work optimally.

```bash
# Save current working directory
pushd .

# Setup build directory
cd aocl-crypto
mkdir build
cd build

# Configure AOCL-Cryptography with make build system.
export CC=gcc-13; export CXX=g++-13
echo "Running \"cmake ../ -DALCP_ENABLE_EXAMPLES=ON \
-DOPENSSL_INSTALL_DIR=/usr \
-DCMAKE_INSTALL_PREFIX=$PWD/install \
-DENABLE_AOCL_UTILS=ON \
-DAOCL_UTILS_INSTALL_DIR=$(realpath $PWD/../../aocl-utils/build/install)\""
cmake ../ -DALCP_ENABLE_EXAMPLES=ON \
            -DOPENSSL_INSTALL_DIR=/usr \
            -DCMAKE_INSTALL_PREFIX=$PWD/install \
            -DENABLE_AOCL_UTILS=ON \
            -DAOCL_UTILS_INSTALL_DIR=$(realpath $PWD/../../aocl-utils/build/install)

# Build the project with all available threads.
make -j $(nproc --all)

# Install the project into build/install directory
make install

# Revert back to the original working directory.
popd
```

### Executing Examples

To execute the examples, loader has to find the library to load. For this both AOCL-Utils and AOCL-Cryptography's library path has to be added into `LD_LIBRARY_PATH`. Once this is added we can execute any executable which is compiled with AOCL-Cryptography. To show proof of the concept `aes-cfb` example can be executed.

```bash
# Save current working directory
pushd .

# Setup environment
export LD_LIBRARY_PATH=$PWD/aocl-utils/build/install/lib:$PWD/aocl-utils/build/install/lib64:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$PWD/aocl-crypto/build/install/lib:$PWD/aocl-crypto/build/install/lib64:$LD_LIBRARY_PATH

# Get into build directory
cd aocl-crypto/build

# Execute the CFB Example
$PWD/examples/cipher/aes-cfb

# Revert back to the original working directory.
popd
```

### Single script to do all the above

To end with, here is a script which runs everything. Feel free to save this script somewhere as a .sh file and execute it. As always please ensure you do have write access to the current working directory.

```bash
#!/usr/bin/env bash

# This file is supposed to be a guide to compile aocl-crypto with examples 
# from source.
# It should only require minimal interaction from user.
# All functions in this file should be straight forward and minimal.
# For detailed info please take a look at BUILD.md located in the root of 
# AOCL-Cryptography source code directory.

# Global Variables to be modifed depending on repo location
AOCL_CRYPTO_REPO="https://github.com/amd/aocl-crypto.git"
AOCL_UTILS_REPO="https://github.com/amd/aocl-utils.git"
AOCL_BRANCH="dev"

# Function to exit with an error if some execution failed
quit_if_status_not_zero(){
    if [ $1 -ne 0 ]; then
        echo "Command returned error"
        exit -1 
    fi
}

# Function to install all packages, OS indipendant (eventually)
ensure_packages(){
    detect_ubuntu 24.04
    if [ $? -eq 0 ]; then
        echo "Running \"sudo apt update\""
        sudo apt update                      # Sync repository information
        echo "Running \"apt install build-essential\""
        sudo apt install build-essential     # Basic packages to support compilation
        quit_if_status_not_zero $?
        echo "Running \"sudo install git\""
        sudo apt install git                 # To clone github repositories
        quit_if_status_not_zero $?
        echo "Running \"sudo apt install libssl-dev\""
        sudo apt install libssl-dev          # For openssl
        quit_if_status_not_zero $?
        echo "Running \"sudo apt install make\""
        sudo apt install make                # Build system
        quit_if_status_not_zero $?
        echo "Running \"sudo apt install cmake\""
        sudo apt install cmake               # Build system generator
        quit_if_status_not_zero $?
        echo "Running \"sudo apt install p7zip-full\""
        sudo apt install p7zip-full          # Re-archive static libs
        quit_if_status_not_zero $?
        echo "Running \"sudo apt install gcc-12 g++-12\""
        sudo apt install gcc-12 g++-12       # Compiler
        quit_if_status_not_zero $?
        return 0
    fi
    # detect_rhel 8
    # if [ $? -eq 1 ]; then
    #    sudo yum install...
    #    ...
    #    return 1
    echo "OS support check failed!"
    exit -1
}

# Function to make sure what this script writes don't already exist
ensure_no_directory_conflict(){
    # Check if aocl-crypto directory already exists
    if [[ -d aocl-crypto ||  -f aocl-crypto ]]; then
        echo "aocl-crypto exists!"
        echo "Please run \"rm -rf aocl-crypto\""
        exit -1
    fi
    # Check if aocl-utils directory already exists
    if [[ -d aocl-utils || -f aocl-utils ]]; then
        echo "aocl-utils exists!"
        echo "Please run \"rm -rf aocl-utils\""
        exit -1
    fi
}

# Function to clone the repo both aocl-utils and aocl-crypto.
clone_repos(){

    # Clone AOCL-Cryptography
    echo "Running \"git clone $AOCL_CRYPTO_REPO -b $AOCL_BRANCH\""
    git clone $AOCL_CRYPTO_REPO -b $AOCL_BRANCH
    quit_if_status_not_zero $?

    sleep 1

    # Clone AOCL-Utils
    echo "Running \"git clone $AOCL_UTILS_REPO -b $AOCL_BRANCH\""
    git clone $AOCL_UTILS_REPO -b $AOCL_BRANCH
    quit_if_status_not_zero $?

}

# Function to build aocl-utils with minimal configuration
compile_aocl_utils(){

    pushd .
    echo "cd into aocl-utils"
    cd aocl-utils
    echo "creating build directory"
    mkdir build
    echo "cd into build directory"
    cd build
    echo "Setting GCC-13 as the compiler"
    export CC=gcc-13; export CXX=g++-13
    echo "Running \"cmake ../ -DCMAKE_INSTALL_PREFIX=$PWD/install -DCMAKE_BUILD_TYPE=Release -DALCI_DOCS=OFF\""
    cmake ../ -DCMAKE_INSTALL_PREFIX=install -DCMAKE_BUILD_TYPE=Release -DALCI_DOCS=OFF
    echo "Running \"make -j $(nproc --all)\""
    make -j $(nproc --all)
    quit_if_status_not_zero $?
    make install
    quit_if_status_not_zero $?
    popd

}

# Function to build aocl-crypto with minimal configuration
compile_aocl_crypto(){
    
    pushd .
    echo "cd into aocl-crypto"
    cd aocl-crypto
    echo "creating build directory"
    mkdir build
    echo "cd into build directory"
    cd build
    echo "Setting GCC-13 as the compiler"
    export CC=gcc-13; export CXX=g++-13
    echo "Running \"cmake ../ -DALCP_ENABLE_EXAMPLES=ON \
-DOPENSSL_INSTALL_DIR=/usr \
-DCMAKE_INSTALL_PREFIX=$PWD/install \
-DENABLE_AOCL_UTILS=ON \
-DAOCL_UTILS_INSTALL_DIR=$(realpath $PWD/../../aocl-utils/build/install)\""
    cmake ../ -DALCP_ENABLE_EXAMPLES=ON \
              -DOPENSSL_INSTALL_DIR=/usr \
              -DCMAKE_INSTALL_PREFIX=$PWD/install \
              -DENABLE_AOCL_UTILS=ON \
              -DAOCL_UTILS_INSTALL_DIR=$(realpath $PWD/../../aocl-utils/build/install)
    echo "Running \"make -j $(nproc --all)\""
    make -j $(nproc --all)
    quit_if_status_not_zero $?
    make install
    quit_if_status_not_zero $?
    popd

}

# Function to show how to execute an example properly
run_example_cfb(){

    pushd .
    echo "Exporting library paths for loader"
    # Update loader with aocl-utils lib
    export LD_LIBRARY_PATH=$PWD/aocl-utils/build/install/lib:$PWD/aocl-utils/build/install/lib64:$LD_LIBRARY_PATH
    # Update loader with aocl-crypto lib
    export LD_LIBRARY_PATH=$PWD/aocl-crypto/build/install/lib:$PWD/aocl-crypto/build/install/lib64:$LD_LIBRARY_PATH
    echo "cd into aocl-crypto/build"
    cd aocl-crypto/build
    echo "Executing \"$PWD/examples/cipher/aes-cfb\""
    $PWD/examples/cipher/aes-cfb
    quit_if_status_not_zero $?
    echo "Executed Successfully!, output above"
    popd

}

# Make sure we dont destroy anything
ensure_no_directory_conflict
# Make sure all the needed packages (dependancies) are installed
ensure_packages
# Clone Utils and Crypto
clone_repos
# Build Utils and Install it into a prefix inside build directory
compile_aocl_utils
# Build Crypto and Install it into a prefix inside the build directory
compile_aocl_crypto
# Run an example to show that, its indeed working.
run_example_cfb
```

## Setting up Application Development Environment

As AOCL-Cryptography is a C/C++ library you can write a C/C++ application with AOCL-Cryptography and accelerate the cryptographic performance. 

AOCL-Cryptography interacts with applications using C-API. In future for C++ a native API will be exposed reducing the complexity and increasing the flexibility of the API.

### Exporting Environment Variables
```bash
# Adding AOCL-Utils into environment variables
export LD_LIBRARY_PATH="aocl-utils/build/install/lib:aocl-utils/build/install/lib64:$LD_LIBRARY_PATH"
export LIBRARY_PATH="aocl-utils/build/install/lib:aocl-utils/build/install/lib64:$LIBRARY_PATH"
export C_INCLUDE_PATH="aocl-utils/build/install/include:$C_INCLUDE_PATH"
export CPLUS_INCLUDE_PATH="aocl-utils/build/install/include:$CPLUS_INCLUDE_PATH"

# Adding AOCL-Cryptography into environment variables
export LD_LIBRARY_PATH="aocl-crypto/build/install/lib:aocl-crypto/build/install/lib64:$LD_LIBRARY_PATH"
export LIBRARY_PATH="aocl-crypto/build/install/lib:aocl-crypto/build/install/lib64:$LIBRARY_PATH"
export C_INCLUDE_PATH="aocl-crypto/build/install/include:$C_INCLUDE_PATH"
export CPLUS_INCLUDE_PATH="aocl-crypto/build/install/include:$CPLUS_INCLUDE_PATH"
```

### Sample application to show version

Copy and paste this into a terminal, it will create a file named `version.c`
```bash
cat << EOF > version.c
#include <alcp/alcp.h>
#include <stdio.h>

int
main()
{
    // Call alcp_get_version function which returns a string
    printf("ALCP_VERSION_IS: %s\n", alcp_get_version());
    return 0;
}
EOF
```

#### Compiling the application

As you have already run the `Exporting Environment Variables` code snippet, building becomes very simple.

```bash
gcc version.c -o version -lalcp -laoclutils
```

#### Execute the application

```bash
./version
```

### Single script to setup the example application

As always to end with, here is a script which builds and executes application. Feel free to modify it as you please for your own application.

```bash
# Adding AOCL-Utils into environment variables
export LD_LIBRARY_PATH="aocl-utils/build/install/lib:aocl-utils/build/install/lib64:$LD_LIBRARY_PATH"
export LIBRARY_PATH="aocl-utils/build/install/lib:aocl-utils/build/install/lib64:$LIBRARY_PATH"
export C_INCLUDE_PATH="aocl-utils/build/install/include:$C_INCLUDE_PATH"
export CPLUS_INCLUDE_PATH="aocl-utils/build/install/include:$CPLUS_INCLUDE_PATH"

# Adding AOCL-Cryptography into environment variables
export LD_LIBRARY_PATH="aocl-crypto/build/install/lib:aocl-crypto/build/install/lib64:$LD_LIBRARY_PATH"
export LIBRARY_PATH="aocl-crypto/build/install/lib:aocl-crypto/build/install/lib64:$LIBRARY_PATH"
export C_INCLUDE_PATH="aocl-crypto/build/install/include:$C_INCLUDE_PATH"
export CPLUS_INCLUDE_PATH="aocl-crypto/build/install/include:$CPLUS_INCLUDE_PATH"

# Write the sample Application
cat << EOF > version.c
#include <alcp/alcp.h>
#include <stdio.h>

int
main()
{
    // Call alcp_get_version function which returns a string
    printf("ALCP_VERSION_IS: %s\n", alcp_get_version());
    return 0;
}
EOF

# Compile the application
gcc version.c -o version -lalcp -laoclutils

# Execute the application
./version
```

Now you got AOCL-Cryptography setup for development, Enjoy :-).