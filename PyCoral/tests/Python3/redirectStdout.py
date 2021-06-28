#!/usr/bin/env python
import os, sys, fcntl
import coral
print("Message #1")

# See http://www.parallelpython.com/component/option,com_smf/Itemid,29/topic,414.msg1225#msg1225
stdout = sys.__stdout__.fileno() # This fd is normally 1
stderr = sys.__stderr__.fileno() # This fd is normally 2
savestdout = os.dup(stdout)
savestderr = os.dup(stderr)
rpipe, wpipe = os.pipe()
os.dup2(wpipe, stdout)
os.dup2(wpipe, stderr)
coral.MessageStream("").setMsgVerbosity(3)
coral.ConnectionService().configuration()
print("Message #2")

os.dup2(savestdout, stdout)
os.dup2(savestderr, stderr)
os.close(wpipe)
print("Message #3")

fcntl.fcntl(rpipe, fcntl.F_SETFL, os.O_NONBLOCK)
rpipe = os.fdopen(rpipe,'r',0)
while True:
    try: cline = rpipe.readline()
    except: break # Stop reading if the pipe has been closed
    if cline == "" : break # Stop reading if EOF was read
    sys.stdout.write(cline)
rpipe.close()
