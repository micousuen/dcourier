/* 
 * File:   file_out.hpp
 * Author: root
 *
 * Created on August 30, 2016, 3:40 PM
 */

#ifndef OUT_FILE_HPP
#define	OUT_FILE_HPP

#include <fstream>
#include <deque>
#include <sys/stat.h>
#include "Com.hpp"
#include "utils.hpp"

namespace out_components{
    class file_out_single:public procom::odatatrans, public procom::run_as_thread
    {
    public:
        // Set output file path
        file_out_single(std::string fileoutpath = "./DataOut.txt"){
            filepath=fileoutpath;
            if(!outfile.is_open()){
                outfile.open(filepath.c_str(),std::ios::app|std::ios::out);
            }
        }
        ~file_out_single(){
            if(outfile.is_open())
                outfile.close();
        }
        /*in lack of a file path check function*/

        // Write all buffer data to output file -  append after data
        bool write_app(){
            if(buffer_is_valid()){
                if(!outfile.is_open()){
                    outfile.open(filepath.c_str(),std::ios::app|std::ios::out);
                }
                data_it=data_trans_buffer.begin();
                datanum=get_buffer_size();
                for(dataout_counter=0;dataout_counter<datanum;dataout_counter++){
                    _door.lock();
                    datatemp=data_it->data;
                    outfile.write(datatemp.c_str(),datatemp.size());
                    data_trans_buffer.pop_front();
                    data_trans_buffer.shrink_to_fit();
                    data_it=data_trans_buffer.begin();
                    _door.unlock();
                }
                return true;
            }
            else
                return false;
        }

        // Write all buffer data to output file - overwrite data
        bool write_overwrite(){
            if(buffer_is_valid()){
                if(!outfile.is_open()){
                    outfile.open(filepath.c_str(),std::ios::out);
                }
                for(data_it=data_trans_buffer.begin();data_it!=data_trans_buffer.end();){
                    _door.lock();
                    datatemp=data_it->data;
                    outfile.write(datatemp.c_str(),datatemp.size());
                    data_it=data_trans_buffer.erase(data_it);
                    _door.unlock();
                }
                return true;
            }
            else
                return false;
        }

        // Run this as a thread
        bool thread_run(){
            file_out_single_thread_exit_flag=false;
            file_out_single_thread = std::thread(& out_components::file_out_single::endless_write, this);
            return true;
        }
        
        // Try to break file out
        bool thread_break(){
            file_out_single_thread_exit_flag=true;
            file_out_single_thread.join();
            return true;
        }

        // Try to close file is file is opened
        void try_close_file(){
            if(outfile.is_open())
                outfile.close();
        }

    private:
        std::string datatemp;
        std::size_t dataout_counter;
        std::size_t datanum;
        std::thread file_out_single_thread;
        bool file_out_single_thread_exit_flag;
        std::string filepath;
        std::fstream outfile;
        std::deque< procom::datacontainer >::iterator data_it;
        // Run as a single thread
        void endless_write(){
            std::cout<<m_utils::log("file_out_rolling thread running",4);
            while(true){
                if(!write_app()){
                    usleep(CHECK_INTERVAL);
                }
                if(file_out_single_thread_exit_flag){
                    std::cout<<m_utils::log("single file out thread exit",4);
                    break;
                }
                continue;
            }
        }
        
    };

    class file_out_rolling:public procom::odatatrans, public procom::run_as_thread
    {
    public:
        // Set output file path
        file_out_rolling(std::string fileoutpath = "./DataOut_0.txt", unsigned int maxFileSize_megaByte = 5, int maxFileNum_file = 5){
            CurrentFileCount=0;
            filepath=fileoutpath;
            maxFileSize=maxFileSize_megaByte;
            maxFileNum=maxFileNum_file;
            for(int iround=0;iround<maxFileNum+1;iround++){
                filesize=Get_File_Size(filepath.c_str());
                filetime1=Get_File_mTime(filepath.c_str());
                filetime2=Get_File_mTime(Generate_nextfilename(filepath).c_str());
                if(filetime2 > filetime1){
                    filepath=Generate_nextfilename(filepath);
                    continue;
                }
                else{
                    break;
                }
            }
        }

