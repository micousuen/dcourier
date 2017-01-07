/* 
 * File:   Com.hpp
 * Author: root
 *
 * Created on August 29, 2016, 11:00 AM
 */

// Include communication through network(socket) and in program(in threads)
#ifndef Com_hpp
#define Com_hpp

#define MAXRECVLENGTH      5000
#define TARGETPER_DELI     "|"
#define TARGETPER_DEFAULT  "*"
#define SOURCECLIENTINFO_DEFAULT "none"
#define CHECK_INTERVAL     10
#define THREAD_EXIT_WAIT   1000

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <map>
#include <vector>
#include <deque>
#include <iostream>
#include <string.h> // For memset
#include <errno.h>
#include <fcntl.h>
#include "utils.hpp"

// For communication on network using socket
namespace netcom{
    
    // Include: 
    // Socket Initialization: create, bind, listen, accept, connect
    // Data transmission: send, recv
    // Set and manipulate socket: set_non_blocking, GetAddressBySocket, is_valid, getsock, setsock, close
    class Socket
    {
     public:
        bool is_valid() const { return m_sock != -1; }
        
        Socket() :
        m_sock ( -1 )
        {
            memset ( &m_addr,
                     0,
                     sizeof ( m_addr ) );
         }

        virtual ~Socket()
        {
            if ( is_valid() )
            ::close ( m_sock );
        }

        bool close(){
            if ( is_valid() ){
                ::close( m_sock );
                m_sock=-1;
                return true;
            }
            else{
                return false;
            }
        }

        bool create()
        {
          m_sock = socket ( AF_INET,
                            SOCK_STREAM,
                            0 );

          if ( !is_valid() )
            return false;

          // TIME_WAIT - argh
          int on = 1;
          if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
            return false;

          return true;

        }

        bool bind ( const int port )
        {
          if ( ! is_valid() )
            {
              return false;
            }

          m_addr.sin_family = AF_INET;
          m_addr.sin_addr.s_addr = INADDR_ANY;
          m_addr.sin_port = htons ( port );

          int bind_return = ::bind ( m_sock,
                                     ( struct sockaddr * ) &m_addr,
                                     sizeof ( m_addr ) );

          if ( bind_return == -1 )
            {
              return false;
            }

          return true;
        }

        bool listen() const
        {
          if ( ! is_valid() )
            {
              return false;
            }
          int listen_return = ::listen ( m_sock, MAXCONNECTIONS );
          if ( listen_return == -1 )
            {
              return false;
            }
          return true;
        }

        bool accept ( Socket& new_socket ) const
        {
          int addr_length = sizeof ( m_addr );
          new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );

          if ( new_socket.m_sock <= 0 )
            return false;
          else
            return true;
        }

        bool send ( const std::string s ) const
        {
          int status = ::send ( m_sock, s.c_str(), s.size(), MSG_NOSIGNAL );
          if ( status == -1 )
            {
              return false;
            }
          else
            {
              return true;
            }
        }

        int recv ( std::string& s ) const
        {
          char buf [ MAXRECV + 1 ];
          s = "";
          memset ( buf, 0, MAXRECV + 1 );
          int status = ::recv ( m_sock, buf, MAXRECV, 0 );
          if ( status == -1 )
            {
              std::cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
              return 0;
            }
          else if ( status == 0 )
            {
              return 0;
            }
          else
            {
              s = buf;
              return status;
            }
        }

        bool connect ( const std::string host, const int port )
        {
          if ( ! is_valid() ) return false;

          m_addr.sin_family = AF_INET;
          m_addr.sin_port = htons ( port );

          int status = inet_pton ( AF_INET, host.c_str(), &m_addr.sin_addr );

          if ( errno == EAFNOSUPPORT ) return false;

          status = ::connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );

          if ( status == 0 )
            return true;
          else
            return false;
        }

        void set_non_blocking ( const bool b )
        {
          int opts;
          opts = fcntl ( m_sock,
                         F_GETFL );
          if ( opts < 0 )
            {
              return;
            }
          if ( b )
            opts = ( opts | O_NONBLOCK );
          else
            opts = ( opts & ~O_NONBLOCK );
          fcntl ( m_sock,
                  F_SETFL,opts );
        }
        
        bool GetAddressBySocket(sockaddr_in &m_address){
            memset(&m_address, 0, sizeof(m_address));
            socklen_t nAddrLen = sizeof(m_address);
            if(::getpeername(m_sock, 
                            (sockaddr* )&m_address, 
                            &nAddrLen)!=0){
                return false;
            }
            else{
                return true;
            }
        }
        
     private:
        int m_sock;
        sockaddr_in m_addr;
        const int MAXHOSTNAME = 200;
        const int MAXCONNECTIONS = 128;
        const int MAXRECV = MAXRECVLENGTH;
    };
}

// For communication in program (between threads)
namespace procom{
    struct datacontainer{
        std::string data;
        std::string srcinfo;
        std::string dstinfo;
        std::string note;
        void clear(){
            data.clear();
            srcinfo.clear();
            dstinfo.clear();
            note.clear();
        };
    };
    
    
    // By now, the map in dequeue will contain: Data, ClientID, TargetPermission
    class datatrans
    {
    public:
        std::mutex _door;
        std::condition_variable * global_cv;
        std::condition_variable local_cv;
        datatrans(){
            global_cv = &default_cv;
        }
        // Set condition_variable. 
        virtual void set_global_cv(std::condition_variable * new_cv){
            global_cv = new_cv;
        }
        // Set default condition_variable
        virtual void set_global_cv_default(){
            global_cv = &default_cv;
        }
        // Check there is data in data_trans_buffer, this is not thread safe, just as a judge reference
        virtual bool buffer_is_valid(){
            return (data_trans_buffer.size()!=0);
        }
        // Get size of buffer, thread safe
        virtual std::size_t get_buffer_size(){
            return data_trans_buffer.size();
        }
        // Get max size of this dequeue
        size_t get_size_limit(){
            return size_limit;
        }
        // Set max size of this dequeue
        void set_size_limit(size_t max_size){
            *const_cast<std::size_t *>(&size_limit)=max_size;
        }
    protected:
        std::deque < datacontainer > data_trans_buffer;
    private:
        std::condition_variable default_cv;
        const std::size_t size_limit=10000;
    };
    
    // Data can only flow in this datatrans
    class bdatatrans:public datatrans
    {
    public:
        // Push data to the back of buffer, data should be in datacontainer format
        virtual void push_back(datacontainer dp){
            data_trans_buffer.push_back(dp);
        }
        // Push data to the front of buffer, data should be in datacontainer format
        virtual void push_front(datacontainer dp){
            data_trans_buffer.push_front(dp);
        }
        // If the dequeue is full, block data push_back process
        virtual void push_back_fullblock(datacontainer dp){
            if(get_buffer_size()<get_size_limit()){
                data_trans_buffer.push_back(dp);
            }
            else{
                std::unique_lock<std::mutex> full_blk(full_block);
                local_cv.wait(full_blk);
                data_trans_buffer.push_back(dp);
            }
        }
        // If the dequeue is full, block data push_front process
        virtual void push_front_fullblock(datacontainer dp){
            if(get_buffer_size()<get_size_limit()){
                data_trans_buffer.push_front(dp);
            }
            else{
                std::unique_lock<std::mutex> full_blk(full_block);
                local_cv.wait(full_blk);
                data_trans_buffer.push_back(dp);
            }
        }
        // Get data in datacontainer format from front
        virtual datacontainer get_front(){
            return data_trans_buffer.front();
        }
        // Get data in datacontainer format from back
        virtual datacontainer get_back(){
            return data_trans_buffer.back();
        }
        // Pop data from the front
        virtual void pop_front(){
            data_trans_buffer.pop_front();
            data_trans_buffer.shrink_to_fit();
            local_cv.notify_all();
        }
        // Pop data from the back
        virtual void pop_back(){
            data_trans_buffer.pop_back();
            data_trans_buffer.shrink_to_fit();
            local_cv.notify_all();
        }
        virtual std::deque<datacontainer>::iterator get_begin_iterator(){
            return data_trans_buffer.begin();
        }
        virtual std::deque<datacontainer>::iterator get_end_iterator(){
            return data_trans_buffer.end();
        }
        virtual void clear(){
            data_trans_buffer.clear();
        }
    private:
        std::mutex full_block;
    };
    
