#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-dnssec-module.h"
#include "ns3/mobility-module.h"
#include <fstream>
#include <arpa/inet.h>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("DceDnsReflection");

bool m_delay = false;
uint32_t nNodes = 20;
uint32_t max_pkts = 1000;

void setPos (Ptr<Node> n, int x, int y, int z)
{
  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
}

/*
 * DNSSEC query
15:57:08.714120 IP 133.11.123.54.53398 > 133.11.124.164.53: 59758+ [1au] A? isc.org. (36)
        0x0000:  001e 7a82 de00 782b cb73 2643 0800 4500  ..z...x+.s&C..E.
        0x0010:  0040 5a94 0000 4011 1e28 850b 7b36 850b  .@Z...@..(..{6..
        0x0020:  7ca4 d096 0035 002c 022f e96e 0100 0001  |....5.,./.n....
        0x0030:  0000 0000 0001 0369 7363 036f 7267 0000  .......isc.org..
        0x0040:  0100 0100 0029 1000 0000 8000 0000       .....)........
e96e is the head of UDP payload (DNS)

u8 [] =   0xe9, 0x6e, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x69, 0x73, 0x63, 0x03, 0x6f, 0x72, 0x67, 0x00, 0x00,
  0x01, 0x00, 0x01, 0x00, 0x00, 0x29, 0x10, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00


023c 0120  .......5.7$..<..
        0x0020:  0001 0000 0000 0001 076e 7333 2d64 6e73  .........ns3-dns
        0x0030:  0477 6964 6502 6164 026a 7000 0001 0001  .wide.ad.jp.....
        0x0040:  0000 2910 0000 0080 0000 00   
uint8_t dnssec_query[47] = {
  0x02, 0x3c, 0x01, 0x20,
  0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x6e, 0x73, 0x33, 0x2d, 0x64, 0x6e, 0x73,
  0x04, 0x77, 0x69, 0x64, 0x65, 0x02, 0x61, 0x64, 0x02, 0x6a, 0x70, 0x00, 0x00, 0x01, 0x00, 0x01,
  0x00, 0x00, 0x29, 0x10, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00
};

09:00:01.000000 IP 10.0.0.2.13116 > 10.0.0.1.53: 57573+% [1au] A? mail.example.j
p. (44)
        0x0000:  0021 4500 0048 09f3 0000 4011 5cb0 0a00  .!E..H....@.\...
        0x0010:  0002 0a00 0001 333c 0035 0034 1448 e0e5  ......3<.5.4.H..
        0x0020:  0110 0001 0000 0000 0001 046d 6169 6c07  ...........mail.
        0x0030:  6578 616d 706c 6502 6a70 0000 0100 0100  example.jp......
        0x0040:  0029 1000 0000 8000 0000                 .)........
uint8_t dnssec_query[44] = {
0xe0, 0xe5,
0x01, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x6d, 0x61, 0x69, 0x6c, 0x07,
0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x02, 0x6a, 0x70, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
0x00, 0x29, 0x10, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00
};

*/

uint8_t dnssec_query[44] = {
0xe0, 0xe5,
0x01, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x6d, 0x61, 0x69, 0x6c, 0x07,
0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x02, 0x6a, 0x70, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
0x00, 0x29, 0x10, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00
};

void SendSpoofDNS (Ptr<Node> node, Ipv4Address dst)
{
  Ptr<Socket> socket;
  socket = node->GetObject<Socket> ();
  if (!socket)
    {
      socket = Socket::CreateSocket (node, LinuxIpv4RawSocketFactory::GetTypeId ());
      node->AggregateObject (socket);
    }

  Ptr<LinuxSocketImpl> linux_sock = StaticCast<LinuxSocketImpl> (socket);
  int hincl = 1;
  linux_sock->Setsockopt (IPPROTO_IP, IP_HDRINCL, &hincl, sizeof (hincl));

  //  socket->Bind ();
  socket->Connect (InetSocketAddress (dst));

  //  socket->SetAttribute ("IpHeaderInclude", BooleanValue (true));
  //  Ptr<Packet> p = Create<Packet> (dnssec_query, sizeof (dnssec_query));
  Ptr<Packet> p = Create<Packet> ();
  QuestionSectionHeader dnsq;
  dnsq.SetQname ("www.example.jp.");
  dnsq.SetQtype (1);
  dnsq.SetQclass (1);
  p->AddHeader (dnsq);
  DNSHeader dnsh;
  p->AddHeader (dnsh);
  UdpHeader udph;
  udph.SetDestinationPort (53);
  p->AddHeader (udph);

  Ipv4Header ipHeader;
  // faked
  ipHeader.SetSource (Ipv4Address ("10.0.3.2"));
  ipHeader.SetDestination (dst);
  ipHeader.SetProtocol (17);
  ipHeader.SetPayloadSize (p->GetSize ());
  ipHeader.SetTtl (255);
  p->AddHeader (ipHeader);
  socket->Send (p, 0);

  //  socket->Close ();

  Simulator::ScheduleWithContext (node->GetId (), MicroSeconds (1000),
                                  &SendSpoofDNS, node, dst);
}

uint64_t tot53bytes = 0;
void
P2pRxCallback (std::string context, Ptr<const Packet> originalPacket)
{
  tot53bytes += originalPacket->GetSize ();
  //  NS_LOG_DEBUG ("received packet is " << originalPacket->GetSize () << " bytes");
}

