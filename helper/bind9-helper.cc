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
#include "ns3/log.h"
#include "bind9-helper.h"
#include "ns3/dce-application-helper.h"
#include "ns3/names.h"
#include <fstream>
#include <map>
#include <sys/stat.h>
#include <ruby.h>

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("Bind9Helper");

class Bind9Config : public Object
{
private:
  static int index;
public:
  Bind9Config ()
    : m_debug (false),
      m_usemanualconf (false),
      m_binary ("named"),
      m_iscache (false),
      m_isdnssec (true),
      m_querylog (false)
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
  std::string m_nsaddr;
  bool m_iscache;
  bool m_isdnssec;
  bool m_querylog;

  virtual void
  Print (std::ostream& os) const
  {
    // os << "# IPsec configuration - NO IPSEC AT THE MOMENT" << std::endl
    //    << "UseMnHaIPsec disabled;" << std::endl
    //    << "KeyMngMobCapability disabled;" << std::endl
    //    << "# EOF" << std::endl;
  }
};

Query::Query (Time tx_timestamp, std::string qname, std::string class_name, std::string type_name, std::string recur_flag)
{
  m_tx_timestamp = tx_timestamp;
  m_qname = qname;
  m_class_name = class_name;
  m_type_name = type_name;
  m_recur_flag = recur_flag;
}


std::ostream& operator << (std::ostream& os, Bind9Config const& config)
{
  config.Print (os);
  return os;
}


Bind9Helper::Bind9Helper ()
{
  ruby_init();
  ruby_init_loadpath();
  // ruby_cleanup(0);
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
Bind9Helper::SetNsAddr (Ptr<Node> node, std::string nsaddr)
{
  Ptr<Bind9Config> bind9_conf = node->GetObject<Bind9Config> ();
  if (!bind9_conf)
    {
      bind9_conf = CreateObject<Bind9Config> ();
      node->AggregateObject (bind9_conf);
    }

  bind9_conf->m_nsaddr = nsaddr;
  return;
}

void
Bind9Helper::SetCacheServer (Ptr<Node> node)
{
  Ptr<Bind9Config> bind9_conf = node->GetObject<Bind9Config> ();
  if (!bind9_conf)
    {
      bind9_conf = CreateObject<Bind9Config> ();
      node->AggregateObject (bind9_conf);
    }

  bind9_conf->m_iscache = true;
  return;
}

void
Bind9Helper::DisableDnssec (Ptr<Node> node)
{
  Ptr<Bind9Config> bind9_conf = node->GetObject<Bind9Config> ();
  if (!bind9_conf)
    {
      bind9_conf = CreateObject<Bind9Config> ();
      node->AggregateObject (bind9_conf);
    }

  bind9_conf->m_isdnssec = false;
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
  conf_dir << "/tmp/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_dir << "/namedb/";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);

  varrun_dir << "files-" << node->GetId () << "";
  varrun_dir << "/var/";
  ::mkdir (varrun_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  varrun_dir << "/run";
  ::mkdir (varrun_dir.str ().c_str (), S_IRWXU | S_IRWXG);

  // generate /tmp/namedb/named.conf
  conf_file << conf_dir.str () << "/named.conf";
  std::ofstream conf, zonef;
  conf.open (conf_file.str ().c_str ());

  conf << "options {"  << std::endl;
  conf << "  directory \"/tmp/namedb\";"  << std::endl;
  conf << "  pid-file \"/var/run/named.pid\";"  << std::endl;
  conf << "  listen-on { any; };"  << std::endl;
  conf << "  listen-on-v6 { none; };"  << std::endl;
  if (bind9_conf->m_iscache)
    {
      conf << "  recursion yes;"  << std::endl;
    }
  conf << "};"  << std::endl;

  // FIXME: add EnableQueryLog()
  if (bind9_conf->m_querylog)
    {
      conf << "logging {"  << std::endl;
      conf << "        channel queries-log {"  << std::endl;
      conf << "                file \"/var/log/queries.log\" versions 3 size 10m;"  << std::endl;
      conf << "                severity info;"  << std::endl;
      conf << "                print-time yes;"  << std::endl;
      conf << "                print-severity yes;"  << std::endl;
      conf << "                print-category yes;"  << std::endl;
      conf << "        };"  << std::endl;
      conf << "        category queries { queries-log; };"  << std::endl;
      conf << "};"  << std::endl;
    }

  if (bind9_conf->m_iscache)
    {
      conf << "zone \".\" {"  << std::endl;
      conf << "        type hint;"  << std::endl;
      conf << "        file \"named.root\";"  << std::endl;
      conf << "};"  << std::endl;
    }

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

      // /tmp/namedb/{zonefile}
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

  // named.root
  if (bind9_conf->m_iscache)
    {
      conf_file.str ("");
      conf_file << conf_dir.str () << "/named.root";
      std::ofstream rootf;
      rootf.open (conf_file.str ().c_str ());
      rootf << ".                        3600000  IN  NS    ns." << std::endl;
      rootf << "ns.                      3600000      A  10.0.0.1" << std::endl;
      rootf.close ();
    }

  // fake /etc/passwd
  conf_file.str ("");
  conf_dir.str ("");
  conf_dir << "files-" << node->GetId () << "/etc";
  ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
  conf_file << "files-" << node->GetId () << "/etc/passwd";
  conf.open (conf_file.str ().c_str ());
  conf << "deadcode;deadcode;deadcode;deadcode;deadcode;deadcode;" << std::endl;
  conf.close ();
}

void
Bind9Helper::CreateZones (NodeContainer c)
{
  Ptr<Node> node;
  std::ofstream conf;
  conf.open ("./nsconfig.txt");

  std::stringstream conf_dir;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      node = (*i);
      Ptr<Bind9Config> bind9_conf = node->GetObject<Bind9Config> ();
      // FIXME XXX
      conf_dir.str ("");
      conf_dir << "files-" << node->GetId () << "";
      ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
      conf_dir << "/tmp/";
      ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);
      conf_dir << "/namedb/";
      ::mkdir (conf_dir.str ().c_str (), S_IRWXU | S_IRWXG);

      if (!bind9_conf->m_zones->empty ())
        {
          conf << bind9_conf->m_nsaddr << " " << bind9_conf->m_nsaddr << " "
               << *(bind9_conf->m_zones->begin ())
               << " files-" << node->GetId (); 

          // dnssec or not
          if (bind9_conf->m_isdnssec)
            conf << " yes";
          else
            conf << " no";
          conf << std::endl;
        }
    }
  conf.close ();

  // call createzone.rb
  int state;
  rb_load_protect (rb_str_new2("createzones/createzones.rb"), 0, &state);
  //  rb_load_protect (rb_str_new2("createzones/createzones.rb build/nsconfig.txt -o ./"), 0, &state);
  if (state)
    {
      VALUE rbError = rb_funcall(rb_gv_get("$!"), rb_intern("message"), 0);
      std::cerr << StringValuePtr(rbError) << std::endl;
  }

}

