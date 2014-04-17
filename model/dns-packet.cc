/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Hajime Tazaki
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
 * Author: Hajime Tazaki <tazaki@sfc.wide.ad.jp>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "dns-packet.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DNSHeader");
NS_OBJECT_ENSURE_REGISTERED (DNSHeader);

DNSHeader::DNSHeader ()
  : m_id (0xe0e5),
    m_flags (0x0110),
    m_qdcount (1),
    m_ancount (0),
    m_nscount (0),
    m_arcount (1)
{
}

DNSHeader::~DNSHeader ()
{
}

TypeId
DNSHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DNSHeader")
    .SetParent<Header> ()
    .AddConstructor<DNSHeader> ()
  ;
  return tid;
}
TypeId
DNSHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
DNSHeader::Print (std::ostream &os) const
{
  /// \todo
}

void
DNSHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtonU16 (m_id);
  i.WriteHtonU16 (m_flags);
  i.WriteHtonU16 (m_qdcount);
  i.WriteHtonU16 (m_ancount);
  i.WriteHtonU16 (m_nscount);
  i.WriteHtonU16 (m_arcount);
}

uint32_t
DNSHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_id  = i.ReadNtohU16 ();
  m_flags  = i.ReadNtohU16 ();
  m_qdcount  = i.ReadNtohU16 ();
  m_ancount  = i.ReadNtohU16 ();
  m_nscount  = i.ReadNtohU16 ();
  m_arcount  = i.ReadNtohU16 ();
  return GetSerializedSize ();
}

// Question section
QuestionSectionHeader::QuestionSectionHeader ()
{
}

QuestionSectionHeader::~QuestionSectionHeader ()
{
}

TypeId
QuestionSectionHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuestionSectionHeader")
    .SetParent<Header> ()
    .AddConstructor<QuestionSectionHeader> ()
  ;
  return tid;
}
TypeId
QuestionSectionHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void 
QuestionSectionHeader::Print (std::ostream &os) const
{
  /// \todo
}

uint32_t
QuestionSectionHeader::GetSerializedSize (void) const
{
  // FIXME: END0 added
  return m_qname.size () + 1 + sizeof (m_qtype) + sizeof (m_qclass) + 11;
}

void
QuestionSectionHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  std::string::size_type pos = 0, prev_pos = 0;
  while ((pos = m_qname.find_first_of ('.', prev_pos)) != std::string::npos)
    {
      i.WriteU8 (pos - prev_pos);
      i.Write ((const uint8_t *)m_qname.substr (prev_pos, pos - prev_pos).c_str (), pos - prev_pos);
      prev_pos = pos + 1;
    }
  i.WriteU8 (0x00);
  i.WriteHtonU16 (m_qtype);
  i.WriteHtonU16 (m_qclass);

  // FIXME: add EDNS0 option (length=11)
  i.WriteU8 (0x00);  i.WriteU8 (0x00);
  i.WriteU8 (0x29);  i.WriteU8 (0x10);
  i.WriteU8 (0x00);  i.WriteU8 (0x00);
  i.WriteU8 (0x00);  i.WriteU8 (0x80);
  i.WriteU8 (0x00);  i.WriteU8 (0x00);
  i.WriteU8 (0x00);


}

uint32_t
QuestionSectionHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t labelSize;
  m_qname = std::string ("");
  while ((labelSize = i.ReadU8 ()) != 0x00)
    {
      char label[labelSize];
      i.Read ((uint8_t *)label, labelSize);
      m_qname += label;
      m_qname += ".";
    }
  std::cout << m_qname << std::endl;
  // char *qname = malloc (
  // m_qname  = i.ReadNtohU16 ();
  m_qtype  = i.ReadNtohU16 ();
  m_qclass  = i.ReadNtohU16 ();
  return GetSerializedSize ();
}


}  // namespace ns3

