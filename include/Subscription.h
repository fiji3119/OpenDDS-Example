/*
 * Subscription.cpp
 *
 *  Created on: Nov 6, 2020
 *      Author: nkumar
 */
#ifndef SRC_SUBSCRIPTION_H_
#define SRC_SUBSCRIPTION_H_

#include <string>
#include <functional>
#include "Common.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Qos_Helper.h>
#include <dds/DCPS/TopicDescriptionImpl.h>
#include <dds/DCPS/SubscriberImpl.h>
#include <dds/DdsDcpsSubscriptionC.h>
#include <dds/DCPS/WaitSet.h>

#include "DataReaderListener.h"


namespace MyExample {

class SubscriptionBase {
public:
    virtual ~SubscriptionBase() = default;
    virtual DDS::ReturnCode_t enable() { return DDS::RETCODE_OK; }
    int32_t  size_;
};

template <typename T>
class Subscription : public SubscriptionBase {
public:
    typedef T MessageType;
    typedef std::function<void(const MessageType&)> listenerCallback;
    Subscription(int32_t argc,
            char *argv[],
            const std::string& topic,
            listenerCallback fcn);

    Subscription(DDS::DomainParticipantFactory_var dfp, const std::string& topic, listenerCallback fcn);

    virtual ~Subscription();
    void register_callback(listenerCallback fcn) { userCallerback_ = fcn; }
    DDS::ReturnCode_t enable();
    DDS::DomainParticipant_var get_participant() { return participant_; }
    DDS::ReturnCode_t wait_match(int32_t count=1);

private:
    /* DDS */
    DDS::DomainParticipantFactory_var  dpf_;
    DDS::DomainParticipant_var participant_;
    listenerCallback userCallerback_;
    const std::string&  topicName_;
    DDS::DataReader_var  reader_;
};

// Implementation

//======================================================//
// Function: Constructor
// Description:
// Return: None
//======================================================//
template <typename T>
Subscription<T>::Subscription(int32_t argc,
        char *argv[],
        const std::string& topic_name,
        listenerCallback fcn) : userCallerback_(fcn), topicName_(topic_name)
{
}

template <typename T>
Subscription<T>::Subscription(DDS::DomainParticipantFactory_var dfp, const std::string& topic, listenerCallback fcn) :
             dpf_(dfp), topicName_(topic), userCallerback_(fcn)
{
}

template <typename T>
Subscription<T>::~Subscription()
{
    participant_->delete_contained_entities();
}
//======================================================//
// Function: enable (DDS::DomainParticipantFactory_var dpf)
// Description: Creates participant, subscriber, topic and
//              data reader for this message type
// Return: None
//======================================================//
template <typename T>
::DDS::ReturnCode_t
 Subscription<T>::enable()
{
    DDS::ReturnCode_t ret = DDS::RETCODE_OK;

    if (CORBA::is_nil(dpf_.in()))
    {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("(%P|%t): Subscription:%s Domain Participant Factory does not exist!\n"), __FUNCTION__),1);
    }

    //Create Participant
    participant_ =	dpf_->create_participant(DOMAIN_ID,
                                            PARTICIPANT_QOS_DEFAULT,
                                            ::DDS::DomainParticipantListener::_nil() ,
                                            ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (CORBA::is_nil(participant_.in()))
    {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("(%P|%t): Subscription:%s create_participant failed.\n"), __FUNCTION__),1);
    }
    //Create subscriber
    ::DDS::Subscriber_var sub =  participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT,
                                                                ::DDS::SubscriberListener::_nil(),
                                                                ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (CORBA::is_nil(sub))
    {
        ACE_ERROR_RETURN((LM_ERROR,
                          ACE_TEXT("(%P|%t): Subscription:%s create_subscriber failed.\n"),
                          __FUNCTION__),
                          1);
    }

    //Create datareaders
    ::DDS::DataReaderQos dr_qos;
    ret = sub->get_default_datareader_qos(dr_qos);
    /*************************
     * Set QoS here, if needed
     * TBD, no now
     ***********************/
    //Create listener
    ::DDS::DataReaderListener_var listener = new DataReaderListenerImpl<T>(userCallerback_);
    const ::DDS::TypeSupport_var ts = new typename ::OpenDDS::DCPS::DDSTraits<T>::TypeSupportTypeImpl();
    const CORBA::String_var type_name = ts->get_type_name();

    ACE_DEBUG((LM_INFO, ACE_TEXT("%P:%l: Type name = %C\n"), type_name.in()));

    ret = ts->register_type(participant_, type_name);
    if (ret != ::DDS::RETCODE_OK)
    {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("ERROR: %N:%l: Subscription::%s -")
                ACE_TEXT("%C register_type failed!\n"),
                __FUNCTION__, type_name),
                1);
    }
    const DDS::Topic_var topic = participant_->create_topic(topicName_.c_str(),
                                                            type_name,
                                                            TOPIC_QOS_DEFAULT,
                                                            0,
                                                            OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (!topic)
    {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("ERROR: %N:%l: Subscription::%s -")
                ACE_TEXT("%C create_topic failed!\n"), __FUNCTION__, topicName_.c_str() ),
                1);
    }

    ::DDS::DataReader_var reader = sub->create_datareader(topic,
                                                          dr_qos,
                                                          listener,
                                                          OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (!reader) {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("ERROR: %N:%l: main() -")
                ACE_TEXT(" create_datareader failed!\n")),
                1);
    }
    DDS::DataReader_var reader_i = OpenDDS::DCPS::DDSTraits<T>::DataReaderType::_narrow(reader);
    if (!reader_i) {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("ERROR: %N:%l: Subscription:%s -")
                ACE_TEXT(" DataReader _narrow failed!\n"), __FUNCTION__),
                1);
    }

    //Save reader
    reader_ = reader_i;

    //Wait for match
    wait_match();

    return ret;
}

//======================================================//
// Function:   wait_match (int32_t count)
// Description: Waits for appropriate publisher to come up
// Return:  DDS::ReturnCode_t type
//======================================================//
template <typename T>
DDS::ReturnCode_t Subscription<T>::wait_match(int32_t count)
{
    DDS::ReturnCode_t stat;
    DDS::StatusCondition_var condition = reader_->get_statuscondition();
    condition->set_enabled_statuses(DDS::SUBSCRIPTION_MATCHED_STATUS);

    DDS::WaitSet_var ws = new DDS::WaitSet;
    ws->attach_condition(condition);

    while (true) {
        DDS::SubscriptionMatchedStatus matches;
        stat = reader_->get_subscription_matched_status(matches);
        if ( stat != DDS::RETCODE_OK) {
            ACE_ERROR_RETURN((LM_ERROR,
                    ACE_TEXT("ERROR: %N:%l: main() -")
                    ACE_TEXT(" get_subscription_matched_status failed!\n")),
                    stat);
        }

        if (matches.current_count >= count) // && matches.total_count > 0)
        {
            break;
        }

        DDS::ConditionSeq conditions;
        DDS::Duration_t timeout = { 60, 0 };
        stat = ws->wait(conditions, timeout);
        if ( stat != DDS::RETCODE_OK) {
            ACE_ERROR_RETURN((LM_ERROR,
                    ACE_TEXT("ERROR: %N:%l: main() -")
                    ACE_TEXT(" wait failed!\n")),
                    stat);
        }
    } //endwhile

    ws->detach_condition(condition);
    return stat;
}

} /* namespace MyExample */

#endif /* SRC_SUBSCRIPTION_H_ */

