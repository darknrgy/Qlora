Qlora is a LoRa relaying package, currently approaching mesh network behvaior. Multiple devices can be strung up across long distances to allow for simple text based communication.

- Nodes are either relay or terminals
- Nodes can be configured directly or remotely
- Relay nodes feature a lower power standby mode that uses 80mW (from a 16V battery pack -> buck converter)
- Extensive debug features, such as copious logging and ignoring nodes in confined environments
- Packet data is encrypted with a basic xor cypher + key that shifts deterministically with the packetId; the future plan is to use AES encryption
- Optional Wifi server to act as a terminal in the absence of serial port access

Planned updates include: 
- Refactor the application to use a state machine structure
- Add solar power (and associated software) to maintain continuous power for remote relays
- Fully implemented mesh networking versus a relay system with exceptions built in for mesh networking
