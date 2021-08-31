/*
 * DataReaderListener.h
 *
 *  Created on: Nov 6, 2020
 *      Author: nkumar
 */

#ifndef INCLUDE_DATAREADERLISTENER_H_
#define INCLUDE_DATAREADERLISTENER_H_
#include <dds/DdsDcpsSubscriptionExtC.h>
#include <dds/DCPS/Definitions.h>
#include <dds/DCPS/TypeSupportImpl.h>

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

template<typename T>
class DataReaderListenerImpl
  : public virtual OpenDDS::DCPS::LocalObject<OpenDDS::DCPS::DataReaderListener>
{
public:
  typedef T MessageType;
  typedef std::function<void (const MessageType&)>   RegisteredCallback;

  DataReaderListenerImpl(RegisteredCallback fcn) : callbackFunction_(fcn)
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) DataReaderListenerImpl::")
               ACE_TEXT("DataReaderListenerImpl\n")));
  }

  virtual ~DataReaderListenerImpl()
  {
    ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) DataReaderListenerImpl::")
               ACE_TEXT("~DataReaderListenerImpl\n")));
  }

  virtual void on_requested_deadline_missed(::DDS::DataReader_ptr,
    const ::DDS::RequestedDeadlineMissedStatus&)
  {
    ACE_DEBUG((LM_DEBUG,
      ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_requested_deadline_missed\n")));
  }

  virtual void on_requested_incompatible_qos(::DDS::DataReader_ptr,
    const ::DDS::RequestedIncompatibleQosStatus&)
  {
    ACE_DEBUG((LM_DEBUG,
      ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_requested_incompatible_qos\n")));
  }

  virtual void on_liveliness_changed(::DDS::DataReader_ptr,
    const ::DDS::LivelinessChangedStatus&)
  {
    ACE_DEBUG((LM_DEBUG,
      ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_liveliness_changed\n")));
  }

  virtual void on_subscription_matched(::DDS::DataReader_ptr,
    const ::DDS::SubscriptionMatchedStatus&)
  {
    ACE_DEBUG((LM_DEBUG,
      ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_subscription_matched \n")));
  }

  virtual void on_sample_rejected(::DDS::DataReader_ptr,
    const DDS::SampleRejectedStatus&)
  {
    ACE_DEBUG((LM_DEBUG,
      ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_sample_rejected \n")));
  }

  virtual void on_data_available(::DDS::DataReader_ptr reader)
  {
      typedef OpenDDS::DCPS::DDSTraits<T> TraitsType;
      const typename TraitsType::DataReaderType::_var_type dr = TraitsType::DataReaderType::_narrow(reader);
      MessageType msg;
      DDS::SampleInfo info;

      if (DDS::RETCODE_OK == dr->take_next_sample(msg, info))
      {
          if (info.valid_data)
          {
              callbackFunction_(msg);
          }
      }
  }

  virtual void on_sample_lost(::DDS::DataReader_ptr,
    const DDS::SampleLostStatus&)
  {
    ACE_DEBUG((LM_DEBUG,
      ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_subscription_disconnected \n")));
  }

  virtual void on_subscription_disconnected(::DDS::DataReader_ptr,
    const ::OpenDDS::DCPS::SubscriptionDisconnectedStatus&)
  {
    ACE_DEBUG((LM_DEBUG,
      ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_subscription_disconnected \n")));
  }

  virtual void on_subscription_reconnected(::DDS::DataReader_ptr,
    const ::OpenDDS::DCPS::SubscriptionReconnectedStatus&)
  {
    ACE_DEBUG((LM_DEBUG,
      ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_subscription_reconnected \n")));
  }

  virtual void on_subscription_lost(::DDS::DataReader_ptr,
    const ::OpenDDS::DCPS::SubscriptionLostStatus&)
  {
    ACE_DEBUG((LM_DEBUG,
      ACE_TEXT("(%P|%t) DataReaderListenerImpl::on_subscription_lost \n")));
  }

  virtual void on_budget_exceeded(::DDS::DataReader_ptr,
    const ::OpenDDS::DCPS::BudgetExceededStatus&)
  {
    ACE_DEBUG ((LM_DEBUG, "(%P|%t) received on_budget_exceeded \n"));
  }

private:
  RegisteredCallback callbackFunction_;

};


#endif /* INCLUDE_DATAREADERLISTENER_H_ */