int main (int argc, char *argv[])
{
  bool m_disableDnssec = false;
  uint32_t startTime = 10;
  uint32_t stopTime = 40;
  bool enablePcap = false;
  CommandLine cmd;
  cmd.AddValue ("delay" , "add process delay (default 1)", m_delay);
  cmd.AddValue ("pcap", "enable Pcap output (default no)", enablePcap);
  cmd.AddValue ("nNodes" , "num of nodes (default 20)", nNodes);
  cmd.AddValue ("disableDnssec", "disable DNSSEC (default enable)", m_disableDnssec);
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (nNodes);
  setPos (nodes.Get (0), 0, 50, 0);
  setPos (nodes.Get (1), 0, 100, 0);
  setPos (nodes.Get (2), 0, 150, 0);
  setPos (nodes.Get (3), 0, 200, 0);
  setPos (nodes.Get (4), -50, 200, 0);

/*
  Ptr<RateErrorModel> em = CreateObjectWithAttributes<RateErrorModel> (
                           "RanVar", RandomVariableValue (UniformVariable (0., 1.)),
                           "ErrorRate", DoubleValue (0.00001));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
*/

  //InternetStackHelper stack;
  LinuxStackHelper stack;
  stack.Install (nodes);
  DceManagerHelper processManager;
  if (m_delay)
    {
      processManager.SetDelayModel ("ns3::TimeOfDayProcessDelayModel");
    }
  processManager.SetNetworkStack("ns3::LinuxSocketFdFactory",
                                 "Library", StringValue ("liblinux.so"));
  processManager.Install (nodes);

  // Assign address
  NetDeviceContainer devices;

  PointToPointHelper p2p;
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer interfaces;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Gbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("100ms"));
  devices = p2p.Install (NodeContainer (nodes.Get (0), nodes.Get (1)));
  address.SetBase ("10.0.0.0", "255.255.255.0");
  interfaces = address.Assign (devices);
  devices = p2p.Install (NodeContainer (nodes.Get (1), nodes.Get (2)));
  address.SetBase ("10.0.1.0", "255.255.255.0");
  interfaces = address.Assign (devices);
  devices = p2p.Install (NodeContainer (nodes.Get (2), nodes.Get (3)));
  address.SetBase ("10.0.2.0", "255.255.255.0");
  interfaces = address.Assign (devices);

  for (int i = 4 ; i < nodes.GetN (); i++)
    {
      std::stringstream sstr;
      sstr << "10.0." <<  (i - 1) << ".0";
      address.SetBase (sstr.str ().c_str (), "255.255.255.0");
      devices = p2p.Install (NodeContainer (nodes.Get (3), nodes.Get (i)));
      interfaces = address.Assign (devices);

      // Schedule Fake DNS query
      if (i != 4)
        {
          Simulator::ScheduleWithContext (nodes.Get (i)->GetId (), Seconds (startTime + 0.1*i),
                                          &SendSpoofDNS, nodes.Get (i), 
                                          interfaces.GetAddress (0, 0));
        }
    }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  LinuxStackHelper::PopulateRoutingTables ();

  DceApplicationHelper process;
  ApplicationContainer apps;

  Bind9Helper bind9;
  //  bind9.EnableDebug (nodes);

  // use createzones
  bind9.UseManualConfig (nodes);
  bind9.AddZone (nodes.Get (0), ".");
  bind9.SetNsAddr (nodes.Get (0), "10.0.0.1");
  bind9.AddZone (nodes.Get (1), "jp");
  bind9.SetNsAddr (nodes.Get (1), "10.0.1.1");
  bind9.AddZone (nodes.Get (2), "example.jp");
  bind9.SetNsAddr (nodes.Get (2), "10.0.2.1");
  if (m_disableDnssec)
    {
      bind9.DisableDnssec (nodes.Get (1));
      bind9.DisableDnssec (nodes.Get (2));
    }
  bind9.Install (NodeContainer (nodes.Get (0), nodes.Get (1), nodes.Get (2)));

  // cache server
  UnboundHelper unbound;
  unbound.SetCacheServer (nodes.Get (3));
  //  unbound.EnableDebug (nodes.Get (3));
  // XXX: need to be implemented to UnboundHelper or createzones.rb
  ::system ("mkdir -p files-3/tmp/namedb/");
  ::system ("rm -f files-3/tmp/namedb/auto-trust-anchor");
  ::system ("cat files-2/tmp/namedb/*.key > files-3/tmp/namedb/auto-trust-anchor");
  if (m_disableDnssec)
    {
      unbound.DisableDnssec (nodes.Get (3));
    }
  unbound.Install (nodes.Get (3));

  Config::Connect ("/NodeList/4/DeviceList/*/$ns3::PointToPointNetDevice/MacRx",
		   MakeCallback (&P2pRxCallback));

  if (enablePcap)
    {
      p2p.EnablePcapAll ("dce-dns-reflection");
    }
  Simulator::Stop (Seconds (stopTime));
  Simulator::Run ();
  Simulator::Destroy ();

  std::cout << "Total message in network is " << tot53bytes << " bytes ( " 
            << tot53bytes*8/(stopTime - startTime) << " bps )" << std::endl;
  return 0;
}
