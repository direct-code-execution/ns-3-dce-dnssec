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

void SendSpoofDNS (Ptr<Node> node, Ipv4Address dst)
{
  // for (int i = 0; i < max_pkts; i++)
  //   {
      Ptr<Socket> socket;
      socket = Socket::CreateSocket (node, LinuxIpv4RawSocketFactory::GetTypeId ());

      Ptr<LinuxSocketImpl> linux_sock = StaticCast<LinuxSocketImpl> (socket);
      int hincl = 1;
      linux_sock->Setsockopt (IPPROTO_IP, IP_HDRINCL, &hincl, sizeof (hincl));

      //  socket->Bind ();
      socket->Connect (InetSocketAddress (dst));

      //  socket->SetAttribute ("IpHeaderInclude", BooleanValue (true));
      Ptr<Packet> p = Create<Packet> (1400);
      UdpHeader udph;
      udph.SetDestinationPort (53);
      p->AddHeader (udph);
      Ipv4Header ipHeader;
      // faked
      ipHeader.SetSource (Ipv4Address ("10.0.0.2"));
      ipHeader.SetDestination (dst);
      ipHeader.SetProtocol (17);
      ipHeader.SetPayloadSize (p->GetSize ());
      ipHeader.SetTtl (255);
      p->AddHeader (ipHeader);
      socket->Send (p, 0);

      //  socket->Close ();
      Simulator::ScheduleWithContext (node->GetId (), MilliSeconds (10),
                                      &SendSpoofDNS, node, dst);
      //    }
}

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.AddValue ("delay" , "add process delay (default 1)", m_delay);
  cmd.AddValue ("nNodes" , "num of nodes (default 20)", nNodes);
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (nNodes);
  setPos (nodes.Get (1), 0, 100, 0);

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
  for (int i = 1 ; i < nodes.GetN (); i++)
    {
      std::stringstream sstr;
      sstr << "10.0." <<  (i - 1) << ".0";
      address.SetBase (sstr.str ().c_str (), "255.255.255.0");
      devices = p2p.Install (NodeContainer (nodes.Get (0), nodes.Get (i)));
      interfaces = address.Assign (devices);

      // Schedule Fake DNS query
      if (i != 1)
        {
          Simulator::ScheduleWithContext (nodes.Get (i)->GetId (), Seconds (20 + 0.3*i),
                                          &SendSpoofDNS, nodes.Get (i), 
                                          interfaces.GetAddress (0, 0));
        }
    }

  DceApplicationHelper process;
  ApplicationContainer apps;

  Bind9Helper bind9;
  //bind9.UseManualConfig (nodes.Get (0));
  bind9.AddZone (nodes.Get (0), "ns3-dns.wide.ad.jp.");
  bind9.Install (nodes.Get (0));

  UnboundHelper unbound;
#if 0
  // node0 is forwarder
  unbound.SetForwarder (nodes.Get (1), "10.0.0.1");
  unbound.Install (nodes.Get (1));
  for (int i = 0; i < 2; i++)
    {
      unbound.SendQuery (nodes.Get (1), Seconds (1+ 10*i), 
			 "ns1.ns3-dns.wide.ad.jp", "IN", "A");
    }

#endif

  p2p.EnablePcapAll ("dce-dns-reflection");
  Simulator::Stop (Seconds (400.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
