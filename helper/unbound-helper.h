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
#ifndef UNBOUND_HELPER_H
#define UNBOUND_HELPER_H

#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/object-factory.h"
#include "ns3/boolean.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/address-utils.h"

namespace ns3 {

/**
 * \brief create a umip (unbound) daemon as an application and associate it to a node
 *
 */
class UnboundHelper
{
public:
  /**
   * Create a UnboundHelper which is used to make life easier for people wanting
   * to use unbound Applications.
   *
   */
  UnboundHelper ();

  /**
   * Install a unbound application on each Node in the provided NodeContainer.
   *
   * \param nodes The NodeContainer containing all of the nodes to get a unbound
   *              application via ProcessManager.
   *
   * \returns A list of unbound applications, one for each input node
   */
  ApplicationContainer Install (NodeContainer nodes);

  /**
   * Install a unbound application on the provided Node.  The Node is specified
   * directly by a Ptr<Node>
   *
   * \param node The node to install the Application on.
   *
   * \returns An ApplicationContainer holding the unbound application created.
   */
  ApplicationContainer Install (Ptr<Node> node);

  /**
   * Install a unbound application on the provided Node.  The Node is specified
   * by a string that must have previosly been associated with a Node using the
   * Object Name Service.
   *
   * \param nodeName The node to install the ProcessApplication on.
   *
   * \returns An ApplicationContainer holding the unbound application created.
   */
  ApplicationContainer Install (std::string nodeName);

  /**
   * \brief Configure ping applications attribute
   * \param name   attribute's name
   * \param value  attribute's value
   */
  void SetAttribute (std::string name, const AttributeValue &value);


  // Common
  void EnableDebug (NodeContainer nodes);
  void UseManualConfig (NodeContainer nodes);
  void SetBinary (NodeContainer nodes, std::string binary);
  void SetForwarder (NodeContainer nodes, Ipv4Address fwd);
  void SendQuery (Ptr<Node> node, Time at, std::string query);

private:
  /**
   * \internal
   */
  ApplicationContainer InstallPriv (Ptr<Node> node);
  void GenerateConfig (Ptr<Node> node);
};

} // namespace ns3

#endif /* UNBOUND_HELPER_H */
