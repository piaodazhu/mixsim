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

#include "ndn-app-mixsim.hpp"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "helper/ndn-fib-helper.hpp"

NS_LOG_COMPONENT_DEFINE("ndn.AppMixSim");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(AppMixSim);

TypeId
AppMixSim::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::AppMixSim")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<AppMixSim>()
			.AddAttribute("Prefixes", "All Allowed Forwarding Prefixes. Split with $",
                    StringValue("/"),
                    MakeStringAccessor(&AppMixSim::m_prefixes), MakeStringChecker())
			.AddAttribute("ILPort", "Interest UDP Local Listening Port",
                    IntegerValue(8787),
                    MakeIntegerAccessor(&AppMixSim::m_interest_lport), MakeIntegerChecker<int>())
			.AddAttribute("IRPort", "Interest UDP Remote Receiving Port",
                    IntegerValue(8798),
                    MakeIntegerAccessor(&AppMixSim::m_interest_rport), MakeIntegerChecker<int>())
		  .AddAttribute("DLPort", "Data UDP Local Listening Port",
                    IntegerValue(8788),
                    MakeIntegerAccessor(&AppMixSim::m_data_lport), MakeIntegerChecker<int>())
			.AddAttribute("DRPort", "Data UDP Remote Receiving Port",
                    IntegerValue(8797),
                    MakeIntegerAccessor(&AppMixSim::m_data_rport), MakeIntegerChecker<int>())
			.AddAttribute("RAIP", "Remote Adaptor IP",
                    StringValue("127.0.0.1"),
                    MakeStringAccessor(&AppMixSim::m_raip), MakeStringChecker())
			.AddAttribute(
										"Signature",
										"Fake signature, 0 valid signature (default), other values application-specific",
										UintegerValue(0), MakeUintegerAccessor(&AppMixSim::m_signature),
										MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&AppMixSim::m_keyLocator), MakeNameChecker());
  return tid;
}

AppMixSim::AppMixSim()
  : m_rand(CreateObject<UniformRandomVariable>())
	, m_totalticks(0)
	, m_case1ticks(0)
	, m_case2ticks(0)
{
	NS_LOG_FUNCTION_NOARGS();
	m_interest_udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_interest_udpfd < 0) {
		perror("socket local");
		exit(1);
	}
	setnonblocking(m_interest_udpfd);

	m_data_udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_data_udpfd < 0) {
		perror("socket remote");
		exit(1);
	}
	setnonblocking(m_data_udpfd);
}

void 
AppMixSim::setnonblocking(int sockfd) {
    int flag = fcntl(sockfd, F_GETFL, 0);
    if (flag < 0) {
        perror("fcntl F_GETFL fail");
        return;
    }
    if (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) < 0) {
        perror("fcntl F_SETFL fail");
    }
}

void
AppMixSim::StartApplication()
{
	memset(&m_interest_raddr, 0, sizeof(struct sockaddr_in)); 
	m_interest_raddr.sin_family = AF_INET; 
	m_interest_raddr.sin_port = htons(uint16_t(m_interest_rport));
	m_interest_raddr.sin_addr.s_addr = inet_addr(m_raip.c_str()); 

	memset(&m_data_raddr, 0, sizeof(struct sockaddr_in)); 
	m_data_raddr.sin_family = AF_INET; 
	m_data_raddr.sin_port = htons(uint16_t(m_data_rport));
	m_data_raddr.sin_addr.s_addr = inet_addr(m_raip.c_str()); 

	struct sockaddr_in iaddr; 
	memset(&iaddr, 0, sizeof(struct sockaddr_in)); 
	iaddr.sin_family = AF_INET; 
	iaddr.sin_port = htons(uint16_t(m_interest_lport));
	iaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	if(bind(m_interest_udpfd, (struct sockaddr *)&iaddr, sizeof(iaddr)) < 0) {
		perror("m_interest_udpfd bind error:"); 
		exit(1);
	}

	struct sockaddr_in daddr; 
	memset(&daddr, 0, sizeof(struct sockaddr_in)); 
	daddr.sin_family = AF_INET; 
	daddr.sin_port = htons(uint16_t(m_data_lport));
	daddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	if(bind(m_data_udpfd, (struct sockaddr *)&daddr, sizeof(daddr)) < 0) {
		perror("m_data_udpfd bind error:"); 
		exit(1);
	}
  printf("udp listening interest on %s:%d...\n", "0.0.0.0", m_interest_lport);
	printf("udp listening data on %s:%d...\n", "0.0.0.0", m_data_lport);

	gettimeofday(&m_start, NULL);

	App::StartApplication();

  registerFib();
	
	ScheduleNextTick();
}

void
AppMixSim::registerFib() {
	int len = m_prefixes.size();
	if (len == 0) {
		return;
	}
	const char *buf = m_prefixes.c_str();
	int last = 0;
	for (int idx = 0; idx < len; idx++) {
		if (buf[idx] == '$') {
			// std::cout << std::string(buf+last, idx-last) << std::endl;
			FibHelper::AddRoute(GetNode(), std::string(buf+last, idx-last), m_face, 0);
		  last = idx+1;
		}
	}
	// std::cout << std::string(buf+last, len-last) << std::endl;
	FibHelper::AddRoute(GetNode(), std::string(buf+last, len-last), m_face, 0);
}

