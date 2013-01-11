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
#include "bind9-helper.h"
#include "ns3/dce-application-helper.h"
#include "ns3/names.h"
#include <fstream>
#include <map>
#include <sys/stat.h>

namespace ns3 {

class Bind9Config : public Object
{
private:
  static int index;
public:
  Bind9Config ()
    : m_debug (false),
      m_usemanualconf (false),
      m_binary ("named")
  {
    m_zones = new std::vector<std::string> ();
  }
  ~Bind9Config ()
  {
  }

  static TypeId
  GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::Bind9Config")
      .SetParent<Object> ()
      .AddConstructor<Bind9Config> ()
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
  std::vector<std::string> *m_zones;

  virtual void
  Print (std::ostream& os) const
  {
    // os << "# IPsec configuration - NO IPSEC AT THE MOMENT" << std::endl
    //    << "UseMnHaIPsec disabled;" << std::endl
    //    << "KeyMngMobCapability disabled;" << std::endl
    //    << "# EOF" << std::endl;
  }
};
std::ostream& operator << (std::ostream& os, Bind9Config const& config)
{
  config.Print (os);
  return os;
}


Bind9Helper::Bind9Helper ()
{
}

void
Bind9Helper::SetAttribute (std::string name, const AttributeValue &value)
{
}


void
Bind9Helper::AddZone (Ptr<Node> node, std::string zone_name)
{
  Ptr<Bind9Config> bind9_conf = node->GetObject<Bind9Config> ();
  if (!bind9_conf)
    {
      bind9_conf = CreateObject<Bind9Config> ();
      node->AggregateObject (bind9_conf);
    }

  bind9_conf->m_zones->push_back (zone_name);
  return;
}

void
Bind9Helper::EnableDebug (NodeContainer nodes)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<Bind9Config> bind9_conf = nodes.Get (i)->GetObject<Bind9Config> ();
      if (!bind9_conf)
        {
          bind9_conf = CreateObject<Bind9Config> ();
          nodes.Get (i)->AggregateObject (bind9_conf);
        }
      bind9_conf->m_debug = true;
    }
  return;
}

void
Bind9Helper::UseManualConfig (NodeContainer nodes)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<Bind9Config> bind9_conf = nodes.Get (i)->GetObject<Bind9Config> ();
      if (!bind9_conf)
        {
          bind9_conf = new Bind9Config ();
          nodes.Get (i)->AggregateObject (bind9_conf);
        }
      bind9_conf->m_usemanualconf = true;
    }
  return;
}

void
Bind9Helper::SetBinary (NodeContainer nodes, std::string binary)
{
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<Bind9Config> bind9_conf = nodes.Get (i)->GetObject<Bind9Config> ();
      if (!bind9_conf)
        {
          bind9_conf = new Bind9Config ();
          nodes.Get (i)->AggregateObject (bind9_conf);
        }
      bind9_conf->m_binary = binary;
    }
  return;
}

void
Bind9Helper::GenerateConfig (Ptr<Node> node)
{
  Ptr<Bind9Config> bind9_conf = node->GetObject<Bind9Config> ();

  if (bind9_conf->m_usemanualconf)
    {
      return;
    }

  // config generation
  std::stringstream conf_dir, conf_file, varrun_dir, zone_file;
  // FIXME XXX
  conf_dir << "files-" << node->GetId () << "";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/etc/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/namedb/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);

  varrun_dir << "files-" << node->GetId () << "";
  varrun_dir << "/var/";
  ::mkdir (varrun_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  varrun_dir << "/run";
  ::mkdir (varrun_dir.str ().c_str (), S_IRWXU | S_IRWXG);

  // generate /etc/namedb/named.conf
  conf_file << conf_dir.str () << "/named.conf";
  std::ofstream conf, zonef;
  conf.open (conf_file.str ().c_str ());

  conf << "options {"  << std::endl;
  conf << "  directory \"/etc/namedb\";"  << std::endl;
  conf << "  pid-file \"/var/run/named.pid\";"  << std::endl;
  conf << "  listen-on { any; };"  << std::endl;
  conf << "  listen-on-v6 { none; };"  << std::endl;
  conf << "};"  << std::endl;

  // zone information
  for (std::vector<std::string>::iterator i = bind9_conf->m_zones->begin ();
       i != bind9_conf->m_zones->end (); ++i)
    {
      if (i != bind9_conf->m_zones->begin ())
        {
          conf << "," ;
        }
      conf << "zone \"" <<  (*i) << "\" {" << std::endl;
      conf << "      type master;" << std::endl;
      conf << "      file \"" << (*i) << "zone\";" << std::endl;
      conf << "};" << std::endl;

      // /etc/namedb/{zonefile}
      zone_file << conf_dir.str () << "/" << (*i) << "zone";
      zonef.open (zone_file.str ().c_str ());
      zonef << "$ORIGIN ." << std::endl;
      zonef << "$TTL 100" << std::endl;
      zonef << (*i) << "\t IN SOA " << (*i) << " sho." << (*i) << " (" << std::endl;
      zonef << "                                1  ; serial" << std::endl;
      zonef << "                                1800       ; refresh (30 minutes)" << std::endl;
      zonef << "                                900        ; retry (15 minutes)" << std::endl;
      zonef << "                                604800     ; expire (1 week)" << std::endl;
      zonef << "                                10800      ; minimum (3 hours)" << std::endl;
      zonef << "                                )" << std::endl;
      zonef << "  NS      ns1." <<  (*i) << std::endl << std::endl ;
      zonef << "$ORIGIN " << (*i) << std::endl;
      zonef << "ns1                     IN A    10.0.0.2" << std::endl;
      zonef.close ();
    }

  conf << *bind9_conf;
  conf.close ();

  // fake /etc/passwd
  conf_file.str ("");
  conf_file << "files-" << node->GetId () << "/etc/passwd";
  conf.open (conf_file.str ().c_str ());
  conf << "deadcode;deadcode;deadcode;deadcode;deadcode;deadcode;" << std::endl;
  conf.close ();
}


ApplicationContainer
Bind9Helper::Install (Ptr<Node> node)
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
Bind9Helper::Install (std::string nodeName)
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
Bind9Helper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

ApplicationContainer
Bind9Helper::InstallPriv (Ptr<Node> node)
{
  DceApplicationHelper process;
  ApplicationContainer apps;

  Ptr<Bind9Config> bind9_conf = node->GetObject<Bind9Config> ();
  if (!bind9_conf)
    {
      bind9_conf = new Bind9Config ();
      node->AggregateObject (bind9_conf);
    }
  GenerateConfig (node);

  process.ResetArguments ();
  process.SetBinary (bind9_conf->m_binary);
  process.ParseArguments ("-f -d 8 -4 -u root");
  process.ParseArguments ("-c /tmp/namedb/named.conf");
  process.SetStackSize (1 << 16);
  apps.Add (process.Install (node));
  apps.Get (0)->SetStartTime (Seconds (1.0 + 0.01 * node->GetId ()));
  node->AddApplication (apps.Get (0));

  return apps;
}

} // namespace ns3
