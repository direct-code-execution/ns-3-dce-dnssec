#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-dnssec-module.h"
#include <fstream>

using namespace ns3;

bool m_delay = true;

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
  p2p.EnablePcapAll ("dce-dns-simple");

  DceManagerHelper processManager;
  processManager.SetLoader ("ns3::CoojaLoaderFactory");
  //  processManager.SetLoader ("ns3::DlmLoaderFactory");
  if (m_delay)
    {
      // processManager.SetDelayModel ("ns3::TimeOfDayProcessDelayModel");
    }
  processManager.SetTaskManagerAttribute ("FiberManagerType",
                                          EnumValue (0));
  processManager.SetNetworkStack("ns3::LinuxSocketFdFactory",
				 "Library", StringValue ("liblinux.so"));

  // Assign address
  for (int n=0; n < nodes.GetN (); n++)
    {
      AddAddress (nodes.Get (n), Seconds (0.1), "sim0", "10.0.0.", 1 + n, "/24");
      RunIp (nodes.Get (n), Seconds (0.2), "link set sim0 up");
    }

  processManager.Install (nodes);

  DceApplicationHelper process;
  ApplicationContainer apps;

  Bind9Helper bind9;
  //bind9.UseManualConfig (nodes.Get (0));
  bind9.AddZone (nodes.Get (0), "ns3-dns.wide.ad.jp.");
  bind9.Install (nodes.Get (0));

  UnboundHelper unbound;
  // node0 is forwarder
  unbound.SetForwarder (nodes.Get (1), "10.0.0.1");
  unbound.Install (nodes.Get (1));
  for (int i = 0; i < 20; i++)
    {
      unbound.SendQuery (nodes.Get (1), Seconds (1+ 10*i), 
			 "ns1.ns3-dns.wide.ad.jp");
    }

  Simulator::Stop (Seconds (400.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
