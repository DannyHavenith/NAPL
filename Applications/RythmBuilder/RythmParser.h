#ifndef __RYTHMPARSER_H__
#define __RYTHMPARSER_H__

#include <boost/filesystem/path.hpp>
#include <string>
bool ParseRythm(  const boost::filesystem::path &instrument_path, const std::string &content);

#endif //__RYTHMPARSER_H__