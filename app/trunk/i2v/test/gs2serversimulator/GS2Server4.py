import time
import wx
import socket
import threading
import re
import sys
import wx.lib.masked as masked

# Button definitions
ID_START = wx.NewId()

# Define notification event for thread completion
EVT_RESULT_ID = wx.NewId()

def EVT_RESULT(win, func):
    """Define Result Event."""
    win.Connect(-1, -1, EVT_RESULT_ID, func)

class ResultEvent(wx.PyEvent):
    """Simple event to carry arbitrary result data."""
    def __init__(self, data):
        """Init Result Event."""
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_RESULT_ID)
        self.data = data

class WorkerThread(threading.Thread):
    """Worker Thread Class."""
    def __init__(self, notify_window, gs2IPAddress, rseIPAddress):
        """Init Worker Thread Class."""
        threading.Thread.__init__(self)
        self.gs2IPAddress = gs2IPAddress
        self.rseIPAddress = rseIPAddress
        self.port = 9090        # Arbitrary non-privileged port
        self._notify_window = notify_window
        self._want_abort = 0
        self.clientThreads = []
        # This starts the thread running on creation, but you could
        # also make the GUI thread responsible for calling this
        self.start()

    def run(self):
        """Run Worker Thread."""
        socket.setdefaulttimeout(0.100)
        self.s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.s.bind((self.gs2IPAddress, self.port))
        wx.PostEvent(self._notify_window, ResultEvent('Receiving on port '+ str(self.port) + '\n'))
        while self._want_abort == 0:
            try:
                # receive
                data, address = self.s.recvfrom(1024)
            except socket.timeout:
                continue
            except socket.error:
                break
                
            if data: 
                wx.PostEvent(self._notify_window, ResultEvent(data + '\n'))

    def send(self, msg):
	self.s.sendto(msg+";", (self.rseIPAddress, self.port))

    def abort(self):
        """abort worker thread."""
        # Method for use by main thread to signal an abort
        self._want_abort = 1

