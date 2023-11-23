//包含idl文件中自定义数据类型，并通过fastddsgen工具生成的数据类型头文件
#include "HelloKylandPubSubTypes.h"


//包含fastdds本身的头文件
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>

#include <iostream>
#include <unistd.h>

using namespace eprosima::fastdds::dds;

class HelloKylandPublisher
{
public:
    //初始化构造函数
    HelloKylandPublisher() : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr), type_(new HelloKylandPubSubType())
    {
    }

    //析构函数
    virtual ~HelloKylandPublisher()
    {
        //判断各实体是否存在，存在则删除
        if(writer_ != nullptr)
        {
            publisher_->delete_datawriter(writer_);
        }
        if(publisher_ != nullptr)
        {
            participant_->delete_publisher(publisher_);
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
        //传输的数据内容
        hello_.message("hello kyland");
        //定义域参与者qos策略
        DomainParticipantQos participant_qos;
        //participant_qos.name("participant_publisher");

        //由域参与者工厂创建域参与者，并指定域id，域参与者qos策略
        participant_ = DomainParticipantFactory::get_instance()->create_participant(1, participant_qos);
        //判断是否创建成功
        if(participant_ == nullptr)
        {
            return false;
        }

        //注册数据类型
        type_.register_type(participant_);
        
        //由域参与者创建话题  并指定话题名， 类型名，话题qos策略    --话题名和类型名都作为标识符，对应不上无法通信
        topic_ = participant_->create_topic("HelloKylandTopic", "HelloKyland", TOPIC_QOS_DEFAULT);
        //判断是否创建成功
        if(topic_ == nullptr)
        {
            return false;
        }

        //由域参与者创建发布者  并指定发布者qos策略
        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
        if(publisher_ == nullptr)
        {
            return false;
        }

        //由发布者创建datawriter，并指定话题，datawriter的qos策略
        writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT);
        if(writer_ == nullptr)
        {
            return false;
        }
        return true;
    }

    bool publish()
    {
        //datawriter调用write方法，并将传输的数据写入
        writer_->write(&hello_);
    }

    void run()
    {
        while(publish())
        {
            std::cout << hello_.message() << std::endl;
            sleep(1);
        }   
    }

private:

    //在idl文件中自定义的数据类型
    HelloKyland hello_;
    //定义域参与者
    DomainParticipant* participant_;
    //定义发布者
    Publisher* publisher_;
    //定义话题
    Topic* topic_;
    //定义datawriter
    DataWriter* writer_;
    //定义支持的类型
    TypeSupport type_;
};

int main(int argc, char** argv)
{
    std::cout << "starting publihser" << std::endl;
    //创建发布者指针
    HelloKylandPublisher* pub = new HelloKylandPublisher();
    if(pub->init())
    {
        pub->run();
    }

    delete pub;
    return 0;
}