        ~file_out_rolling(){
            if(outfile.is_open())
                outfile.close();
        }
        // Write file
        bool write_app(){
            if(buffer_is_valid()){
//                std::cout<<m_utils::log("The size of out buffer is: "+std::to_string(buffer_size()),3);
                // Check if file is opened
                if(!outfile.is_open())
                    outfile.open(filepath.c_str(), std::ios::out|std::ios::app);
                
                //std::cout<<m_utils::log("Current file is "+filepath,4);
                procom::odatatrans::get_front_databunch(dataout);
                // Write to data out file
                for(std::vector<std::string>::iterator bg=dataout.begin();bg!=dataout.end();bg++){
                    // get data from buffer, and pop data out after this operation
                    tempstr=*bg;
                    m_utils::str_std(tempstr);
                    outfile.write(tempstr.c_str(),tempstr.size());    
                }
                dataout.clear();
                // Judge if file is over max size. If it is, prepare to turn to next file to write data.
                filesize=Get_File_Size(filepath.c_str());
                if(filesize>(1000000*maxFileSize)){
                    outfile.close();
                    filepath=Generate_nextfilename(filepath);
                    // Delete the earliest log file
                    filesize=Get_File_Size(filepath.c_str());
                    if(filesize>(1000000*maxFileSize)){
                        ::remove(filepath.c_str());
                    }
                }
                outfile.close();
                return true;
            }
            else
                return false;
        }
        // Run this as a thread
        bool thread_run(){
            file_out_rolling_thread_exit_flag=false;
            file_out_rolling_thread = std::thread(& out_components::file_out_rolling::endless_write, this);
            file_out_rolling_thread.detach();
            return true;
        }
        
        // Try to break file out
        bool thread_break(){
            file_out_rolling_thread_exit_flag=true;
            usleep(THREAD_EXIT_WAIT);
            return true;
        }
        
        // Try to close file is file is opened
        void try_close_file(){
            if(outfile.is_open())
                outfile.close();
        }

    private:
        std::thread file_out_rolling_thread;
        bool file_out_rolling_thread_exit_flag;
        std::string filepath;
        std::fstream outfile;
        std::size_t filesize;
        std::size_t datanum,dataout_counter;
        std::size_t filetime1, filetime2;
        double maxFileSize;
        int maxFileNum;
        unsigned int CurrentFileCount;
        struct stat buf;
        std::size_t insert_it;
        std::size_t delimiter;
        std::string tempstr;
        std::stringstream stream;
        std::deque< procom::datacontainer >::iterator data_it;
        std::vector<std::string> dataout;

        // Run this in a single thread
        void endless_write(){
            std::cout<<m_utils::log("file_out_rolling thread running",4);
            while(true){
                if(!write_app()){
                    usleep(CHECK_INTERVAL);
                }
                if(file_out_rolling_thread_exit_flag){
                    while(true){
                        if(!write_app())
                            break;
                    }
                    std::cout<<m_utils::log("rolling file out thread exit",4);
                    break;
                }
                continue;
            }
        }
        // Return target file size
        unsigned long Get_File_Size(const char* filename){
            if(stat(filename,&buf)<0)
                return 0;
            else
                return (unsigned long)buf.st_size;
        }
        // Return target file modify time
        unsigned long Get_File_mTime(const char* filename){
            memset(&buf,0,sizeof(buf));
            if(stat(filename,&buf)<0)
                return 0;
            else
                return (unsigned long)(buf.st_mtim.tv_sec*1000000000+buf.st_mtim.tv_nsec);
        }
        // Generate filename
        std::string Generate_nextfilename(std::string OutPath){
            insert_it=OutPath.rfind("_");
            delimiter=OutPath.rfind(".");
            if(insert_it!=std::string::npos && delimiter!=std::string::npos){
                tempstr.clear();tempstr.shrink_to_fit();
                tempstr=OutPath.substr(insert_it+1,delimiter-insert_it-1);
                stream<<tempstr;
                stream>>CurrentFileCount;
                stream.clear();stream.str("");
                tempstr.clear();tempstr.shrink_to_fit();
                CurrentFileCount++;
                // Rolling File function will be opened only if MaxFileNum>0. If MaxFileNum set to -1, No file delete will happened.
                if(maxFileNum>0){
                    // Rolling File 
                    if(!(CurrentFileCount<maxFileNum)){
                        CurrentFileCount=0;
                    }
                }
                stream<<"_"<<CurrentFileCount;
                stream>>tempstr;
                stream.clear();stream.str("");
            }
            else{
                tempstr.clear();tempstr.shrink_to_fit();
                stream<<"_"<<CurrentFileCount;
                stream>>tempstr;
                stream.clear();stream.str("");
            }
            // Prepare for rolling filename
            if(insert_it==std::string::npos){
                OutPath.insert(delimiter,tempstr.c_str());
            }
            else{
                OutPath.replace(insert_it,delimiter-insert_it,tempstr.c_str());
            }
            return OutPath;
        }
    };
}


#endif	/* FILE_OUT_HPP */

