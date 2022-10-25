# Multipath RTP (MPRTP) protocol implementation

For more information about MPRTP protocol please refer [this document](https://datatracker.ietf.org/doc/html/draft-ietf-avtcore-mprtp-03).

## Using library

Please find examples of MPRTP: client/server examples/client.cpp and examples/server.cpp

### Creating MPRTP server

1. Create instance of ```mprtplib::receiver``` class
2. Use ```mprtplib::receiver::add_interface``` to add network interfaces to listen
3. In loop Call ```mprtplib::receiver::fetch_packet``` to get received packets
4. After all received packets are processed call ```mprtplib::receiver::poll()``` to listen for new packets

### Creating MPRTP client

1. Create your own class inherited from ```mprtplib::sender```
2. Implement ```mprtplib::sender::is_path_valid``` method. This method should return ```true``` if path from local interface to remote is valid.
3. Call ```mprtplib::sender::add_interface``` to add local interfaces into the client
4. Create and fill ```mprtplib::address``` vector with destination addresses.
5. Call ```mprtplib::sender::connect``` to create connection to the server
6. Call ```mprtplib::sender::send_data``` to send data to the server
7. You should call ```mprtplib::sender::poll``` to process sent data

# References

* [Source code](https://github.com/Trivium-Solutions/mprtplib)
* [Multipath RTP (MPRTP) draft-ietf-avtcore-mprtp-03](https://datatracker.ietf.org/doc/html/draft-ietf-avtcore-mprtp-03)
