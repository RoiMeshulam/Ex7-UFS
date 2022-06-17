# Authors
Arik Tatievski

Roi Meshulam

# What is this project?

This project is divided into two parts:

Part A - A program that make our own UFS and the user can decide what disk size he wants.

Part B - myFILE implication (Identical to FILE we all know but with our implication)

# The way we made our project 

*Part A:*

First of all we need to allocate space for our disk using mymkfs(Getting the size) and mymount(Allocates the space and creates a file which will be our disk and gives us a 'root' for the rest of the program).

We need to control which directories go under the root and how to address them using myopen,myopenDIR,myclose,mycloseDIR

Now we can open files under the root, so we implicate controlling the files using mywrite, myread, mylseek(NOTICE IT WILL NOT WORK ON A FILE THAT IS NOT IN myopenfile - 'our storage')

Finally, we can get a view of our storage using myreaddir.

*Part B:*

If you followed so far, you will notice that all of Part A is basically a template for a FILE implication.

So all we do is make our own file (myFILE) using everything from Part A

# How to use our project:

Run bash command `make all` in order to compile the project.

It will generate our shared libraries :  libmylibc.so , libmyfs.so .

# Extras
You can also write in a bash environment `make test` and run the `./myfs_test` to run a check for Part A and `./mystdio_test` to run a check for Part B

# Hope you find good usuage of this project!