    // Data can only flow in this datatrans
    class idatatrans:public bdatatrans
    {
    public:
        // Push data to the back of buffer. Only data string is required, note and targetPermission are optional.
        virtual void push_back_data(std::string data, std::string destination = TARGETPER_DEFAULT, std::string note = SOURCECLIENTINFO_DEFAULT){
            packer(data,destination,note);
            _door.lock();
            procom::bdatatrans::push_back(dp);
            _door.unlock();
            // Once data been pushed to dequeue, notify all condition variable
            global_cv->notify_all();
            datapack.clear();
        }
        // Push data to the front of buffer. Only data string is required, note and targetPermission are optional.
        virtual void push_front_data(std::string data, std::string destination = TARGETPER_DEFAULT, std::string note = SOURCECLIENTINFO_DEFAULT){
            packer(data,destination,note);
            _door.lock();
            procom::bdatatrans::push_front(dp);
            _door.unlock();
            // Once data been pushed to dequeue, notify all condition variable
            global_cv->notify_all();
            datapack.clear();
        }
    private:
        // Temporary map to store packed data.
        datacontainer datapack;
        datacontainer dp;
        // Pack data with data, note and targetPermission strings.
        // The default of targetPermission is all target("*"). If you want to point out which targets should this data go, 
        // organize TargetID together with a delimiter "|", like "fileout|socketout|logout"
        virtual void packer(std::string data, std::string destination, std::string note){
            dp.data=data;
            dp.note=note;
            dp.dstinfo=destination;
        }
    };
    
    // Data can only flow out this datatrans
    class odatatrans:public bdatatrans
    {
    public:
        // Only get data from the front of buffer. Other parts(note and targetPermission) will be thrown. Data will erased in buffer after this.
        virtual std::string get_front_data(){
            if(buffer_is_valid()){
                tempstr.clear();
                _door.lock();
                tempstr=procom::bdatatrans::get_front().data;
                procom::bdatatrans::pop_front();
                _door.unlock();
                return tempstr;
            }
            else
                return "";
        }
        virtual void get_front_databunch(std::vector<std::string> & databunch){
            if(buffer_is_valid()){
                tempstr.clear();
                _door.lock();
                for(std::deque<datacontainer>::iterator bg=procom::bdatatrans::get_begin_iterator();bg!=procom::bdatatrans::get_end_iterator();bg++){
                    databunch.push_back(bg->data);
                }
                procom::bdatatrans::clear();
                _door.unlock();
            }
        }
        // Get all parameters in data map from the front of buffer. Data will erased in buffer after this. 
        virtual bool get_front_sep(std::string & data, std::string & source, std::string & note){
            if(buffer_is_valid()){
                data.clear();
                note.clear();
                source.clear();
                _door.lock();
                data=procom::bdatatrans::get_front().data;
                note=procom::bdatatrans::get_front().note;
                source=procom::bdatatrans::get_front().srcinfo;
                procom::bdatatrans::pop_front();
                _door.unlock();
                return true;
            }
            else
                return false;
        }
        // Only get data from the back of buffer. Other parts(note and targetPermission) will be thrown. Data will erased in buffer after this.
        virtual std::string get_back_data(){
            if(buffer_is_valid()){
                tempstr.clear();
                _door.lock();
                tempstr=procom::bdatatrans::get_back().data;
                procom::bdatatrans::pop_back();
                _door.unlock();
                return tempstr;
            }
            else
                return "";
        }
        // Get all parameters in data map from the back of buffer. Data will erased in buffer after this. 
        virtual bool get_back_sep(std::string & data, std::string & source, std::string & note){
            if(buffer_is_valid()){
                data.clear();
                note.clear();
                source.clear();
                _door.lock();
                data=procom::bdatatrans::get_back().data;
                note=procom::bdatatrans::get_back().note;
                source=procom::bdatatrans::get_back().srcinfo;
                procom::bdatatrans::pop_back();
                _door.unlock();
                return true;
            }
            else
                return false;
        }
    private:
        // temp string to process data
        std::string tempstr;
    };
    
