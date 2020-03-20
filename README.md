## Interview test program for Cinemo company
#### Write a C/C++ commandline application that encodes a set of WAV files to MP3          

Requirements:


(1) application is called with pathname as argument, e.g.

applicationname F:\MyWavCollection all WAV-files contained directly in that folder are to be encoded to MP3

(2) use all available CPU cores for the encoding process in an efficient way by utilizing multi-threading

(3) statically link to lame encoder library

(4) application should be compilable and runnable on Windows and Linux

(5) the resulting MP3 files are to be placed within the same directory as the source WAV files, the filename extension should be changed appropriately to .MP3

(6) non-WAV files in the given folder shall be ignored

(7) multithreading shall be implemented in a portable way, for example using POSIX pthreads.

(8) frameworks such as Boost or Qt shall not be used

(9) the LAME encoder should be used with reasonable standard settings (e.g. quality based encoding with quality level "good")

 
 Here is why I implemented this test this way.
 
Usually I can't afford to spend more than two hours on a test task but I spent to this task 5 hours including learning of wave format and lame library, adding MSVS support and testing on two platforms. Thus when I wrote the code I used the simplest approaches. You didn't write requirements about error handling system, support of internalization, code convention and so on, so I chose the simplest possible approaches everywhere. This is the cause why I didn't add a thread pool. Standard std::async implementation has its own mechanism of  thread management which doesn't allow to overload the system by too big number of threads. And more advanced mechanism of thread management was not required. 
