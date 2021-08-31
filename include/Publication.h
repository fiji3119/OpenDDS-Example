/*
 * Publication.cpp
 *
 *  Created on: Nov 9, 2020
 *      Author: nkumar
 */
#ifndef SRC_PUBLICATION_H_
#define SRC_PUBLICATION_H_
#include <string>
#include "Common.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Qos_Helper.h>
#include <dds/DCPS/TopicDescriptionImpl.h>
#include <dds/DCPS/PublisherImpl.h>
#include <dds/DdsDcpsPublicationC.h>
#include <dds/DCPS/WaitSet.h>
#include <dds/DCPS/TypeSupportImpl.h>

#include <ace/Task.h>

namespace MyExample {

class PublicationBase : public ACE_Task_Base {
public:
    virtual ~PublicationBase() = default;
    virtual DDS::ReturnCode_t enable() { return DDS::RETCODE_OK; }
    int32_t size_;
};

template <typename T>
class Publication : public PublicationBase {
public:
    typedef T MessageType;
    typedef OpenDDS::DCPS::DDSTraits<T> TraitsType;
    typedef typename TraitsType::DataWriterType::_var_type DataWriter_var;
//    typedef typename TraitsType::T DSample;

    Publication(int32_t argc, char *argv[], const std::string& topic);
    Publication(DDS::DomainParticipantFactory_var dfp, const std::string& topic);

    virtual ~Publication();
    DDS::ReturnCode_t enable();
    DDS::ReturnCode_t publish(const MessageType& msg);
    DDS::DomainParticipant_var get_participant() { return participant_; }

    //Task_Base interfaces.
    virtual int32_t open(void*);
    virtual int32_t svc();
    virtual int32_t close( uint64_t flags = 0);
    //Thread control.
    void start();
    void stop();
    DDS::ReturnCode_t wait_match(int32_t count=1);

private:
    /* DDS */
    DDS::DomainParticipantFactory_var  dpf_;
    DDS::DomainParticipant_var participant_;
    DataWriter_var   writer_;
    const std::string&  topicName_;

};



//======================================================//
// Function: Constructor
// Description:
// Return: None
//======================================================//
template <typename T>
Publication<T>::Publication(int32_t argc, char *argv[], const std::string& topic) :
             topicName_(topic)
{

}

template <typename T>
Publication<T>::Publication(DDS::DomainParticipantFactory_var dpf, const std::string& topic) :
               dpf_(dpf), topicName_(topic)
{
}

template <typename T>
Publication<T>::~Publication()
{
    participant_->delete_contained_entities();
}

//======================================================//
// Function: open(void *arg)
// Description: Initialize thread, call via start()
// Return: None
//======================================================//
template <typename T>
int32_t Publication<T>::open(void *arg)
{
    //--TBD--
    return 0;
}

//======================================================//
// Function: close (uint64_t flag)
// Description: Exit thread
// Return: None
//======================================================//
template <typename T>
int32_t Publication<T>::close(uint64_t flag)
{
    //--TBD--
    return 0;
}

//======================================================//
// Function: start()
// Description: Start thread, via open()
// Return: None
//======================================================//
template <typename T>
void Publication<T>::start()
{
    //--TBD--

}

//======================================================//
// Function: stop()
// Description: Exit forever loop in thread
// Return: None
//======================================================//
template <typename T>
void Publication<T>::stop()
{
    //--TBD--
}

//======================================================//
// Function: svc()
// Description: Method running under thread, generate data
//              i.e publishing data at specific rate (Hz)
// Return: None
//======================================================//
template <typename T>
int32_t Publication<T>::svc()
{
    //--TBD--
    return 0;
}

//======================================================//
// Function: enable (DDS::DomainParticipantFactory_var dpf)
// Description: Creates participant, publisher, topic and
//              data reader for this message type
// Return: None
//======================================================//
template <typename T>
DDS::ReturnCode_t
Publication<T>::enable()
{
    if (CORBA::is_nil(dpf_.in()))
    {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("(%P|%t): Publication:%s Domain Participant Factory does not exist!\n"), __FUNCTION__),1);
    }

    DDS::ReturnCode_t ret = DDS::RETCODE_OK;
    //Create Participant
    participant_ =  dpf_->create_participant(DOMAIN_ID,
                                            PARTICIPANT_QOS_DEFAULT,
                                            ::DDS::DomainParticipantListener::_nil() ,
                                            ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);

    if (CORBA::is_nil(participant_.in()))
    {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("(%P|%t): Publication:%s create_participant failed.\n"), __FUNCTION__),1);
    }

    // Create the publisher
    ::DDS::Publisher_var pub = participant_->create_publisher(PUBLISHER_QOS_DEFAULT,
                                                              DDS::PublisherListener::_nil(),
                                                              OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (CORBA::is_nil(pub.in()))
    {
      ACE_ERROR_RETURN((LM_ERROR,
                        ACE_TEXT("(%P|%t) create_publisher failed.\n")),
                        1);
    }

    // Create the datawriters
    DDS::DataWriterQos dw_qos;
    pub->get_default_datawriter_qos(dw_qos);
    /*************************
     * Set QoS here, if needed
     ***********************/
    const DDS::TypeSupport_var ts = new typename OpenDDS::DCPS::DDSTraits<T>::TypeSupportTypeImpl();
    const CORBA::String_var type_name = ts->get_type_name();
    ACE_DEBUG((LM_INFO, ACE_TEXT("%P:%l: Type name = %C\n"), type_name.in()));

    //Register type
    ret = ts->register_type(participant_, type_name);
    if (ret != ::DDS::RETCODE_OK)
    {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("ERROR: %N:%l: Publication::%s -")
                ACE_TEXT("%C register_type failed!\n"),
                __FUNCTION__, type_name),
                1);
    }

    //Create topic
    const DDS::Topic_var topic = participant_->create_topic(topicName_.c_str(),
                                                            type_name,
                                                            TOPIC_QOS_DEFAULT,
                                                            0,
                                                            OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (!topic)
    {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("ERROR: %N:%l: Publication::%s -")
                ACE_TEXT("%C create_topic failed!\n"), __FUNCTION__, topicName_.c_str() ),
                1);
    }

    //Narrow'd writer for publishing data
    const DDS::DataWriter_var writer  = pub->create_datawriter(topic,
                                                           dw_qos,
                                                           DDS::DataWriterListener::_nil(),
                                                           OpenDDS::DCPS::DEFAULT_STATUS_MASK);
    if (CORBA::is_nil(writer))
    {
        ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("ERROR: %N:%l: Publication:%s -")
                ACE_TEXT(" create_datawriter failed!\n"), __FUNCTION__),
                1);
    }

    //Save this, you need it when publishing
    writer_ = OpenDDS::DCPS::DDSTraits<T>::DataWriterType::_narrow(writer);

    //Wait
    wait_match();

    return ret;
}