#if 1

std::list <Query>
Bind9Helper::ImportQueryLog (std::string logfile)
{
  std::ifstream topgen;
  topgen.open (logfile.c_str ());
  std::list<Query> query_list;

  if (!topgen.is_open ())
    {
      NS_LOG_WARN ("Bind9 querylog file object is not open, check file name and permissions");
      return query_list;
    }

  /*
   * Data format (bind9 querylog)
   *
   * 19-Jun-2013 14:46:38.370 queries: client 133.11.103.26#57936: query: eowp.alc.co.jp IN A + (133.11.124.164)
   * {Date} {"queries:"} {"client"} {srcip#port":"} {"query:"} {name} {classname} {typename} {recursive flag} {resolver?}
   *
   */
  std::string dummy;
  std::string date;
  std::string timestamp;
  std::string qname;
  std::string class_name;
  std::string type_name;
  std::string recur_flag;
  int queryNum = 0;
  Time startTime = Seconds (0);

  std::istringstream lineBuffer;
  std::string line;

  for (int i = 0; !topgen.eof (); i++)
    {
      getline (topgen, line);
      queryNum++;
      lineBuffer.clear ();
      lineBuffer.str (line);

      // {Date} {Time} {"queries:"} {"client"} {srcip#port":"} {"query:"} {name} {classname} {typename} {recursive flag} {resolver?}
      lineBuffer >> date;
      lineBuffer >> timestamp;
      lineBuffer >> dummy;
      lineBuffer >> dummy;
      lineBuffer >> dummy;
      lineBuffer >> dummy;
      lineBuffer >> qname;
      lineBuffer >> class_name;
      lineBuffer >> type_name;
      lineBuffer >> recur_flag;
      lineBuffer >> dummy;

      if ((!qname.empty ()) && (!type_name.empty ()))
        {
          NS_LOG_INFO ("Query " << qname << " class: " << class_name << " type: " <<
                       type_name << " flag: " << recur_flag);

          struct tm tm;
          char *ret = strptime (timestamp.c_str (), "%X", &tm);
          if (startTime == Seconds (0))
            {
              startTime = Seconds (tm.tm_sec);
              startTime += MilliSeconds (atoi (ret + 1));
            }

          Query query (Seconds (10 + tm.tm_sec) + MilliSeconds (atoi (ret + 1)) - startTime,
                       qname, class_name, type_name, recur_flag);
          NS_LOG_INFO ("tx timestamp " << query.m_tx_timestamp);
          query_list.push_back (query);
        }
    }

  NS_LOG_INFO ("Bind9 query log created with " << queryNum << " queries ");
  topgen.close ();

  return query_list;
}
#endif

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
  CreateZones (c);
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
