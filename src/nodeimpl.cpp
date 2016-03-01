#include "nodeimpl.h"

#include <fastrtps/Domain.h>
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>

using namespace eprosima::fastrtps;

namespace commkit
{

NodeImpl::NodeImpl()
{
}

NodeImpl::~NodeImpl()
{
    if (part != nullptr) {
        Domain::removeParticipant(part);
    }
}

bool NodeImpl::init(const NodeOpts &opts)
{
    ParticipantAttributes pa;
    pa.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    pa.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    pa.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    pa.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    pa.rtps.builtin.domainId = opts.domainID;
    pa.rtps.builtin.leaseDuration = c_TimeInfinite;

    pa.rtps.sendSocketBufferSize = 8712;
    pa.rtps.listenSocketBufferSize = 17424;

    pa.rtps.setName(opts.name.c_str());

    part = Domain::createParticipant(pa);
    return (part != nullptr);
}

} // namespace commkit
