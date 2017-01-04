#!/bin/bash

g++ -framework OpenGL -lglfw -lGLEW model_data.cpp shader.cpp main.cpp -o glutapp
