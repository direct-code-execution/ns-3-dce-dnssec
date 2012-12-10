#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/dce-dnssec-module.h"
#include <fstream>

using namespace ns3;

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

bool m_delay = true;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.AddValue ("delay" , "add process delay (default 1)", m_delay);
  cmd.Parse (argc, argv);

  NodeContainer nodes;
  nodes.Create (2);

  NetDeviceContainer devices;

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Gbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1ms"));
  devices = p2p.Install (nodes);
/*
  Ptr<RateErrorModel> em = CreateObjectWithAttributes<RateErrorModel> (
                           "RanVar", RandomVariableValue (UniformVariable (0., 1.)),
                           "ErrorRate", DoubleValue (0.00001));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
*/
  p2p.EnablePcapAll ("process-unbound");

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

  for (int n=0; n < 2; n++)
    {
      AddAddress (nodes.Get (n), Seconds (0.1), "sim0", "10.0.0.", 2 + n, "/8" );
      RunIp (nodes.Get (n), Seconds (0.11), "link set sim0 up arp off");
      RunIp (nodes.Get (n), Seconds (0.2), "link show");
      RunIp (nodes.Get (n), Seconds (0.3), "route show table all");
      RunIp (nodes.Get (n), Seconds (0.4), "addr list");
    }
  RunIp (nodes.Get (1), Seconds (1.2), "route add default via 10.0.0.2 dev sim0");

  DceApplicationHelper process;
  ApplicationContainer apps;

#if 0
  process.SetBinary ("unbound");
  process.ResetArguments ();
  process.ParseArguments ("-d");
  process.SetStackSize (1<<16);
  apps = process.Install (nodes.Get (0));
  apps.Start (Seconds (1.0));
#else
#if 0
  process.SetBinary ("named");
  process.ResetArguments ();
  process.ParseArguments ("-f");
  process.ParseArguments ("-d");
  process.ParseArguments ("8");
  process.ParseArguments ("-4");
  process.ParseArguments ("-u");
  process.ParseArguments ("root");
  process.ParseArguments ("-c");
  process.ParseArguments ("/etc/namedb/named.conf");
  process.SetStackSize (1<<16);
  apps = process.Install (nodes.Get (0));
  apps.Start (Seconds (1.0));
#else
  Bind9Helper bind9;
  //bind9.UseManualConfig (nodes.Get (0));
  bind9.Install (nodes.Get (0));
#endif
#endif

  for (int i = 0; i < 20; i++)
    {
#if 1
      process.SetBinary ("unbound-host");
      process.ResetArguments ();
      process.ParseArguments ("ns1.ns3-dns.wide.ad.jp");
      process.ParseArguments ("-d");
      process.ParseArguments ("-d");
      process.ParseArguments ("-v");
      process.ParseArguments ("-r");
      process.ParseArguments ("-t");
      process.ParseArguments ("A");
//      process.ParseArguments ("-f");
//      process.ParseArguments ("/etc/root.key");
#else
      process.SetBinary ("nslookup");
      process.ResetArguments ();
      process.ParseArguments ("ns1.ns3-dns.wide.ad.jp");
#endif
      process.SetStackSize (1<<16);
      apps = process.Install (nodes.Get (1));

      apps.Start (Seconds (1+ 10*i));
    }

  Simulator::Stop (Seconds (2000000.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
