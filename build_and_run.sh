#!/bin/bash

# Clean and build
cd build
cmake .. && make

# Run the executable from project root so relative asset paths work
cd ..
./build/Ortos_II