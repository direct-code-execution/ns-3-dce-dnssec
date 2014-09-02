#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/dce-dnssec-module.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/constant-position-mobility-model.h"
#include <fstream>
#include <sys/resource.h>
#include "ns3/breakpoint.h"

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
#if 0
	  if (udpHdr.GetDestinationPort () == 62724) 
	    {
      NS_BREAKPOINT ();
	    }
#endif
	}
      else if (packet->PeekHeader (tcpHdr) != 0)
	{
	  packet->RemoveHeader (tcpHdr);
	  if (tcpHdr.GetDestinationPort () == 53 ||
	      tcpHdr.GetSourcePort () == 53)
	    {
	      NS_LOG_INFO ("received dns packet (tcp) " << originalPacket->GetSize () << " bytes");
	      tot53pkts += originalPacket->GetSize ();
	    }
	}
    }

}

static void
SetRlimit ()
{
  int ret;
  struct rlimit limit;
  limit.rlim_cur = 10240;
  limit.rlim_max = 10240;

  ret = setrlimit (RLIMIT_NOFILE, &limit);
  if (ret == -1)
    {
      perror ("setrlimit");
    }
  return;
}

bool m_delay = true;
uint32_t nNodes = 1;
bool enablePcap = false;
std::string linkDelay = "1ms";
double lossRatio = 0.00;
uint32_t m_qps = 1;
bool m_disableDnssec = false;
bool useQlog = true;
double stopTime = 10000.0;
bool m_real = false;
std::string querylog_file = "qlog-10.log";

int main (int argc, char *argv[])
{
  SetRlimit ();

  CommandLine cmd;
  cmd.AddValue ("processDelay", "add process delay (default 1)", m_delay);
  cmd.AddValue ("nNodes", "the number of client nodes (default 1)", nNodes);
  cmd.AddValue ("pcap", "enable Pcap output (default no)", enablePcap);
  cmd.AddValue ("linkDelay", "configure each link delay (default 1ms)", linkDelay);
  cmd.AddValue ("lossRatio", "configure each link packet loss ratio (default 0%)", lossRatio);
  cmd.AddValue ("qps", "query per second (default 1qps)", m_qps);
  cmd.AddValue ("useQlog", "use bind9 query log as input of client queries (default none)", useQlog);
  cmd.AddValue ("querylog", "filename of querylog", querylog_file);
  cmd.AddValue ("disableDnssec", "disable DNSSEC (default enable)", m_disableDnssec);
  cmd.AddValue ("real", "realtime simulation mode (default disable)", m_real);
  cmd.Parse (argc, argv);

  if (m_real)
    {
      GlobalValue::Bind ("SimulatorImplementationType", 
                         StringValue ("ns3::RealtimeSimulatorImpl"));
    }
  GlobalValue::BindFailSafe ("SimulationTimeBase", 
                     UintegerValue (time (NULL)));

  // 
  ::system ("rm -f files-*/tmp/namedb/*.key");

  NodeContainer nodes, cacheSv, client;
  Bind9Helper bind9;
  nodes = bind9.ImportNodesCreateZones (querylog_file, m_disableDnssec);
  cacheSv.Create (1);
  client.Create (1);

  NetDeviceContainer devices;
  PointToPointHelper p2p;
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("5Gbps"));
  csma.SetChannelAttribute ("Delay", StringValue (linkDelay));
  devices = csma.Install (NodeContainer (nodes, cacheSv, client));

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
  csma.EnablePcap ("dce-dnssec", devices.Get (devices.GetN () - 2), true);
