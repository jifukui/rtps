#include "HelloKylandPubSubTypes.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>

#include <unistd.h>

using namespace eprosima::fastdds::dds;

class HelloKylandSubscriber
{
public:
    //初始化构造函数
    HelloKylandSubscriber() : participant_(nullptr), subscriber_(nullptr), reader_(nullptr), topic_(nullptr), type_(new HelloKylandPubSubType())
    {
    }
    //析构函数
    ~HelloKylandSubscriber()
    {
        //判断各实体是否存在，存在则删除
        if(subscriber_ != nullptr)
        {
            participant_->delete_subscriber(subscriber_);
        }
        if(reader_ != nullptr)
        {
            subscriber_->delete_datareader(reader_);
        }
        if(topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }

        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    //初始化
    bool init()
    {
        //定义域参与者qos策略
        DomainParticipantQos participant_qos;
        //participant_qos.name("participant_subscriber");
        //由域参与者工厂创建域参与者，并指定域id，域参与者qos策略
        participant_ = DomainParticipantFactory::get_instance()->create_participant(1, participant_qos);
        //判断是否创建成功
        if(participant_ == nullptr)
        {
            return false;
        }

        //注册数据类型
        type_.register_type(participant_);

        //由域参与者创建话题  并指定话题名， 类型名，话题qos策略  --话题名和类型名都作为标识符，对应不上无法通信
        topic_ = participant_->create_topic("HelloKylandTopic", "HelloKyland",TOPIC_QOS_DEFAULT);
        //判断是否创建成功
        if(topic_ == nullptr)
        {
            return false;
        }


        //由域参与者创建订阅者  并指定订阅者qos策略
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        if(subscriber_ == nullptr)
        {
            return false;
        }

    
        //由订阅者创建datareader  并指定话题，datareader的qos策略，回调函数
        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);
        if(reader_ == nullptr)
        {
            return false;
        }

        return true;
    }

    void run()
    {
        while(true)
        {
            sleep(1);
        }
    }

private:
    //定义域参与者
    DomainParticipant* participant_;
    //定义订阅者
    Subscriber* subscriber_;
    //定义datareader
    DataReader* reader_;
    //定义话题
    Topic* topic_;
    //定义支持的类型
    TypeSupport type_;
    //定义回调函数  ，对接受到的数据处理
    class SubListener : public DataReaderListener
    {
    public:
        SubListener()
        {
        }

        ~SubListener() override
        {
        }
        //判断订阅者是否连接上发布者
        void on_subscription_matched(DataReader*, const SubscriptionMatchedStatus& info) override
        {
            if(info.current_count_change == 1)
            {
                std::cout << "subscriber matched" << std::endl;
            }
            else if(info.current_count_change == -1)
            {
                std::cout << "subscriber unmatched" << std::endl;
            }
            else{
                std::cout << info.current_count_change << "is not a valid value" << std::endl;
            }
        }
        //获取接受到的数据，作处理
        void on_data_available(DataReader* reader) override
        {
            SampleInfo info;
            if(reader->take_next_sample(&hello_, &info) == ReturnCode_t::RETCODE_OK)
            {
                if(info.valid_data)
                {
                    std::cout << hello_.message() << std::endl;
                }
            }
        }

        //在idl文件中自定义的数据类型
        HelloKyland hello_;
    }listener_;
};

int main(int argc, char** argv)
{
    std::cout << "starting subscriber" << std::endl;
    //创建订阅者者指针
    HelloKylandSubscriber* sub = new HelloKylandSubscriber();
    if(sub->init())
    {
        sub->run();
    }
    delete sub;
    return 0;
}