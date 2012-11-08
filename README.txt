NetID skulkrn2 (Sachin Ramakanth Kulkarni)
NetID ellores2 (Abhishek Ellore Sreenath)

How to compile and run.

There are two sections to this code. The admission contact and the node. The two executables should not be run simultaneously on the same machine. 

Compiling admission contact :
In the folder admission_contact, type: 'make' to get a normal version of the binary
In the folder admission_contact, type: 'make debug' to get a debug version of the binary. This will print more output on the screen
In both above cases, you should get an executable called 'admission_contact' in the same folder.

Running admission contact :
In the folder admission_contact, type: ./admission_contact to run the admission contact


Compiling node :
In the folder ui, type: 'make' to get a normal version of the binary
In the folder ui, type: 'make debug' to get a debug version of the binary.
In both cases, you should get an executable called 'ui' in the same folder.

Running node :
In the folder ui, type: ./ui to run the node part of the code. 

//TODO : 
1) When IP address goes down, get the filenames, other IP where chunk is present and new IP where to replicate
2) Mutexs to be added
3) Writing of metadata to the file
4) Pushing metadata to neighbor