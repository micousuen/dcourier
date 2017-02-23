/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   utils.hpp
 * Author: root
 *
 * Created on August 12, 2016, 11:58 AM
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#define COMMENT_CHAR '#'

#include <sstream>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <iconv.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "Exception.h"

// Include <log generator>,<type convertor>,<timestamp string generator>, <getcyclecount of cpu>
namespace m_utils{
    #if defined (__i386__)
    static __inline__ unsigned long long GetCycleCount(void)
    {
            unsigned long long int x;
            __asm__ volatile("rdtsc":"=A"(x));
            return x;
    }
    #elif defined (__x86_64__)
    static __inline__ unsigned long long getCycleCount(void)
    {
            unsigned hi,lo;
            __asm__ volatile("rdtsc":"=a"(lo),"=d"(hi));
            return ((unsigned long long)lo)|(((unsigned long long)hi)<<32);
    }
    #endif
    
    // To generate current timestamp
    std::string currentTimeStr(bool withtimezone=true) {
        struct timeval epochtime;
        struct timezone tz;
        struct tm datetimestruct;
        char datetimec_str[100];
        std::string datetime;
        unsigned int ms;
        std::string ms_str;
        std::stringstream ss;

        gettimeofday(&epochtime, &tz);
        localtime_r(&epochtime.tv_sec, &datetimestruct);
        strftime(datetimec_str, 100, "%F %T.", &datetimestruct);
        datetime.assign(datetimec_str);
        ms = (unsigned int) (epochtime.tv_usec / 1000);
        if (ms < 100)
            datetime.append("0");
        if (ms < 10)
            datetime.append("0");
        ss.clear();
        ss << ms;
        ss >> ms_str;
        ss.clear();
        ss.str();
        datetime.append(ms_str);
        ms_str.clear();
        if(withtimezone){
            if (tz.tz_minuteswest < 0) {
                if (tz.tz_minuteswest / 60 > -10)
                    ss << "+0" << (-tz.tz_minuteswest / 60) << "00";
                else
                    ss << "+" << (-tz.tz_minuteswest / 60) << "00";
                ss>>ms_str;
                ss.clear();
                ss.str("");
            } else {
                if (tz.tz_minuteswest / 60 < 10)
                    ss << "-0" << (tz.tz_minuteswest / 60) << "00";
                else
                    ss << "-" << (tz.tz_minuteswest / 60) << "00";
                ss>>ms_str;
                ss.clear();
                ss.str("");
            }
            datetime.append(ms_str);
            return datetime;
        }
        else
            return datetime;
    }
    
    template<class out_type, class in_type>
    out_type typeconvert(const in_type input) {
        std::stringstream ss;
        out_type output;
        ss.clear();
        ss << input;
        ss >> output;
        ss.clear();
        ss.str();
        return static_cast<out_type>(output);
    }
    
    std::string log(std::string logcontent, bool showthread = true){
        if(showthread)
            return "["+currentTimeStr()+"]"+"\tThread: "+typeconvert<std::string>(syscall(SYS_gettid))+"\t"+logcontent+"\n";
        else
            return "["+currentTimeStr()+"]"+"\t"+logcontent+"\n";
    }
    
    const std::string logl[5]={"FATAL","ERROR","WARNING","DEBUG","INFO"};
    std::string log(std::string logcontent, int loglevel, bool showthread = true){
        if(showthread)
            return "["+currentTimeStr()+"]"+"\tThread: "+typeconvert<std::string>(syscall(SYS_gettid))+"\t"+logl[loglevel]+"\t"+logcontent+"\n";
        else
            return "["+currentTimeStr()+"]"+"\t"+logl[loglevel]+"\t"+logcontent+"\n";
    }
    
    std::string log(std::string logcontent, std::string loglevel, bool showthread = true){
        if(showthread)
            return "["+currentTimeStr()+"]"+"\tThread: "+typeconvert<std::string>(syscall(SYS_gettid))+"\t"+loglevel+"\t"+logcontent+"\n";
        else
            return "["+currentTimeStr()+"]"+"\t"+loglevel+"\t"+logcontent+"\n";
    }

    template<class threadinfotype,class logleveltype>
    std::string log(std::string logcontent, logleveltype loglevel, threadinfotype threadinfo, std::string IDinfo){
        return "["+currentTimeStr()+"]"+"\t"+typeconvert<std::string>(loglevel)+"\tThread: "+typeconvert<std::string>(threadinfo)+"\tID: "+IDinfo+"\t"+logcontent+"\n";
    }
    
    // Base class of utils. Utils classes are high performance utils, most of them need to be instanced before usage.  
    class util
    {
    public:
        const std::string author = "Micou";
    };
    
    // This function will make sure that there is a \n at the end of this string
    void str_std(std::string & string){
        if(string.back()!='\n')
            string.push_back('\n');
    }
    
    // If a high performance type convertor is needed, use this class to do type convert
    class hp_typeconvert:public util
    {
    private:
        std::stringstream ss;
    public:
        template<class out_type, class in_type>
        out_type convert(const in_type & input) {
            out_type output;
            ss.clear();
            ss << input;
            ss >> output;
            ss.clear();
            ss.str();
            return static_cast<out_type>(output);
        }
    };
    
    // class to read and write configure file. configure file path is needed
    class config:public util
    {
    public:
        // Initiate configure file class with a provided filepath
        config(std::string configFilePath){
            filepath=configFilePath;
            if(!ReadConfig(filepath,m)){
                std::cout<<log("Create a configure file under setting path");
                std::ofstream outfile;
                outfile.open(filepath.c_str(),std::ios::app|std::ios::out);
                outfile.close();
            }
        }
        
