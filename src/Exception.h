// SocketException class


#ifndef Exception_class
#define Exception_class

#include <string>

class SocketException:public std::exception
{
 public:
  SocketException ( std::string s ) : m_s ( s ) {};
  ~SocketException (){};

  std::string description() { return m_s; }

 private:

  std::string m_s;

};

class CNCDCException:public std::exception
{
public:
    CNCDCException ( std::string s ) : m_s ( s ) {};
    ~CNCDCException (){};

  std::string description() { return m_s; }

private:

    std::string m_s;
};

#endif