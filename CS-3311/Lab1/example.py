from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import OVSBridge
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.log import setLogLevel, info
from mininet.cli import CLI

from sys import argv

class SingleSwitchTopo(Topo):
    def build(self):
        switch0 = self.addSwitch('s0')

        host1 = self.addHost('h1', cpu=.25, mac='00:00:00:00:00:01')        
        host2 = self.addHost('h2', cpu=.25)
        host3 = self.addHost('h3', cpu=.25)
        host4 = self.addHost('h4', cpu=.25)
        host5 = self.addHost('h5', cpu=.25)
        
        self.addLink(host1, switch0, use_htb=True)        
        self.addLink(host2, switch0, use_htb=True)
        self.addLink(host1, switch0, use_htb=True)        
        self.addLink(host3, switch0, use_htb=True)
        self.addLink(host1, switch0, use_htb=True)        
        self.addLink(host4, switch0, use_htb=True)
        self.addLink(host1, switch0, use_htb=True)        
        self.addLink(host5, switch0, use_htb=True)

def Test():
    topo = SingleSwitchTopo()
    net = Mininet( topo=topo, host=CPULimitedHost, link=TCLink, autoStaticArp=False )
    net.start()    
    info( "Dumping host connections\n" )    
    dumpNodeConnections(net.hosts)    
    h1, h2, h3, h4, h5 = net.getNodeByName('h1', 'h2', 'h3', 'h4', 'h5')    
    s0= net.getNodeByName('s0')
    CLI(net)    
    net.stop()

if __name__ == '__main__':    
    # setLogLevel( 'debug' )    
    setLogLevel('info')    
    # Prevent test_simpleperf from failing due to packet loss   
    Test()
