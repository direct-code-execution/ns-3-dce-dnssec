#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/dce-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/dce-dnssec-module.h"
#include <fstream>

using namespace ns3;

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
  LinuxStackHelper stack;
  //processManager.SetLoader ("ns3::DlmLoaderFactory");
  processManager.SetLoader ("ns3::CoojaLoaderFactory");
  if (m_delay)
    {
      processManager.SetDelayModel ("ns3::TimeOfDayProcessDelayModel");
    }
  processManager.SetTaskManagerAttribute ("FiberManagerType",
                                          EnumValue (0));
  processManager.SetNetworkStack("ns3::LinuxSocketFdFactory",
				 "Library", StringValue ("libnet-next-2.6.so"));

  stack.Install (nodes);
  // Assign address
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  processManager.Install (nodes);

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
  Bind9Helper bind9;
  //bind9.UseManualConfig (nodes.Get (0));
  bind9.Install (nodes.Get (0));
#endif

  for (int i = 0; i < 20; i++)
    {
      process.SetBinary ("unbound-host");
      process.ResetArguments ();
      process.ParseArguments ("ns1.ns3-dns.wide.ad.jp");
      process.ParseArguments ("-d");
      process.ParseArguments ("-d");
      process.ParseArguments ("-v");
      process.ParseArguments ("-r");
      //      process.ParseArguments ("-C /etc/unbound.conf");
      process.ParseArguments ("-t");
      process.ParseArguments ("A");
//      process.ParseArguments ("-f");
//      process.ParseArguments ("/etc/root.key");
      process.SetStackSize (1<<16);
      apps = process.Install (nodes.Get (1));

      apps.Start (Seconds (1+ 10*i));
    }

  Simulator::Stop (Seconds (2000000.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
