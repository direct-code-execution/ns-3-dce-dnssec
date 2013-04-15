#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/dce-dnssec-module.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/constant-position-mobility-model.h"
#include <fstream>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("DceDnssec");

void setPos (Ptr<Node> n, int x, int y, int z)
{
  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
}

static void RunIp (Ptr<Node> node, Time at, std::string str)
{
  DceApplicationHelper process;
  ApplicationContainer apps;
  process.SetBinary ("ip");
  process.SetStackSize (1<<16);
  process.ResetArguments();
  process.ParseArguments(str.c_str ());
  apps = process.Install (node);
  apps.Start (at);
}

static void AddAddress (Ptr<Node> node, Time at, const char *name, const std::string prefixAddr,
                        int number, std::string suffixAddr)
{
  std::ostringstream oss;
  oss << "-f inet addr add " << prefixAddr << number << suffixAddr << " dev " << name;
  RunIp (node, at, oss.str ());
}

uint64_t tot53pkts = 0;
void
CsmaRxCallback (std::string context, Ptr<const Packet> originalPacket)
{
  Ptr<Packet> packet = originalPacket->Copy ();
  EthernetHeader ethhdr;
  Ipv4Header v4hdr;
  TcpHeader tcpHdr;
  UdpHeader udpHdr;

  if (!packet->PeekHeader (ethhdr))
    {
      return;
    }

  packet->RemoveHeader (ethhdr);
  // if not ipv4
  if (ethhdr.GetLengthType () != 0x0800)
    {
      return;
    }

  if (packet->PeekHeader (v4hdr) != 0)
    {
      packet->RemoveHeader (v4hdr);
      if (packet->PeekHeader (udpHdr) != 0)
	{
	  packet->RemoveHeader (udpHdr);
	  if (udpHdr.GetDestinationPort () == 53 ||
	      udpHdr.GetSourcePort () == 53)
	    {
	      NS_LOG_INFO ("received dns packet " << originalPacket->GetSize () << " bytes");
	      tot53pkts += originalPacket->GetSize ();
	    }
	}
      else if (packet->PeekHeader (tcpHdr) != 0)
	{
	  packet->RemoveHeader (tcpHdr);
	  if (tcpHdr.GetDestinationPort () == 53 ||
	      tcpHdr.GetSourcePort () == 53)
	    {
	      tot53pkts += originalPacket->GetSize ();
	    }
	}
    }

}
bool m_delay = true;
uint32_t nNodes = 1;
bool enablePcap = false;
std::string linkDelay = "1ms";
double lossRatio = 0.00;
uint32_t m_qps = 1;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.AddValue ("processDelay", "add process delay (default 1)", m_delay);
  cmd.AddValue ("nNodes", "the number of client nodes (default 1)", nNodes);
  cmd.AddValue ("pcap", "enable Pcap output (default no)", enablePcap);
  cmd.AddValue ("linkDelay", "configure each link delay (default 1ms)", linkDelay);
  cmd.AddValue ("lossRatio", "configure each link packet loss ratio (default 0%)", lossRatio);
  cmd.AddValue ("qps", "query per second (default 1qps)", m_qps);
  cmd.Parse (argc, argv);

  NodeContainer trustAuth, subAuth, fakeRoot, cacheSv, client;
  NodeContainer nodes;
  fakeRoot.Create (1);
  trustAuth.Create (1);
  subAuth.Create (2);
  cacheSv.Create (1);
  client.Create (nNodes);
  nodes = NodeContainer (fakeRoot, trustAuth, subAuth, cacheSv, client);
  setPos (fakeRoot.Get (0), 0, 100, 0);
  setPos (trustAuth.Get (0), 0, 110, 0);
  setPos (subAuth.Get (0), -50, 120, 0);
  setPos (subAuth.Get (1), 50, 130, 0);
  setPos (cacheSv.Get (0), 0, 140, 0);
  for (uint32_t i = 0; i < nNodes; i++)
    {
      setPos (client.Get (i), -(50 * (nNodes-1)/2) + i*50, 200, 0);
    }

  NetDeviceContainer devices;

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("500Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue (linkDelay));
  devices = csma.Install (nodes);
  // bottle neck link
  Ptr<RateErrorModel> em;

  em = CreateObjectWithAttributes<RateErrorModel>
    ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"),
     "ErrorRate", DoubleValue (lossRatio),
     "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET)
     );
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  if (enablePcap)
    {
      csma.EnablePcapAll ("dce-dnssec");
    }

  DceManagerHelper processManager;
  //processManager.SetLoader ("ns3::DlmLoaderFactory");
  if (m_delay)
    {
      processManager.SetDelayModel ("ns3::TimeOfDayProcessDelayModel");
    }
  processManager.SetTaskManagerAttribute ("FiberManagerType",
                                          EnumValue (0));
  processManager.SetNetworkStack("ns3::LinuxSocketFdFactory",
				 "Library", StringValue ("libnet-next-2.6.so"));
  processManager.Install (nodes);

  for (int n=0; n < nodes.GetN (); n++)
    {
      AddAddress (nodes.Get (n), Seconds (0.1), "sim0", "10.0.0.", 1 + n, "/8");
      AddAddress (nodes.Get (n), Seconds (0.1), "sim0", "192.168.255.", 51 + n, "/24");
      RunIp (nodes.Get (n), Seconds (0.2), "link set sim0 up");
      // RunIp (nodes.Get (n), Seconds (0.2), "link show");
      // RunIp (nodes.Get (n), Seconds (0.3), "route show table all");
      // RunIp (nodes.Get (n), Seconds (0.4), "addr list");
    }

  DceApplicationHelper process;
  ApplicationContainer apps;
  Bind9Helper bind9;
  UnboundHelper unbound;
  // 
  // FakeRoot Configuration (node 0)
  // 
  bind9.UseManualConfig (fakeRoot);
  bind9.SetNsAddr (fakeRoot.Get (0), "10.0.0.1");
  bind9.AddZone (fakeRoot.Get (0), ".");

  // 
  // Tursted Anthor Authority Server Configuration (node 1)
  // 
  bind9.UseManualConfig (trustAuth);
  bind9.SetNsAddr (trustAuth.Get (0), "10.0.0.2");
  bind9.AddZone (trustAuth.Get (0), "org");

  // 
  // Authority Server of Sub-Domain Configuration (node 2)
  // 
  bind9.UseManualConfig (subAuth.Get (0));
  bind9.SetNsAddr (subAuth.Get (0), "10.0.0.3");
  bind9.AddZone (subAuth.Get (0), "example.org");

  // 
  // Authority Server of Sub-Domain Configuration (node 3)
  // 
  bind9.UseManualConfig (subAuth.Get (1));
  bind9.SetNsAddr (subAuth.Get (1), "10.0.0.4");
  bind9.AddZone (subAuth.Get (1), "second.example.org");

  // 
  // Cache Server Configuration (node 4)
  // 
  //  bind9.UseManualConfig (cacheSv);
  bind9.SetCacheServer (cacheSv.Get (0));

  bind9.Install (NodeContainer (fakeRoot, trustAuth, subAuth, cacheSv));

  // 
  // Client Configuration (node 5 - n)
  // 
  // unbound.SendQuery (cacheSv.Get (0), Seconds (10), 
  // 		     "mail.example.org");

  uint32_t numQuery = m_qps * 50;
  for (int i = 0; i < nNodes; i++)
    {
      // node3 is forwarder
      unbound.SetForwarder (client.Get (i), "10.0.0.5");
      for (int j = 0; j < numQuery; j++)
	{
	  unbound.SendQuery (client.Get (i), Seconds (10 + (1.0/m_qps)*j),
	   		     "mail.example.org.");
	}
      unbound.Install (client.Get (i));

    }

  Config::Connect ("/NodeList/*/DeviceList/0/$ns3::CsmaNetDevice/MacRx",
		   MakeCallback (&CsmaRxCallback));

  Simulator::Stop (Seconds (200.0));
  Simulator::Run ();
  Simulator::Destroy ();

  std::cout << "Total message in network is " << tot53pkts << " bytes" << std::endl;

  return 0;
}
