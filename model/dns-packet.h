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

#ifndef DNS_PACKET_H
#define DNS_PACKET_H

#include "ns3/header.h"
#include "ns3/nstime.h"


namespace ns3 {

// DNS Header (RFC 1035)
// 
// 4.1.1. Header section format
//                                 1  1  1  1  1  1
//   0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                      ID                       |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                    QDCOUNT                    |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                    ANCOUNT                    |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                    NSCOUNT                    |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |                    ARCOUNT                    |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// OPCODE          A four bit field that specifies kind of query in this
//                 message.  This value is set by the originator of a query
//                 and copied into the response.  The values are:
//                 0               a standard query (QUERY)
//                 1               an inverse query (IQUERY)
//                 2               a server status request (STATUS)
//                 3-15            reserved for future use
// RCODE           Response code - this 4 bit field is set as part of
//                 responses.  The values have the following
//                 interpretation:
//                 0               No error condition
//                 1               Format error - The name server was
//                                 unable to interpret the query.
//                 2               Server failure - The name server was
//                                 unable to process this query due to a
//                                 problem with the name server.
//                 3               Name Error - Meaningful only for
//                                 responses from an authoritative name
//                                 server, this code signifies that the
//                                 domain name referenced in the query does
//                                 not exist.
//                 4               Not Implemented - The name server does
//                                 not support the requested kind of query.
//                 5               Refused - The name server refuses to
//                                 perform the specified operation for
//                                 policy reasons.  For example, a name
//                                 server may not wish to provide the
//                                 information to the particular requester,
//                                 or a name server may not wish to perform
//                                 a particular operation (e.g., zone


#define DNS_HEADER_SIZE 12

class DNSHeader : public Header
{
public:
  DNSHeader ();
  virtual ~DNSHeader ();

  virtual uint32_t GetSerializedSize (void) const
  {
    return DNS_HEADER_SIZE;
  }

  void SetId (uint16_t id)
  {
    m_id = id;
  }
  uint16_t GetId () const
  {
    return m_id;
  }

  // flags
  void SetQRbit (bool f)
  {
    if (f)
      m_flags |= (1 << 15);
    else
      m_flags &= ~(1 << 15);
  }
  bool GetQRbit () const
  {
    return (m_flags & (1 << 15));
  }

  void SetOpCode (uint8_t opcode)
  {
    m_flags |= (opcode << 11);
  }
  uint8_t GetOpCode () const
  {
    return (m_flags >> 11) & 0x000F;
  }

  void SetAAbit (bool f)
  {
    if (f)
      m_flags |= (1 << 10);
    else
      m_flags &= ~(1 << 10);
  }
  bool GetAAbit () const
  {
    return (m_flags & (1 << 10));
  }

  void SetTruncated (bool f)
  {
    if (f)
      m_flags |= (1 << 9);
    else
      m_flags &= ~(1 << 9);
  }
  bool GetTruncated () const
  {
    return (m_flags & (1 << 9));
  }
  void SetRDbit (bool f)
  {
    if (f)
      m_flags |= (1 << 8);
    else
      m_flags &= ~(1 << 8);
  }
  bool GetRDbit () const
  {
    return (m_flags & (1 << 8));
  }

  void SetRAbit (bool f)
  {
    if (f)
      m_flags |= (1 << 7);
    else
      m_flags &= ~(1 << 7);
  }
  bool GetRAbit () const
  {
    return (m_flags & (1 << 7));
  }

  void SetRcode (uint8_t rcode)
  {
    m_flags |= (rcode << 0);
  }
  uint8_t GetRcode () const
  {
    return (m_flags >> 0) & 0x000F;
  }

  void SetQdCount (uint16_t qdcount)
  {
    m_qdcount = qdcount;
  }
  uint16_t GetQdCount () const
  {
    return m_qdcount;
  }

  void SetAnCount (uint16_t ancount)
  {
    m_ancount = ancount;
  }
  uint16_t GetAnCount () const
  {
    return m_ancount;
  }

  void SetNsCount (uint16_t nscount)
  {
    m_nscount = nscount;
  }
  uint16_t GetNsCount () const
  {
    return m_nscount;
  }

