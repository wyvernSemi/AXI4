import socket

def getmsg() :

  msgstr = ''
  while True :
     msg = s.recv(1)
     msgstr += msg.decode()
     if msg.decode() == '#' :
       break

  msg = s.recv(1)
  msgstr += msg.decode()
  msg = s.recv(1)
  msgstr += msg.decode()

  return msgstr

HOST = "localhost"  # The server's hostname or IP address
PORT = 49152  # The port used by the server

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

s.sendall("$M 80001c30, 4:ab56ce78#".encode())
data = getmsg()
print("Received " + data)

s.sendall("$m 80001c30,1#".encode())
data = getmsg()
print("Received " + data)

s.sendall("$D#".encode())
data = getmsg()
print("Received " + data)