# GUI Frame class that spins off the worker thread
class MainFrame(wx.Frame):
    """Class MainFrame."""
    def __init__(self, parent, id):
        """Create the MainFrame."""
        wx.Frame.__init__(self, parent, id, 'GS2 Server Simulator')

        panel = wx.Panel(self, -1)

        vbox = wx.BoxSizer(wx.VERTICAL)

        hbox0 = wx.BoxSizer(wx.HORIZONTAL)
        self.gs2IPAddressLabel = wx.StaticText(panel, 0, label='GS2 IP:')
        self.gs2IPAddress = masked.IpAddrCtrl(panel, -1, mask="###.###.###.###")
        self.gs2IPAddress.SetValue("0.0.0.0")
        self.gs2IPAddressLabel2 = wx.StaticText(panel, 0, label='0.0.0.0 means all interfaces')
        hbox0.Add(self.gs2IPAddressLabel, 0, wx.ALL, 2)
        hbox0.Add(self.gs2IPAddress, 0, wx.ALL, 2)
        hbox0.Add(self.gs2IPAddressLabel2, 0, wx.ALL, 2)
        vbox.Add(hbox0, 0, wx.ALL|wx.EXPAND, 2)

        hbox1 = wx.BoxSizer(wx.HORIZONTAL)
        self.rseIPAddressLabel = wx.StaticText(panel, 0, label='RSE IP:')
        self.rseIPAddress = masked.IpAddrCtrl(panel, -1, mask="###.###.###.###")
        self.rseIPAddress.SetValue("192.168.1.30")
        hbox1.Add(self.rseIPAddressLabel, 0, wx.ALL, 2)
        hbox1.Add(self.rseIPAddress, 0, wx.ALL, 2)
        vbox.Add(hbox1, 0, wx.ALL|wx.EXPAND, 2)

        hbox2 = wx.BoxSizer(wx.HORIZONTAL)
        self.startButton = wx.Button(panel, 0, label='Start')
        hbox2.Add(self.startButton, 0, wx.ALL, 2)
        vbox.Add(hbox2, 0, wx.ALL|wx.EXPAND, 2)

        hbox3 = wx.BoxSizer(wx.HORIZONTAL)
        self.obstructedLabel = wx.StaticText(panel, 0, label='Obstructed:')
        self.obstructedCheckBox1 = wx.CheckBox(panel, 0, label='1')
        self.obstructedCheckBox2 = wx.CheckBox(panel, 0, label='2')
        self.obstructedCheckBox3 = wx.CheckBox(panel, 0, label='3')
        self.obstructedCheckBox4 = wx.CheckBox(panel, 0, label='4')
	self.obstructedCheckBox1.Disable()
	self.obstructedCheckBox2.Disable()
	self.obstructedCheckBox3.Disable()
	self.obstructedCheckBox4.Disable()
        hbox3.Add(self.obstructedLabel, 0, wx.ALL, 2)
        hbox3.Add(self.obstructedCheckBox1, 0, wx.ALL, 2)
        hbox3.Add(self.obstructedCheckBox2, 0, wx.ALL, 2)
        hbox3.Add(self.obstructedCheckBox3, 0, wx.ALL, 2)
        hbox3.Add(self.obstructedCheckBox4, 0, wx.ALL, 2)
        vbox.Add(hbox3, 0, wx.ALL|wx.EXPAND, 2)

        hbox4 = wx.BoxSizer(wx.HORIZONTAL)
        self.wrongWayLabel = wx.StaticText(panel, 0, label='Wrong Way:')
        self.wrongWayCheckBox = wx.CheckBox(panel, 0, 'Detected')
	self.wrongWayCheckBox.Disable()
        hbox4.Add(self.wrongWayLabel ,0, wx.ALL, 2)
        hbox4.Add(self.wrongWayCheckBox,0, wx.ALL, 2)
        vbox.Add(hbox4, 0, wx.ALL|wx.EXPAND, 2)

        hbox5 = wx.BoxSizer(wx.HORIZONTAL)
        self.dilemmaZoneLabel = wx.StaticText(panel, 0, label='Dilemma Zone Detect:')
        self.dilemmaZoneStatus = wx.StaticText(panel, 0, label='No')
        hbox5.Add(self.dilemmaZoneLabel ,0, wx.ALL, 2)
        hbox5.Add(self.dilemmaZoneStatus ,0, wx.ALL, 2)
        vbox.Add(hbox5, 0, wx.ALL|wx.EXPAND, 2)

        hbox6 = wx.BoxSizer(wx.HORIZONTAL)
        self.log  = wx.TextCtrl(panel, -1, style=wx.TE_MULTILINE, size=(400,600))
        hbox6.Add(self.log, 1, wx.EXPAND)
        vbox.Add(hbox6, 1, wx.ALL|wx.EXPAND, 2)
	panel.SetSizerAndFit(vbox)

        panel.SetSizer(vbox)
	frameSizer = wx.BoxSizer(wx.VERTICAL)
	frameSizer.Add(panel, 1, wx.ALL|wx.EXPAND, 2)
	self.SetSizerAndFit(frameSizer)

        self.startButton.Bind(wx.EVT_BUTTON, self.OnStart)
        self.wrongWayCheckBox.Bind(wx.EVT_CHECKBOX, self.OnWrongWay)
	self.Bind(wx.EVT_CLOSE, self.OnClose)

	self.timer = wx.Timer(self)
	self.Bind(wx.EVT_TIMER, self.OnIntersectionStatusUpdate, self.timer)

        # Set up event handler for any worker thread results
        EVT_RESULT(self,self.OnResult)

        # And indicate we don't have a worker thread yet
        self.worker = None

	self.sequenceID = 0

    def OnStart(self, evennt):
        self.worker = WorkerThread(self, self.gs2IPAddress.GetAddress(), self.rseIPAddress.GetAddress())
	self.timer.Start(100)
	self.obstructedCheckBox1.Enable()
	self.obstructedCheckBox2.Enable()
	self.obstructedCheckBox3.Enable()
	self.obstructedCheckBox4.Enable()
	self.wrongWayCheckBox.Enable()

    def OnIntersectionStatusUpdate(self, event):
	if self.worker:
	    obstructedStatus = repr(int(self.obstructedCheckBox1.GetValue() == True)) + ","
	    obstructedStatus = obstructedStatus + repr(int(self.obstructedCheckBox2.GetValue() == True)) + ","
	    obstructedStatus = obstructedStatus + repr(int(self.obstructedCheckBox3.GetValue() == True)) + ","
	    obstructedStatus = obstructedStatus + repr(int(self.obstructedCheckBox4.GetValue() == True))
            self.worker.send("INS:"+repr(self.sequenceID)+"/OB:"+obstructedStatus)
            self.sequenceID = self.sequenceID + 1

    def OnWrongWay(self, event):
        if self.worker:
            if self.wrongWayCheckBox.GetValue() == True:
            	self.worker.send("WWS:"+repr(self.sequenceID)+"/WS:x,y,z")
	    else:
            	self.worker.send("WWS:"+repr(self.sequenceID)+"/WC:a")
            self.sequenceID = self.sequenceID + 1

    def OnClose(self, event):
        """Stop Worker thread."""
        # Flag the worker thread to stop if running
        if self.worker:
            try:
            	self.worker.abort()
	    except:
		pass
        self.Destroy()

    def OnResult(self, event):
        """Show Result status."""
        if event.data is None:
            # Thread aborted (using our convention of None return)
            self.worker = None
        else:
            parameters = []
            self.log.AppendText(event.data)
            firstPassTokens = re.split('/|;', event.data)
            print firstPassTokens
            for p in firstPassTokens:
                parameters.append(re.split(':', p))
            print parameters
            if parameters[0][0] == "DZS":
            	self.worker.send("ACK:"+parameters[0][1])
                self.dilemmaZoneStatus.SetLabel('Yes')
            elif parameters[0][0] == "DZC":
            	self.worker.send("ACK:"+parameters[0][1])
                self.dilemmaZoneStatus.SetLabel('No')



class MainApp(wx.App):
    """Class Main App."""
    def OnInit(self):
        """Init Main App."""
        self.frame = MainFrame(None, -1)
        self.frame.Show(True)
        self.SetTopWindow(self.frame)
        return True

if __name__ == '__main__':
    app = MainApp(0)
    app.MainLoop()
