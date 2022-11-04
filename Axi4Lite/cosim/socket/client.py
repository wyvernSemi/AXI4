import socket

def getmsg(skt) :

  msgstr = ''
  while True :
     msg = skt.recv(1)
     msgstr += msg.decode()
     if msg.decode() == '#' :
       break

  msg = skt.recv(1)
  msgstr += msg.decode()
  msg = skt.recv(1)
  msgstr += msg.decode()

  return msgstr

#
# Calculates gdb packet checksum of string and returns as
# an ASCII hex bytes, minus the 0x prefix
#
def chksum(str) :
  checksum = 0

  for c in range(0, len(str)) :
    checksum = (checksum + ord(str[c])) % 256

  return hex(checksum)[2:]

#
# Constructs gdb packet from string argument and sends over socket 'skt'
#
def sendmsg (str, skt) :
  msg = '$' + str + '#' + chksum(str)
  skt.sendall (msg.encode())

if __name__ == "__main__":

  HOST = 'localhost' # The server's hostname or IP address
  PORT = 49152       # The port used by the server

  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  s.connect((HOST, PORT))

  sendmsg('M 80001c30, 4:ab56ce78', s)
  data = getmsg(s)
  print("Received " + data)

  sendmsg('m 80001c30,1', s)
  data = getmsg(s)
  print('Received ' + data)

  sendmsg('m 80001c31,1', s)
  data = getmsg(s)
  print('Received ' + data)

  sendmsg('m 80001c32,1', s)
  data = getmsg(s)
  print('Received ' + data)

  sendmsg('m 80001c33,1', s)
  data = getmsg(s)
  print('Received ' + data)

  sendmsg('D', s)
  data = getmsg(s)
  print('Received ' + data)
