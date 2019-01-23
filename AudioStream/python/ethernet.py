from socket import *

def sendeth(ethernet_packet, payload, interface = "en7"):
  """Send raw Ethernet packet on interface."""
  s = socket(AF_PACKET, SOCK_RAW)

  # From the docs: "For raw packet
  # sockets the address is a tuple (ifname, proto [,pkttype [,hatype]])"
  s.bind((interface, 0))
  return s.send(ethernet_packet + payload)

def pack(byte_sequence):
  """Convert list of bytes to byte string."""
  return b"".join(map(chr, byte_sequence))

if __name__ == "__main__":
      # Note that this example contains HARDCODED packets, meaning that
      # it will ONLY work on the system it was designed for.

      # I got these values by sending a ping while running Wireshark.
      # You can do so yourself.  Another way to construct these manually is to use
      # the impacket library (sudo pip install impacket)

      # src=fe:ed:fa:ce:be:ef, dst=52:54:00:12:35:02, type=0x0800 (IP)
      ethernet_packet = [0x00, 0x60, 0x37, 0x12, 0x34, 0x56, 0x78, 0x7b, 0x8a,
                         0xad, 0xa5, 0x81, 0x08, 0x00]

      icmp_ping = [0x08, 0x00, 0x2b, 0x45, 0x11, 0x22, 0x00, 0x02, 0xa9, 0xf4, 0x5c,
                   0x53, 0x00, 0x00, 0x00, 0x00, 0xf5, 0x7b, 0x01, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                   0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
                   0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e,
                   0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37]

      # Construct Ethernet packet with an IPv4 ICMP PING request as payload
      r = sendeth(pack(ethernet_packet), pack(icmp_ping))
      print("Sent Ethernet w/IPv4 ICMP PING payload of length %d bytes" % r)
