/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 NICT
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Hajime Tazaki <tazaki@nict.go.jp>
 */

#include "ns3/object-factory.h"
#include "unbound-helper.h"
#include "ns3/dce-application-helper.h"
#include "ns3/names.h"
#include <fstream>
#include <map>
#include <sys/stat.h>

namespace ns3 {

class UnboundConfig : public Object
{
private:
  static int index;
public:
  UnboundConfig ()
    : m_debug (false),
      m_usemanualconf (false),
      m_binary ("unbound-host"),
      m_forwarder ("8.8.8.8 (need to configure:XXX)")
  {
  }
  ~UnboundConfig ()
  {
  }

  static TypeId
  GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::UnboundConfig")
      .SetParent<Object> ()
      .AddConstructor<UnboundConfig> ()
    ;
    return tid;
  }
  TypeId
  GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  bool m_debug;
  bool m_usemanualconf;
  std::string m_binary;
  Ipv4Address m_forwarder;

  virtual void
  Print (std::ostream& os) const
  {
    // os << "# IPsec configuration - NO IPSEC AT THE MOMENT" << std::endl
    //    << "UseMnHaIPsec disabled;" << std::endl
    //    << "KeyMngMobCapability disabled;" << std::endl
    //    << "# EOF" << std::endl;
  }
};
std::ostream& operator << (std::ostream& os, UnboundConfig const& config)
{
  config.Print (os);
  return os;
}


UnboundHelper::UnboundHelper ()
{
}

void
UnboundHelper::SetAttribute (std::string name, const AttributeValue &value)
{
}


void
UnboundHelper::SetForwarder (NodeContainer nodes, Ipv4Address fwd)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<UnboundConfig> unbound_conf = nodes.Get (i)->GetObject<UnboundConfig> ();
      if (!unbound_conf)
        {
          unbound_conf = CreateObject<UnboundConfig> ();
          nodes.Get (i)->AggregateObject (unbound_conf);
        }
      unbound_conf->m_forwarder = fwd;
    }
  return;
}

void
UnboundHelper::EnableDebug (NodeContainer nodes)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<UnboundConfig> unbound_conf = nodes.Get (i)->GetObject<UnboundConfig> ();
      if (!unbound_conf)
        {
          unbound_conf = CreateObject<UnboundConfig> ();
          nodes.Get (i)->AggregateObject (unbound_conf);
        }
      unbound_conf->m_debug = true;
    }
  return;
}

void
UnboundHelper::UseManualConfig (NodeContainer nodes)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<UnboundConfig> unbound_conf = nodes.Get (i)->GetObject<UnboundConfig> ();
      if (!unbound_conf)
        {
          unbound_conf = new UnboundConfig ();
          nodes.Get (i)->AggregateObject (unbound_conf);
        }
      unbound_conf->m_usemanualconf = true;
    }
  return;
}

void
UnboundHelper::SetBinary (NodeContainer nodes, std::string binary)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<UnboundConfig> unbound_conf = nodes.Get (i)->GetObject<UnboundConfig> ();
      if (!unbound_conf)
        {
          unbound_conf = new UnboundConfig ();
          nodes.Get (i)->AggregateObject (unbound_conf);
        }
      unbound_conf->m_binary = binary;
    }
  return;
}

void
UnboundHelper::GenerateConfig (Ptr<Node> node)
{
  Ptr<UnboundConfig> unbound_conf = node->GetObject<UnboundConfig> ();

  if (unbound_conf->m_usemanualconf)
    {
      return;
    }

  // config generation
  std::stringstream conf_dir, conf_file;
  // FIXME XXX
  conf_dir << "files-" << node->GetId () << "";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/etc/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);

  conf_file << conf_dir.str () << "/unbound.conf";
  std::ofstream conf;
  conf.open (conf_file.str ().c_str ());
  conf << *unbound_conf;
  conf.close ();

  conf_file.str ("");
  conf_file << "files-" << node->GetId () << "/etc/resolv.conf";
  conf.open (conf_file.str ().c_str ());
  conf << "nameserver " << unbound_conf->m_forwarder << std::endl;
  conf.close ();
}

void
UnboundHelper::SendQuery (Ptr<Node> node, Time at, std::string qname, 
                          std::string class_name, std::string type_name)
{
  DceApplicationHelper process;
  ApplicationContainer apps;
  Ptr<UnboundConfig> unbound_conf = node->GetObject<UnboundConfig> ();
  if (!unbound_conf)
    {
      unbound_conf = new UnboundConfig ();
      node->AggregateObject (unbound_conf);
    }

  process.SetBinary ("unbound-host");
  process.ResetArguments ();
  process.ParseArguments (qname);
  process.ParseArguments ("-r");
  //      process.ParseArguments ("-C /etc/unbound.conf");
  process.ParseArguments ("-c");
  process.ParseArguments (class_name);
  process.ParseArguments ("-t");
  process.ParseArguments (type_name);
  //      process.ParseArguments ("-f");
  //      process.ParseArguments ("/etc/root.key");
  if (unbound_conf->m_debug)
    {
      process.ParseArguments ("-d -d -v");
    }
  process.SetStackSize (1<<16);
  apps = process.Install (node);
  apps.Start (at);
  return;
}

ApplicationContainer
UnboundHelper::Install (Ptr<Node> node)
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UnboundHelper::Install (std::string nodeName)
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UnboundHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

ApplicationContainer
UnboundHelper::InstallPriv (Ptr<Node> node)
{
  DceApplicationHelper process;
  ApplicationContainer apps;

  Ptr<UnboundConfig> unbound_conf = node->GetObject<UnboundConfig> ();
  if (!unbound_conf)
    {
      unbound_conf = new UnboundConfig ();
      node->AggregateObject (unbound_conf);
    }
  GenerateConfig (node);

  process.ResetArguments ();
  process.SetBinary (unbound_conf->m_binary);
  if (unbound_conf->m_debug)
    {
      process.ParseArguments ("-d -d -v");
    }
  process.ParseArguments ("-r");
  process.SetStackSize (1 << 16);
  apps.Add (process.Install (node));
  apps.Get (0)->SetStartTime (Seconds (1.0 + 0.01 * node->GetId ()));
  node->AddApplication (apps.Get (0));
  return apps;
}

} // namespace ns3