  void SetArCount (uint16_t arcount)
  {
    m_arcount = arcount;
  }
  uint16_t GetArCount () const
  {
    return m_arcount;
  }


private:
  uint16_t m_id;
  uint16_t m_flags;
  uint16_t m_qdcount;
  uint16_t m_ancount;
  uint16_t m_nscount;
  uint16_t m_arcount;

public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
};

static inline std::ostream& operator<< (std::ostream& os, const DNSHeader & packet)
{
  packet.Print (os);
  return os;
}

// 4.1.2. Question section format
//                                     1  1  1  1  1  1
//       0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//     |                                               |
//     /                     QNAME                     /
//     /                                               /
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//     |                     QTYPE                     |
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//     |                     QCLASS                    |
//     +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

class QuestionSectionHeader : public Header
{
public:
  QuestionSectionHeader ();
  virtual ~QuestionSectionHeader ();

  void SetQname (std::string qname)
  {
    m_qname = qname;
  }
  std::string GetQname () const
  {
    return m_qname;
  }

  void SetQtype (uint16_t qtype)
  {
    m_qtype = qtype;
  }
  bool GetQtype () const
  {
    return m_qtype;
  }

  void SetQclass (uint16_t qclass)
  {
    m_qclass = qclass;
  }
  bool GetQclass () const
  {
    return m_qclass;
  }

private:
  std::string m_qname;
  uint16_t m_qtype;
  uint16_t m_qclass;

public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Print (std::ostream &os) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
};


#if 0
//        0                   1                   2                   3
//        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |  Message Type |     Vtime     |         Message Size          |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |                      Originator Address                       |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//       |  Time To Live |   Hop Count   |    Message Sequence Number    |
//       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class MessageHeader : public Header
{
public:

  enum MessageType {
    HELLO_MESSAGE = 1,
    TC_MESSAGE    = 2,
    MID_MESSAGE   = 3,
    HNA_MESSAGE   = 4,
  };

  MessageHeader ();
  virtual ~MessageHeader ();

  void SetMessageType (MessageType messageType)
  {
    m_messageType = messageType;
  }
  MessageType GetMessageType () const
  {
    return m_messageType;
  }

  void SetVTime (Time time)
  {
    m_vTime = SecondsToEmf (time.GetSeconds ());
  }
  Time GetVTime () const
  {
    return Seconds (EmfToSeconds (m_vTime));
  }

  void SetOriginatorAddress (Ipv4Address originatorAddress)
  {
    m_originatorAddress = originatorAddress;
  }
  Ipv4Address GetOriginatorAddress () const
  {
    return m_originatorAddress;
  }

  void SetTimeToLive (uint8_t timeToLive)
  {
    m_timeToLive = timeToLive;
  }
  uint8_t GetTimeToLive () const
  {
    return m_timeToLive;
  }

  void SetHopCount (uint8_t hopCount)
  {
    m_hopCount = hopCount;
  }
  uint8_t GetHopCount () const
  {
    return m_hopCount;
  }

  void SetMessageSequenceNumber (uint16_t messageSequenceNumber)
  {
    m_messageSequenceNumber = messageSequenceNumber;
  }
  uint16_t GetMessageSequenceNumber () const
  {
    return m_messageSequenceNumber;
  }

//   void SetMessageSize (uint16_t messageSize)
//   {
//     m_messageSize = messageSize;
//   }
//   uint16_t GetMessageSize () const
//   {
//     return m_messageSize;
//   }

private:
  MessageType m_messageType;
  uint8_t m_vTime;
  Ipv4Address m_originatorAddress;
  uint8_t m_timeToLive;
  uint8_t m_hopCount;
  uint16_t m_messageSequenceNumber;
  uint16_t m_messageSize;

public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  // 5.1.  MID Message Format
  //
  //    The proposed format of a MID message is as follows:
  //
  //        0                   1                   2                   3
  //        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                    OLSR Interface Address                     |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                    OLSR Interface Address                     |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                              ...                              |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  struct Mid
  {
    std::vector<Ipv4Address> interfaceAddresses;
    void Print (std::ostream &os) const;
    uint32_t GetSerializedSize (void) const;
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start, uint32_t messageSize);
  };

  // 6.1.  HELLO Message Format
  //
  //        0                   1                   2                   3
  //        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  //
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |          Reserved             |     Htime     |  Willingness  |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |   Link Code   |   Reserved    |       Link Message Size       |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                  Neighbor Interface Address                   |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                  Neighbor Interface Address                   |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       :                             .  .  .                           :
  //       :                                                               :
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |   Link Code   |   Reserved    |       Link Message Size       |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                  Neighbor Interface Address                   |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                  Neighbor Interface Address                   |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       :                                                               :
  //       :                                       :
  //    (etc.)
  struct Hello
  {
    struct LinkMessage {
      uint8_t linkCode;
      std::vector<Ipv4Address> neighborInterfaceAddresses;
    };

