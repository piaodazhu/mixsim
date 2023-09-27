/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "ndn-provider-virtual.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"

#include <memory>

NS_LOG_COMPONENT_DEFINE("ndn.ProducerVirtual");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(ProducerVirtual);

TypeId
ProducerVirtual::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::ProducerVirtual")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<ProducerVirtual>()
      .AddAttribute("Prefix", "Prefix, for which ProducerVirtual has the data", StringValue("/"),
                    MakeNameAccessor(&ProducerVirtual::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding ProducerVirtual-uniqueness)",
         StringValue("/"), MakeNameAccessor(&ProducerVirtual::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&ProducerVirtual::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&ProducerVirtual::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&ProducerVirtual::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&ProducerVirtual::m_keyLocator), MakeNameChecker());
  return tid;
}

ProducerVirtual::ProducerVirtual()
: m_rand(CreateObject<UniformRandomVariable>())
{
  NS_LOG_FUNCTION_NOARGS();
}

// inherited from Application base class.
void
ProducerVirtual::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
}

void
ProducerVirtual::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
ProducerVirtual::OnInterest(shared_ptr<const Interest> interest)
{
  printf("get interest\n");
  App::OnInterest(interest); // tracing inside

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;
  
  m_interestName = Name(interest->getName());
  FetchTime(interest->getName().at(-1).toSequenceNumber());

//   Name dataName(interest->getName());
//   // dataName.append(m_postfix);
//   // dataName.appendVersion();

//   auto data = make_shared<Data>();
//   data->setName(dataName);
//   data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

//   const char *prefix = "I am a provider inside ndnSIM!";
//   data->setContent(make_shared< ::ndn::Buffer>(prefix, strlen(prefix)));

//   SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

//   if (m_keyLocator.size() > 0) {
//     signatureInfo.setKeyLocator(m_keyLocator);
//   }

//   data->setSignatureInfo(signatureInfo);

//   ::ndn::EncodingEstimator estimator;
//   ::ndn::EncodingBuffer encoder(estimator.appendVarNumber(m_signature), 0);
//   encoder.appendVarNumber(m_signature);
//   data->setSignatureValue(encoder.getBuffer());

//   NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());

//   // to create real wire encoding
//   data->wireEncode();

//   m_transmittedDatas(data, this, m_face);
//   m_appLink->onReceiveData(*data);
}

void 
ProducerVirtual::FetchTime(uint32_t seq) {
  NS_LOG_FUNCTION_NOARGS();
  
  printf("fetch time\n");
  shared_ptr<Name> nameWithSequence = make_shared<Name>("/mixsim/real/time");
  nameWithSequence->appendSequenceNumber(seq);
  //

  // shared_ptr<Interest> interest = make_shared<Interest> ();
  shared_ptr<Interest> interest = make_shared<Interest>();
  interest->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setName(*nameWithSequence);
  interest->setCanBePrefix(false);
  time::milliseconds interestLifeTime(1000);
  interest->setInterestLifetime(interestLifeTime);

  // NS_LOG_INFO ("Requesting Interest: \n" << *interest);
  NS_LOG_INFO("> Interest for " << seq);

  m_transmittedInterests(interest, this, m_face);
  m_appLink->onReceiveInterest(*interest);
}

void
ProducerVirtual::OnData(shared_ptr<const Data> data) {
  printf("get data\n");
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  
  char buf[2048];
  int len = snprintf(buf, 2048, "(%u)[%s]:[%.*s]", seq, "I am a provider inside ndnSIM!", (int)data->getContent().size(), data->getContent().data());

  Name name(m_interestName); // ?
  auto response = make_shared<Data>();
  response->setName(name);
  response->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  response->setContent(make_shared< ::ndn::Buffer>(buf, len));

  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  response->setSignatureInfo(signatureInfo);

  ::ndn::EncodingEstimator estimator;
  ::ndn::EncodingBuffer encoder(estimator.appendVarNumber(m_signature), 0);
  encoder.appendVarNumber(m_signature);
  response->setSignatureValue(encoder.getBuffer());

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << response->getName());
  printf("send response\n");

  // to create real wire encoding
  response->wireEncode();

  m_transmittedDatas(response, this, m_face);
  m_appLink->onReceiveData(*response);
}

} // namespace ndn
} // namespace ns3