void
AppMixSim::StopApplication() {
	printf("total ticks=%lu, case1=%lu, case2=%lu\n", m_totalticks, m_case1ticks, m_case2ticks);

	App::StopApplication();
}

void
AppMixSim::ScheduleNextTick()
{
	m_totalticks++;
	char recv_buf[2048];  

	int size = recv(m_interest_udpfd, recv_buf, sizeof(recv_buf), 0);  
	if (size > 0) {
		// printf("udp local recvfrom %s:%d...\n",  inet_ntoa(m_clientaddr.sin_addr), ntohs(m_port));
		SendNDNInterest((simple_name_data*)recv_buf);
	}
	
	size = recv(m_data_udpfd, recv_buf, sizeof(recv_buf), 0);
	if (size > 0) {
		// printf("udp remote recv. [%s:%d]\n", inet_ntoa(addr.sin_addr), addr.sin_port);
		SendNDNData((simple_name_data*)recv_buf);
	}

	int64_t timeDiff = AnalyzeTimeDiff();

	if (timeDiff <= 0) {
		m_case1ticks++;
		m_tickEvent = Simulator::Schedule(MicroSeconds(0), &AppMixSim::ScheduleNextTick, this);
	} else {
		// printf("tag2, pt=%ld,vt=%ld\n", ptime, vtime);
		m_case2ticks++;

		// uint64_t target = uint64_t(timeDiff - m_lastdiff);
		// m_lastdiff = target;
		m_tickEvent = Simulator::Schedule(MicroSeconds(uint64_t(timeDiff)), &AppMixSim::ScheduleNextTick, this);
	}
}

int64_t
AppMixSim::AnalyzeTimeDiff() {
	int64_t vtime = Simulator::Now().GetMicroSeconds();
	struct timeval curr;
	gettimeofday(&curr, NULL);
	int64_t ptime = int64_t(curr.tv_sec - m_start.tv_sec) * 1000000 + int64_t(curr.tv_usec - m_start.tv_usec);
	return ptime - vtime;
}

void
AppMixSim::SendNDNInterest(simple_name_data *interest) {
	NS_LOG_FUNCTION_NOARGS();
	// 1 seperate name and seqnum
	char namebuf[2048];
	char seqbuf[10];
	int idx;
	for (idx = interest->nlen - 1; interest->buf[idx] != '/'; idx--);
	memcpy(namebuf, interest->buf, idx);
	namebuf[idx] = '\0';
	idx++;
	memcpy(seqbuf, interest->buf + idx, interest->nlen - idx);
	seqbuf[interest->nlen - idx] = '\0';
	
	uint32_t iseq = uint32_t(atoi(seqbuf));
	Name iname(namebuf);

	std::cout << "ForwardNDNInterest " << iname << "#" << iseq << std::endl;
	
	// 2 make interest packet
	iname.appendSequenceNumber(iseq);
  shared_ptr<Interest> ipkt = make_shared<Interest>();
  ipkt->setNonce(m_rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  ipkt->setName(iname);
  ipkt->setCanBePrefix(false);
  ipkt->setInterestLifetime(::ndn::time::milliseconds(2000));

  NS_LOG_INFO("> Interest for " << iseq);
  
	// 3 send interest packet
  auto t = Simulator::Now();
	printf("ON SEND: %lu\n", t.GetMilliSeconds());

  m_transmittedInterests(ipkt, this, m_face);
  m_appLink->onReceiveInterest(*ipkt);
}

void
AppMixSim::SendNDNData(simple_name_data *data) {
	NS_LOG_FUNCTION_NOARGS();
	// 1 seperate name and seqnum
	char namebuf[2048];
	char seqbuf[10];
	int idx;
	for (idx = data->nlen - 1; data->buf[idx] != '/'; idx--);
	memcpy(namebuf, data->buf, idx);
	namebuf[idx] = '\0';
	idx++;
	memcpy(seqbuf, data->buf + idx, data->nlen - idx);
	seqbuf[data->nlen - idx] = '\0';
	
	uint32_t dseq = uint32_t(atoi(seqbuf));
	Name dname(namebuf);
	std::cout << "ForwardNDNData " << dname << "#" << dseq << std::endl;
	
	// 2 make data packet
	dname.appendSequenceNumber(dseq);
	auto dpkt = make_shared<Data>();
	dpkt->setName(dname);
	dpkt->setFreshnessPeriod(::ndn::time::milliseconds(1000));
	dpkt->setContent(make_shared< ::ndn::Buffer>(data->buf + data->nlen, data->dlen));
	SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));
	dpkt->setSignatureInfo(signatureInfo);

	if (m_keyLocator.size() > 0) {
		signatureInfo.setKeyLocator(m_keyLocator);
	}
	::ndn::EncodingEstimator estimator;
	::ndn::EncodingBuffer encoder(estimator.appendVarNumber(m_signature), 0);
	encoder.appendVarNumber(m_signature);
	dpkt->setSignatureValue(encoder.getBuffer());
	dpkt->wireEncode();

	// 3 send data packet
	NS_LOG_INFO("> Data for " << dseq);
  auto t = Simulator::Now();
	printf("ON SEND: %lu\n", t.GetMilliSeconds());

	m_transmittedDatas(dpkt, this, m_face);
	m_appLink->onReceiveData(*dpkt);
}

