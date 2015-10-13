# streamer
A C server and client for streaming over a local network

This project consists of a server and a client, from which one can start streaming videos. Both are implemented in C. The protocols used for the transmission are also implemented by hand (RTCP, RTSP, RTP).
Audio and video are sent over different connections, and are then sincronized at the client with using gstreamer's API (More info at: http://gstreamer.freedesktop.org/).
Both server and client have eight threads that are waiting on various mutexes for listening and sending over different protocols both audio and video.

To run the server, just make all and run ./servidor <port>, where <port> is the port number where it will be listening for inbound connections.
To run the client, type ./video, and in the emerging window you can enter the IP and port of the server, as well as the directory from which you want the server to send you the video and audio.
E.g. : 192.168.0.1 2055 dir, to obtain the video stored in the /dir folder
One advice when running the client: please push the buttons in the order they were meant to: Connect, setup and play. Otherwise the relevant ports and connections will not be reserved/established.

The gstreamer default buffer is only 64kb, which causes congestion when recieving after a set time and eventually make the screening impossible.
A future addition to this project is change the default buffer reserved by the OS and scale it properly.
Unfortunately the in depth documentation for this project is in spanish, as it was coded during the 2012-2013 academic year.
The rest can be found in the .pdf and "LEEME" files within this directory, as well as the doxygen documentation

Authors: M. Gordo and D. González