//  csma.EnablePcap ("dce-dnssec", devices.Get (devices.GetN () - 1), true);

  LinuxStackHelper stack;
  stack.Install (nodes);
  stack.Install (cacheSv);
  stack.Install (client);
  DceManagerHelper processManager;
  if (m_delay)
    {
      processManager.SetDelayModel ("ns3::TimeOfDayProcessDelayModel");
    }
  processManager.SetNetworkStack("ns3::LinuxSocketFdFactory",
				 "Library", StringValue ("liblinux.so"));
  processManager.Install (nodes);
  processManager.Install (cacheSv);
  processManager.Install (client);

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer interfaces;
  address.SetBase ("192.168.0.0", "255.255.0.0", "0.0.100.1");
  interfaces = address.Assign (devices);
  address.SetBase ("192.168.0.0", "255.255.0.0");
  address.Assign (devices);

  if (m_disableDnssec)
    {
      for (int i = 0; i < nodes.GetN (); i++)
        {
          bind9.DisableDnssec (nodes.Get (i));
        }
    }
  bind9.UseManualConfig (nodes);
  bind9.Install (nodes);

  UnboundHelper unbound;
  if (m_disableDnssec)
    {
      unbound.DisableDnssec (cacheSv.Get (0));
    }
  unbound.SetCacheServer (cacheSv.Get (0));
  unbound.EnableDebug (cacheSv);
  unbound.Install (cacheSv);

  // send buffer
  //stack.SysctlSet (cacheSv, ".net.core.wmem_max", "13107100");
  //stack.SysctlSet (cacheSv, ".net.core.wmem_default", "13107100");

  // XXX: need to be implemented to UnboundHelper or createzones.rb
  std::ostringstream oss;
  oss << cacheSv.Get (0)->GetId ();
  ::system ((std::string ("rm -f files-") + oss.str () + "/tmp/namedb/auto-trust-anchor").c_str ());
  ::system ((std::string ("cat files-0/tmp/namedb/*.key > files-") 
             + oss.str () +"/tmp/namedb/auto-trust-anchor").c_str ());

  DceApplicationHelper process;
  ApplicationContainer apps;

  // 
  // Client Configuration (node 5 - n)
  // 

  uint32_t numQuery = m_qps * (stopTime - 10);
  oss.str ("");
  oss << (cacheSv.Get (0)->GetId () + 1);
  if (useQlog)
    {
      std::map<std::string, std::list<Query> > query_map;
      query_map = bind9.ImportQueryLog (querylog_file);

      int j = 0;
      unbound.SetForwarder (client, interfaces.GetAddress (interfaces.GetN () - 2, 0));
      unbound.EnableDebug (client);
      unbound.Install (client);
      if (m_disableDnssec)
        {
          bind9.DisableDnssec (client.Get (0));
        }
#define LOOPS 10
      for (uint32_t k = 0 ; k < LOOPS; k++)
        {
      for (std::map<std::string, std::list<Query> >::iterator i = query_map.begin();
           i != query_map.end(); i++){
        std::string key = (*i).first;
        std::list<Query> val = (*i).second;

        std::list<Query>::iterator it = val.begin (); 
        while (it != val.end())
          {
            Query query = (*it);
            ++it;
            Time timestamp = query.m_tx_timestamp + (Seconds (50 * k));
//            std::cout << timestamp << " " << query.m_tx_timestamp << " "  << query.m_qname << query.m_class_name << query.m_type_name << std::endl;
            
            bind9.SendQuery (client.Get (0), timestamp,
                             query.m_qname, query.m_class_name, 
                             "A"); //query.m_type_name);
          }
        j++;
      }
      }
    }
  else
    {
      unbound.SetForwarder (client, interfaces.GetAddress (interfaces.GetN () - 2, 0));
      unbound.EnableDebug (client);
      unbound.Install (client);
      if (m_disableDnssec)
        {
          bind9.DisableDnssec (client.Get (0));
        }
      for (int j = 0; j < numQuery; j++)
        {
          bind9.SendQuery (client.Get (0), Seconds (10 + (1.0/m_qps)*j),
                           "net.", "IN", "SOA");
          //                "sg98.nc.u-tokyo.ac.jp.", "IN", "A");
          //                           "e3191.dscc.akamaiedge.net.", "IN", "A");

        }
    }

  Config::Connect ("/NodeList/*/DeviceList/0/$ns3::CsmaNetDevice/MacRx",
		   MakeCallback (&CsmaRxCallback));


  Simulator::Stop (Seconds (stopTime));
  Simulator::Run ();
  Simulator::Destroy ();

  std::cout << "Total message in network is " << tot53pkts << " bytes" << std::endl;

  return 0;
}