        // The filepath is needed when creating this class, if not provided, use ./settings.conf
        config(){
            filepath="./settings.conf";
            if(!ReadConfig(filepath,m)){
                std::cout<<log("Create a default configure file");
                std::ofstream outfile;
                outfile.open(filepath.c_str(),std::ios::app|std::ios::out);
                outfile.close();
            }
        }
        
        // Read configure file manually
        void read(){
            ReadConfig(filepath,m);
        }
        
        // Get the value of this configure file
        std::string get(std::string key){
            tempstr.clear();
            tempstr=GetValue(m, key);
            return tempstr;
        }
        
        // Write setting to configure file
        void write(std::string key, std::string value){
            WriteConfig_Update(filepath,key,value);
        }
        
        // Print all settings
        void print(){
            PrintConfig(m);
        }
        
    private:
        std::map<std::string,std::string> m;
        std::string filepath;
        std::string tempstr;
        //If this is a space char
        bool IsSpace(char c){
            if(' '==c||'\t'==c)
                return true;
            return false;
        }

        //If this is a comment line
        bool IsCommentChar(char c){
            switch(c){
                case COMMENT_CHAR:{ 
                    return true;
                    break;
                }
                default:{
                    return false;
                    break;
                }
            }
        }

        //Get rid of space at the begin and at the end of a string
        void Trim(std::string &str){
            if(str.empty()){
                return;
            }
            int i, start_pos,end_pos;
            for (i=0;i<str.size();i++){
                if(!IsSpace(str[i])){
                    break;
                }
            }
            if(i==str.size()){
                str="";
                return;
            }
            start_pos=i;
            for(i=str.size()-1;i>=0;i--){
                if(!IsSpace(str[i])){
                    break;
                }
            }
            end_pos=i;
            str=str.substr(start_pos,end_pos-start_pos+1);
            return;
        }

        //To analyze a line and split key and value out of this line.
        bool AnalyzeLine(const std::string & line,std::string & key, std::string & value){
            if(line.empty())
                return false;
            int start_pos=0,end_pos=line.size()-1,pos;
            if((pos=line.find(COMMENT_CHAR))!=-1){
                if(pos==0){
                    return false;
                }
                end_pos=pos-1;
            }
            //Delete those part that is comment part
            std::string new_line = line.substr(start_pos,end_pos+1-start_pos);
            //If there is no "=", that means this line is not a standard line. Ignore this line.
            if((pos=new_line.find("="))==-1){
                return false;
            }
            key=new_line.substr(0,pos);
            value=new_line.substr(pos+1,end_pos+1-(pos+1));

            Trim(key);
            if(key.empty()){
                return false;
            }
            Trim(value);
            return true;
        }

        // Open file and read keys and values, return them as a map<string,string>
        bool ReadConfig(const std::string & filename, std::map<std::string,std::string> & m){
            m.clear();
            std::ifstream infile(filename.c_str());
            if(!infile){
                std::cout<<log("Configure file Open Error in reading!");
                return false;
            }
            std::string line,key,value;
            while(getline(infile,line)){
                if(AnalyzeLine(line,key,value)){
                    m[key]=value;
                }
            }
            if(infile.is_open())
                infile.close();
            return true;
        }
        
        // get the value of configure file according to the key
        std::string GetValue(std::map<std::string,std::string> m, std::string Key){
            std::map<std::string,std::string>::iterator m_it;
            m_it = m.find(Key);
            if(m_it!=m.end()){
                return m_it->second;
            }
            else{
                return "";
            }
        }

        // Open file and write keys and values
        bool WriteConfig(const std::string & filename, const std::string & key, const std::string & value){

            std::ofstream outfile;
            outfile.open(filename.c_str(),std::ios::app|std::ios::out);
            if(!outfile){
                std::cout<<log("File Open Error in writing!");
                return false;
            }
            std::string strtowrite;
            strtowrite=key+" = "+value+"\n";

            outfile.seekp(std::ios::end);
            outfile.write(strtowrite.c_str(),strtowrite.size());
            if(outfile.is_open())
                outfile.close();
        }

        // Update configure file(will update the value of existing key)
        bool WriteConfig_Update(const std::string & filename,const std::string & key, const std::string & value){

            std::map<std::string,std::string> configfilecontent;
            std::map<std::string,std::string>::iterator configfilecontent_it;
            ReadConfig(filename,configfilecontent);
            configfilecontent_it=configfilecontent.find(key);
            std::string strtowrite;
            std::ofstream outfile;
            if(configfilecontent_it!=configfilecontent.end()){
                configfilecontent_it->second = value;
            }
            else{
                configfilecontent.insert(std::make_pair(key,value));
            }

            outfile.open(filename.c_str(),std::ios::out|std::ios::trunc);
            if(!outfile){
                std::cout<<log("File Open Error in updating!");
                return false;
            }
            for(configfilecontent_it=configfilecontent.begin();configfilecontent_it!=configfilecontent.end();configfilecontent_it++){
                strtowrite=configfilecontent_it->first+" = "+configfilecontent_it->second+"\n";
                outfile.write(strtowrite.c_str(),strtowrite.size());
                strtowrite.clear();
            }
            if(outfile.is_open())
                outfile.close();
            return true;
        }

        // Print configure file out using cout
        void PrintConfig(std::map<std::string, std::string> & m){
            std::map<std::string, std::string>::const_iterator mite;
            for (mite=m.begin(); mite != m.end(); ++mite) {
                std::cout <<log("settings: "+mite->first + "=" + mite->second);
            }
        }
        
    };
}

#endif /* UTILS_HPP */

