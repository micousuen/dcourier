/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on July 22, 2016, 1:48 PM
 */

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <sys/syscall.h>

// Tools(include log generator, configure file reader)
#include "utils.hpp"
// Packed communication library (include network communication and in-program communication)
#include "Com.hpp"
// For local file out
#include "out_file.hpp"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    
    m_utils::config cnclist("./setting/CNCList.txt");
    
    procom::idatatrans in_test;
    out_components::file_out_rolling out("./dataout/dataout_0.txt",1,3);
    procom::delivery dl;
    
    dl.add_source("test_in",&in_test);
    dl.add_target("file_out",&out);
    
    in_test.push_back_data("-------------------------------------------------------------------");
    in_test.push_back_data("Delimiter");
    in_test.push_back_data("-------------------------------------------------------------------");
    
    dl.thread_run();
    out.thread_run();
    
    for(int i=0;i<100;i++){
        for(int j=0;j<10000;j++){
            in_test.push_back_data("This is "+::to_string(i*10000+j));
        }
    }
    
    dl.thread_break();
    out.thread_break();
    
    return 0;
}

