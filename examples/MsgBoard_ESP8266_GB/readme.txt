receive message from mqtt and display
// first byte in message is the dispaly mode 
// the following 4 bytes are display time
	// the rest bytes are data, two byte per character. first byte 0xaa are ascii characters.
	// for ascii chr, the value is ascii-32
	// all two-byte unit not starting with 0xaa, are 2-byte gb2312 encoding. 