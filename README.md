# MiniDB: A Disk Based Object

For this homework I tried to implement an object store that writes any given struct to binary and can retrieve any object provided with the original key. This was meant to emulate a very crude form of a NOSQL database. 

The program essentially creates a hashtable in the miniDB.db file which stores address objects that contain references to general structs that are serialized to the file miniDB.db.heap.

The hashtable is entirely disk based, so pointers instead become positions in files, therefore I had to implement my own defreferencing functions and reimplement arrays.

While unfortunately I was not able to finish, I wanted to create indexes that allowed me to implement Lookup and Display properly and also be able to store objects back into data.csv the proper way they should have been.

Benefits to what I have created allow anytype of struct to be stored and retrieved, so a user can store a variety of obects in the database. In a way this is a disk based Heap.