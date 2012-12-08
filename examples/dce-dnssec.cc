#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
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
bool nNodes = 1;

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.AddValue ("delay", "add process delay (default 1)", m_delay);
  cmd.AddValue ("nNodes", "the number of client nodes (default 2)", nNodes);
  cmd.Parse (argc, argv);

  NodeContainer trustAuth, subAuth, fakeRoot, cacheSv, client;
  NodeContainer nodes;
  fakeRoot.Create (1);
  trustAuth.Create (1);
  subAuth.Create (1);
  cacheSv.Create (1);
  client.Create (nNodes);
  nodes = NodeContainer (trustAuth, subAuth, fakeRoot, cacheSv, client);

  NetDeviceContainer devices;

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("500Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("1ms"));
  devices = csma.Install (nodes);
/*
  Ptr<RateErrorModel> em = CreateObjectWithAttributes<RateErrorModel> (
                           "RanVar", RandomVariableValue (UniformVariable (0., 1.)),
                           "ErrorRate", DoubleValue (0.00001));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
*/
  csma.EnablePcapAll ("process-dnssec");

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
      AddAddress (nodes.Get (n), Seconds (0.1), "sim0", "10.0.0.", 2 + n, "/8" );
      // RunIp (nodes.Get (n), Seconds (0.2), "link show");
      // RunIp (nodes.Get (n), Seconds (0.3), "route show table all");
      // RunIp (nodes.Get (n), Seconds (0.4), "addr list");
    }

  DceApplicationHelper process;
  ApplicationContainer apps;
  // 
  // FakeRoot Configuration (node 0)
  // 
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
  apps = process.Install (fakeRoot);
  apps.Start (Seconds (1.0));

  // 
  // Tursted Anthor Authority Server Configuration (node 1)
  // 
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
  apps = process.Install (trustAuth);
  apps.Start (Seconds (1.0));

  // 
  // Authority Server of Sub-Domain Configuration (node 2)
  // 
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
  apps = process.Install (subAuth);
  apps.Start (Seconds (1.0));

  // 
  // Cache Server Configuration (node 3)
  // 
  process.SetBinary ("unbound");
  process.ResetArguments ();
  process.ParseArguments ("-d");
  process.ParseArguments ("-c");
  process.ParseArguments ("/etc/unbound.conf");
  process.SetStackSize (1<<16);
  apps = process.Install (cacheSv);
  apps.Start (Seconds (1.0));


  // 
  // Client Configuration (node 4 - n)
  // 
  for (int i = 0; i < nNodes; i++)
    {
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
      apps = process.Install (client.Get (i));
      apps.Start (Seconds (1+ 10*i));
    }


  Simulator::Stop (Seconds (2000000.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
