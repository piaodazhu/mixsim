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

#ifndef NDN_APP_MIXSIM_H
#define NDN_APP_MIXSIM_H
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ndn-app.hpp"
#include "ns3/random-variable-stream.h"
#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/traced-value.h"
#include "ndn-cxx/lp/nack.hpp"

namespace ns3 {
namespace ndn {

struct simple_name_data {
	unsigned int nlen;
	unsigned int dlen;
	char buf[2040];
};

typedef struct simple_name_data simple_name_data;

/**
 * @ingroup ndn-apps
 * \brief Ndn application for sending out Interest packets in virtual
 */
class AppMixSim : public App {
public:
  static TypeId
  GetTypeId();

  /**
   * \brief Default constructor
   */
  AppMixSim();

	virtual void
  OnInterest(shared_ptr<const Interest> interest);

  virtual void
  OnData(shared_ptr<const Data> data);

	virtual void
  OnNack(shared_ptr<const lp::Nack> nack);

  void
  SendNDNInterest(simple_name_data *interest);

	void
  SendNDNData(simple_name_data *data);

  void
  SendNDNNack(simple_name_data *nack);

  void
  SendUDPInterest(shared_ptr<const Interest> interest);

	void
  SendUDPData(shared_ptr<const Data> data);

  void
  SendUDPNack(shared_ptr<const lp::Nack> nack);

private:
  virtual void
  StartApplication(); ///< @brief Called at time specified by Start

  virtual void
  StopApplication(); ///< @brief Called at time specified by Start
  
  void 
  setnonblocking(int sockfd);

  void 
  registerFib();

  int64_t
  AnalyzeTimeDiff();

protected:
	void
  ScheduleNextTick();

private:
	Ptr<UniformRandomVariable> m_rand; ///< @brief nonce generator
  
  std::string m_prefixes;

	int m_interest_lport;
	int m_data_lport;
  int m_interest_udpfd;
  int m_data_udpfd;

	int m_interest_rport;
	int m_data_rport;
	std::string m_raip;
	struct sockaddr_in m_interest_raddr;
	struct sockaddr_in m_data_raddr;

  struct timeval m_start;
  uint64_t m_totalticks, m_case1ticks, m_case2ticks;

  uint32_t m_signature;
  Name m_keyLocator;

	EventId m_tickEvent; 
};

} // namespace ndn
} // namespace ns3

#endif