    uint8_t hTime;
    void SetHTime (Time time)
    {
      this->hTime = SecondsToEmf (time.GetSeconds ());
    }
    Time GetHTime () const
    {
      return Seconds (EmfToSeconds (this->hTime));
    }

    uint8_t willingness;
    std::vector<LinkMessage> linkMessages;

    void Print (std::ostream &os) const;
    uint32_t GetSerializedSize (void) const;
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start, uint32_t messageSize);
  };

  // 9.1.  TC Message Format
  //
  //    The proposed format of a TC message is as follows:
  //
  //        0                   1                   2                   3
  //        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |              ANSN             |           Reserved            |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |               Advertised Neighbor Main Address                |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |               Advertised Neighbor Main Address                |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                              ...                              |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  struct Tc
  {
    std::vector<Ipv4Address> neighborAddresses;
    uint16_t ansn;

    void Print (std::ostream &os) const;
    uint32_t GetSerializedSize (void) const;
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start, uint32_t messageSize);
  };


  // 12.1.  HNA Message Format
  //
  //    The proposed format of an HNA-message is:
  //
  //        0                   1                   2                   3
  //        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                         Network Address                       |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                             Netmask                           |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                         Network Address                       |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                             Netmask                           |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  //       |                              ...                              |
  //       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  // Note: HNA stands for Host Network Association
  struct Hna
  {
    struct Association
    {
      Ipv4Address address;
      Ipv4Mask mask;
    };
    std::vector<Association> associations;

    void Print (std::ostream &os) const;
    uint32_t GetSerializedSize (void) const;
    void Serialize (Buffer::Iterator start) const;
    uint32_t Deserialize (Buffer::Iterator start, uint32_t messageSize);
  };

private:
  struct
  {
    Mid mid;
    Hello hello;
    Tc tc;
    Hna hna;
  } m_message; // union not allowed

public:

  Mid& GetMid ()
  {
    if (m_messageType == 0)
      {
        m_messageType = MID_MESSAGE;
      }
    else
      {
        NS_ASSERT (m_messageType == MID_MESSAGE);
      }
    return m_message.mid;
  }

  Hello& GetHello ()
  {
    if (m_messageType == 0)
      {
        m_messageType = HELLO_MESSAGE;
      }
    else
      {
        NS_ASSERT (m_messageType == HELLO_MESSAGE);
      }
    return m_message.hello;
  }

  Tc& GetTc ()
  {
    if (m_messageType == 0)
      {
        m_messageType = TC_MESSAGE;
      }
    else
      {
        NS_ASSERT (m_messageType == TC_MESSAGE);
      }
    return m_message.tc;
  }

  Hna& GetHna ()
  {
    if (m_messageType == 0)
      {
        m_messageType = HNA_MESSAGE;
      }
    else
      {
        NS_ASSERT (m_messageType == HNA_MESSAGE);
      }
    return m_message.hna;
  }


  const Mid& GetMid () const
  {
    NS_ASSERT (m_messageType == MID_MESSAGE);
    return m_message.mid;
  }

  const Hello& GetHello () const
  {
    NS_ASSERT (m_messageType == HELLO_MESSAGE);
    return m_message.hello;
  }

  const Tc& GetTc () const
  {
    NS_ASSERT (m_messageType == TC_MESSAGE);
    return m_message.tc;
  }

  const Hna& GetHna () const
  {
    NS_ASSERT (m_messageType == HNA_MESSAGE);
    return m_message.hna;
  }


};


static inline std::ostream& operator<< (std::ostream& os, const MessageHeader & message)
{
  message.Print (os);
  return os;
}

typedef std::vector<MessageHeader> MessageList;

static inline std::ostream& operator<< (std::ostream& os, const MessageList & messages)
{
  os << "[";
  for (std::vector<MessageHeader>::const_iterator messageIter = messages.begin ();
       messageIter != messages.end (); messageIter++)
    {
      messageIter->Print (os);
      if (messageIter+1 != messages.end ())
        os << ", ";
    }
  os << "]";
  return os;
}
#endif

}  // namespace ns3

#endif /* DNS_PACKET_H */