void
AppMixSim::SendNDNNack(simple_name_data *nack) {
	NS_LOG_FUNCTION_NOARGS();
	// 1 seperate name and seqnum
	char namebuf[2048];
	char seqbuf[10];
	int idx;
	for (idx = nack->nlen - 1; nack->buf[idx] != '/'; idx--);
	memcpy(namebuf, nack->buf, idx);
	namebuf[idx] = '\0';
	idx++;
	memcpy(seqbuf, nack->buf + idx, nack->nlen - idx);
	seqbuf[nack->nlen - idx] = '\0';
	
	uint32_t nseq = uint32_t(atoi(seqbuf));
	Name nname(namebuf);
	std::cout << "ForwardNDNNack " << nname << "#" << nseq << std::endl;
	
	// 2 make nack packet
	nname.appendSequenceNumber(nseq);
	shared_ptr<Interest> ipkt = make_shared<Interest>();
	ipkt->setName(nname);
	auto npkt = make_shared<lp::Nack>(std::move(*ipkt));
	npkt->setHeader(lp::NackHeader());

	// 3 send nack packet
	NS_LOG_INFO("> Nack for " << nseq);
  auto t = Simulator::Now();
	printf("ON SEND: %lu\n", t.GetMilliSeconds());

	m_transmittedNacks(npkt, this, m_face);
	m_appLink->onReceiveNack(*npkt);
}

void
AppMixSim::SendUDPInterest(shared_ptr<const Interest> interest) {
	simple_name_data namedata;

	uint32_t seq = interest->getName().at(-1).toSequenceNumber();
	std::string name = interest->getName().getPrefix(-1).toUri();
	namedata.nlen = snprintf(namedata.buf, 1024, "%s/%u", name.c_str(), seq);
	namedata.dlen = 0;

	int size = sendto(m_data_udpfd, (void*)&namedata, sizeof(simple_name_data), 0, (struct sockaddr*)&m_interest_raddr, sizeof(m_interest_raddr)); 
	if (size < 0) {
		printf("sock %d want send to %s:%d\n", m_interest_udpfd, inet_ntoa(m_interest_raddr.sin_addr), ntohs(m_interest_raddr.sin_port));
		printf("send udp error\n");
	}
	printf("send udp ok. len=%d. sendto %s:%d\n", size, inet_ntoa(m_interest_raddr.sin_addr), ntohs(m_interest_raddr.sin_port));
}

void
AppMixSim::SendUDPData(shared_ptr<const Data> data) {
	simple_name_data namedata;

	uint32_t seq = data->getName().at(-1).toSequenceNumber();
	std::string name = data->getName().getPrefix(-1).toUri();
	namedata.nlen = snprintf(namedata.buf, 1024, "%s/%u", name.c_str(), seq);
	namedata.dlen = data->getContent().size();
	memcpy(namedata.buf + namedata.nlen, data->getContent().data(), namedata.dlen);
	
	int size = sendto(m_interest_udpfd, (void*)&namedata, sizeof(simple_name_data), 0, (struct sockaddr *)&m_data_raddr, sizeof(m_data_raddr)); 
	if (size < 0) {
		printf("send udp error\n");
	}
	printf("send udp ok. len=%d\n", size);
}

void
AppMixSim::SendUDPNack(shared_ptr<const lp::Nack> nack) {
	simple_name_data namedata;

	uint32_t seq = nack->getInterest().getName().at(-1).toSequenceNumber();
	std::string name = nack->getInterest().getName().getPrefix(-1).toUri();
	namedata.nlen = snprintf(namedata.buf, 1024, "%s/%u", name.c_str(), seq);
	namedata.dlen = 0;

	int size = sendto(m_interest_udpfd, (void*)&namedata, sizeof(simple_name_data), 0, (struct sockaddr *)&m_data_raddr, sizeof(m_data_raddr)); 
	if (size < 0) {
		printf("send udp error\n");
	}
	printf("send udp nack ok. len=%d\n", size);
}


void
AppMixSim::OnData(shared_ptr<const Data> data)
{
  App::OnData(data);
	auto t = Simulator::Now();
	printf("ON DATA: %lu\n", t.GetMilliSeconds());

	SendUDPData(data);
}

void
AppMixSim::OnInterest(shared_ptr<const Interest> interest) {
	App::OnInterest(interest);
	auto t = Simulator::Now();
	printf("ON Interest: %lu\n", t.GetMilliSeconds());

	SendUDPInterest(interest);
}

void
AppMixSim::OnNack(shared_ptr<const lp::Nack> nack) {
	App::OnNack(nack);
	auto t = Simulator::Now();
	printf("ON Nack: %lu\n", t.GetMilliSeconds());

	SendUDPNack(nack);
}

} // namespace ndn
} // namespace ns3