//======================================================//
// Function: publish(const T&)
// Description: Publish data. To publish data under separate
//              thread, use svc()
// Return: None
//======================================================//
template <typename T>
DDS::ReturnCode_t Publication<T>::publish(const T& msg)
{
    DDS::ReturnCode_t ret = DDS::RETCODE_OK;

//    const DDS::DataWriter_var dw = DDS::DataWriter::_duplicate(writer_);
    DDS::InstanceHandle_t handle = writer_->register_instance(msg);
    ret = writer_->write(msg, handle);

    return ret;
}


//======================================================//
// Function:   wait_match (int32_t count)
// Description: Waits for appropriate subscriber to come up
// Return:  DDS::ReturnCode_t type
//======================================================//
template <typename T>
DDS::ReturnCode_t Publication<T>::wait_match(int32_t count)
{
    DDS::StatusCondition_var condition = writer_->get_statuscondition();
    condition->set_enabled_statuses(DDS::PUBLICATION_MATCHED_STATUS);

    DDS::WaitSet_var ws(new DDS::WaitSet);
    ws->attach_condition(condition);

    DDS::ReturnCode_t stat;

    DDS::ConditionSeq conditions;
    DDS::PublicationMatchedStatus ms = { 0, 0, 0, 0, 0 };

    const int32_t sec = 3; //secs
    const uint32_t nanosec = 0;
    const DDS::Duration_t wake_interval = { sec, nanosec };
    while (true)
    {
        stat = writer_->get_publication_matched_status(ms);

        if (ms.current_count | ms.current_count_change | ms.total_count | ms.total_count_change)
        {
//            std::cout << ms.current_count << ",";
//            std::cout << ms.current_count_change << ",";
//            std::cout << ms.total_count << ",";
//            std::cout << ms.total_count_change << std::endl;
        }

        if (stat != DDS::RETCODE_OK)
        {
          ACE_ERROR_RETURN((
                      LM_ERROR,
                      ACE_TEXT("(%P|%t) ERROR: %N:%l: wait_match() -")
                      ACE_TEXT(" get_publication_matched_status failed!\n")),
                      stat);
        }

        if (ms.current_count >= count ) // && ms.total_count > count)
        {
            break;
        }
        // wait for a change
        stat = ws->wait(conditions, wake_interval);
        if ((stat != DDS::RETCODE_OK) && (stat != DDS::RETCODE_TIMEOUT))
        {
          ACE_ERROR_RETURN((LM_ERROR,
                            ACE_TEXT("(%P|%t) ERROR: %N:%l: wait_match() -")
                            ACE_TEXT(" wait failed!\n")),
                            stat);
        }//endif
    }//endwhile

    ws->detach_condition(condition);

    return stat;
}

} /* namespace MyExample */
#endif /* SRC_PUBLICATION_H_ */