    class iodatatrans: public idatatrans, public odatatrans
    {
        
    };
    
    // When input or output will run as a thread, use this interface class
    class run_as_thread
    {
    public:
        virtual bool thread_run()=0;
        virtual bool thread_break()=0;
    };
    
    // Deliver data from source to target (data contained in datatrans)
    class delivery:public run_as_thread
    {
    public:
        // Set delimeters using symbols defined at the top of this file
        delivery() {
            //run_counter=0;
            TargetPer_default=TARGETPER_DEFAULT;
            TargetPer_deli=TARGETPER_DELI;
        }
        
        // Add a source to source list. sourceID is required. source must derived from idatatrans.
        void add_source(std::string sourceID, procom::bdatatrans * source){
            _door.lock();
            sourcelist.insert(std::make_pair(sourceID, source));
            clear_null_value(sourcelist);
            _door.unlock();
            // Set source's condition variable as the same condition variable in this class
            source->set_global_cv(&datain_cv);
        }
        
        // Add a target to target list. targetID is required. target must derived from odatatrans.
        void add_target(std::string targetID, procom::bdatatrans * target){
            _door.lock();
            targetlist.insert(std::make_pair(targetID, target));
            clear_null_value(targetlist);
            _door.unlock();
        }
        
        // Check if this delivery is set correctly. A least a source and a target is needed.
        bool delivery_is_valid(){
            return (sourcelist.size()!=0 && targetlist.size()!=0 );
        }
        
        // Deliver data from source to target for one time
        bool deliver(int mode = 0){
            // Make sure at least one source and one target exist
            if( delivery_is_valid() ){
                // Use this flag to detect if at least one data exists in all sources，after function running, if this flag is still true, this function will return false
                sourceEmptyFlag=true;
                // From the very beginning to collect data and send to targets, this collection is not so fair, need to be fixed
                for(list_it = sourcelist.begin();list_it!=sourcelist.end();list_it++){
                    // Lock the door and do something
                    list_it->second->_door.lock();
                    // Check this source if there are data
                    for(data_persource_counter=0,data_persource_max=(list_it->second->get_buffer_size());data_persource_counter<data_persource_max;++data_persource_counter){
//                        std::cout<<m_utils::log("Count of this is "+std::to_string(data_persource_counter),3);
                        sourceEmptyFlag=false;
                        // Get every thing in this data pack
                        temp_datapack=list_it->second->get_front();
                        // Erase data from source
                        list_it->second->pop_front();
                        // Get Targets
                        temp_targetPer_str=temp_datapack.dstinfo;
                        // If targets are not set, send them to every target
                        if(temp_targetPer_str == TargetPer_default ){
                            for(list_it_out = targetlist.begin();list_it_out!=targetlist.end();list_it_out++){
                                // Add target lock to make this thread safe
                                list_it_out->second->_door.lock();
                                // set source info in datacontainer
                                temp_datapack.srcinfo=list_it->first;
                                // push data to this destination
                                list_it_out->second->push_back(temp_datapack);
                                list_it_out->second->_door.unlock();
                            }
                        }
                        // If set, analyze it and send to certain targets
                        else{
                            analyzeTarget(temp_targetPer_str);
                            for(targets_counter=0;targets_counter<targets.size();targets_counter++){
                                // Add target lock to make this thread safe
                                list_it_out=targetlist.find(targets[targets_counter]);
                                // If destination don't exist, throw a warning and skip this destination
                                if(list_it_out==targetlist.end()){
                                    std::cout<<m_utils::log("Error destination "+targets[targets_counter],2);
                                    continue;
                                }
                                // If destination exist, add target lock to make this thread safe
                                list_it_out->second->_door.lock();
                                // set source info in datacontainer
                                temp_datapack.srcinfo=list_it->first;
                                // push data to this destination
                                list_it_out->second->push_back(temp_datapack);
                                list_it_out->second->_door.unlock();
                            }
                        }
                    }    
                    // Release mutex of this source
                    list_it->second->_door.unlock();
                }    
                if(sourceEmptyFlag){
                    return false;
                }
                else{
                    return true;
                }
            }
            else
                return false;
        }
        
