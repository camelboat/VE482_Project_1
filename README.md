#VE482_Project_1_mumsh

## Goals of the project

- This is a shell wrote in C
- It can run in Linux and Minix 3

## How to use

- In Linux or Minix3 system, after use CMakeLists.txt to build the executable file (mumsh), please run it in the terminal by:

	`$ ./mumsh`

- If your shell doesn't get the authority to run, you can modify its authority by:

	`$ sudo chmod +x mumsh`

## Features
The mumsh supports the following features:
- Bash style redirection
- Pipe support
- CTRL-D and CTRL-C inerrupt
- `pwd` and `cd` built-in functions
- Support quotes
- Wait for command to be completed when detected uncomplete commands
- Correctly Handles erros
- Allow processes to run in the background

#### For other detailed information please refer to [p1.pdf](https://github.com/camelboat/VE482_Project_1/blob/master/p1.pdf).