        // A independent thread is suggested to do this
        bool thread_run(){
            delivery_thread_exit_flag = false;
            delivery_thread = std::thread(& procom::delivery::endless_deliver, this, 0);
            return true;
        }
        
        // Thread exit Signal, to exit safely
        bool thread_break(){
            // To check is there any source not been cleaned.
            delivery_thread_exit_flag = true;
            delivery_thread.join();
            return true;
        }
        
    private:
        std::thread delivery_thread;
        bool delivery_thread_exit_flag;
        std::size_t data_persource_counter;
        std::size_t data_persource_max;
        std::size_t targets_counter;
        std::size_t targetPermissionStr_pos1,targetPermissionStr_pos2;
        std::string TargetPer_default;
        std::string TargetPer_deli;
        std::string temp_targetPer_str;
        bool sourceEmptyFlag;
        std::vector<std::string> targets;
        std::map<std::string, procom::bdatatrans * >::iterator list_it;
        std::map<std::string, procom::bdatatrans * >::iterator list_it_out;
        // Temporary map to store packed data.
        datacontainer temp_datapack;
        std::mutex _door;
        std::map<std::string, procom::bdatatrans * > sourcelist;
        std::map<std::string, procom::bdatatrans * > targetlist;
        // condition_variable flags
        std::condition_variable datain_cv;
        std::mutex datain_mtx;
        
        // To analyze target permission string which is from data map. Avoid input "*", this will be slow. 
        // Compare targetPermissionStr with "*" outside of this function.
        void analyzeTarget(std::string targetPermissionStr){
            targets.clear();targets.shrink_to_fit();
            if(targetPermissionStr == TargetPer_default){
                for(list_it=sourcelist.begin();list_it!=sourcelist.end();list_it++)
                    targets.push_back(list_it->first);
            }
            else{
                targetPermissionStr_pos1 = 0;
                for(targetPermissionStr_pos2 = targetPermissionStr.find(TargetPer_deli,0);;)
                {
                    targets.push_back(targetPermissionStr.substr(targetPermissionStr_pos1,targetPermissionStr_pos2-targetPermissionStr_pos1));
                    if(targetPermissionStr_pos2 == std::string::npos)
                        break;
                    targetPermissionStr_pos1=targetPermissionStr_pos2+TargetPer_deli.size();
                    targetPermissionStr_pos2 = targetPermissionStr.find(TargetPer_deli,targetPermissionStr_pos1);
                }
            }
        }
        
        // To clear useless pointers in list
        void clear_null_value(std::map<std::string, procom::bdatatrans * > & list){
            for(list_it = list.begin();list_it!=list.end();){
                if(list_it->second == NULL)
                    list_it = list.erase(list_it);
                else
                    list_it++;
            }
        }
        
        // A independent thread is suggested to do this
        bool endless_deliver(int mode=0){
            std::unique_lock<std::mutex> lck(datain_mtx);
            std::cout<<m_utils::log("delivery thread running",4);
            while(true){
                if(!deliver(mode)){
                    //after function running, if there is no data in all source, block until the timeout
                    // If any data pushed in, use global_cv to awake this to keep working on this.
                    datain_cv.wait_for(lck, std::chrono::milliseconds(1));
                }
                if(delivery_thread_exit_flag){
                    //Keep running deliver until all sources are empty
                    while(true){
                        if(!deliver(mode))
                            break;
                    }
                    std::cout<<m_utils::log("delivery thread exit safely",4);
                    return true;
                }
                continue;
            }
        }
    }; 
}

#endif