#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include "http.hh"
#include "coin.hh"

const char *host_name = "www.cryptocurrencychart.com";
const unsigned short port = 80;
std::string key;
std::string secret;
std::string coin_code;
std::string date_end;
std::string get_header(const std::string &api_call);
int get_coin(const std::string &coin_code, const std::string &date_end, bool verbose);
int get_all(const std::string &date_end, bool verbose);
currency_t curr;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "usage: http_coin -k KEY -s SECRET -c CODE -e END <-v> <-h>" << std::endl;
  std::cout << "-k: key" << std::endl;
  std::cout << "-s: secret" << std::endl;
  std::cout << "-c: CODE, coin code (e.g BTC) or ALL to get all coins" << std::endl;
  std::cout << "-e: END, end date, format as YYYY-MM-DD" << std::endl;
  std::cout << "-v: verbose" << std::endl;
  std::cout << "-h: help, exit" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  bool verbose = true;

  //load currencies list from local file
  curr.get_currencies_list("../coin_list.json");

  for (int idx = 1; idx < argc; idx++)
  {
    if (argv[idx][0] == '-')
    {
      switch (argv[idx][1])
      {
      case 'h':
        usage();
        break;
      case 'k':
        key = argv[idx + 1];
        idx++;
        break;
      case 's':
        secret = argv[idx + 1];
        idx++;
        break;
      case 'c':
        coin_code = argv[idx + 1];
        idx++;
        break;
      case 'e':
        date_end = argv[idx + 1];
        idx++;
        break;
      case 'v':
        verbose = true;
        break;
      default:
        usage();
      }
    }
    else
    {
      usage();
    }
  }

  if (key.empty() || secret.empty() || coin_code.empty() || date_end.empty())
  {
    usage();
  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //get all coins
  ///////////////////////////////////////////////////////////////////////////////////////

  if (coin_code.compare("ALL") == 0)
  {
    if (get_all(date_end, verbose) < 0)
    {
      return 1;
    }

  }

  ///////////////////////////////////////////////////////////////////////////////////////
  //get coin by CODE
  ///////////////////////////////////////////////////////////////////////////////////////

  else
  {
    if (get_coin(coin_code, date_end, verbose) < 0)
    {
      std::cout << "error in response" << std::endl;
      return 1;
    }
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//get_header
///////////////////////////////////////////////////////////////////////////////////////

std::string get_header(const std::string &api_call)
{
  std::string header;
  header += "GET ";
  header += api_call;
  header += " HTTP/1.1\r\n";
  header += "Host: ";
  header += host_name;
  header += "\r\n";
  header += "Accept: application/json\r\n";
  header += "Key: ";
  header += key;
  header += "\r\n";
  header += "Secret: ";
  header += secret;
  header += "\r\n";
  header += "Connection: close";
  header += "\r\n";
  header += "\r\n";
  return header;
}

///////////////////////////////////////////////////////////////////////////////////////
//get_coin
//format
//"http://www.cryptocurrencychart.com/api/coin/history/363/2017-01-01/2017-01-02/marketCap/usd"
///////////////////////////////////////////////////////////////////////////////////////

int get_coin(const std::string &coin_code, const std::string &date_end, bool verbose)
{
  http_t client(host_name, port);

  std::cout << "getting coin " << coin_code << " ... ";

  //open connection
  if (client.open() < 0)
  {
    std::cout << "cannot connect to: " << host_name << ":" << port << std::endl;
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //request
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string header;
  //get currencies list 
  header = get_header("/api/coin/list");

  std::string api_call("/api/coin/history/");

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //convert CODE to ID used in API
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string id = curr.get_coin_id(coin_code);
  if (id.empty())
  {
    std::cout << "code  " << coin_code << " not found..." << std::endl;
    return -1;
  }
  api_call += id;

  //calculate start date, one year before
  int year = std::stoi(date_end.substr(0, 4));
  std::string month = date_end.substr(5, 2);
  std::string day = date_end.substr(8, 2);
  year--;
  std::string syear = std::to_string(year);
  std::string date_start = syear;
  date_start += "-";
  date_start += month;
  date_start += "-";
  date_start += day;

  api_call += "/";
  api_call += date_start;
  api_call += "/";
  api_call += date_end;
  api_call += "/price/usd";
  header = get_header(api_call);

  if (verbose)
  {
    std::cout << "making request:" << "\n";
    std::cout << header << "\n";
  }

  //send request, using built in tcp_client_t socket
  if (client.write_all(header.c_str(), header.size()) < 0)
  {
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //response
  //we sent a close() server request, so we can use the read_all function
  //that checks for recv() return value of zero (connection closed)
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string fname;
  fname = coin_code;
  fname += ".response.txt";

  if (client.read_all_get_close(fname.c_str(), verbose) < 0)
  {
    return -1;
  }

  client.close_socket();

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //read from file
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::ifstream ifs(fname, std::ios::binary);
  std::stringstream buf;
  buf << ifs.rdbuf();
  std::string str_response = http_extract_body(buf.str());
  std::string str_json;

  //some responses include extra characters at start ("408b\r\n") and end (after "}")
  //detect valid JSON 
  if (str_response.at(0) != '{')
  {
    size_t pos = str_response.find("\r\n");
    str_json = str_response.substr(pos + 2);
    pos = str_json.find_last_of("}");
    if (std::string::npos == pos) //invalid JSON
    {
      return -1;
    }
    str_json = str_json.substr(0, pos + 1);
  }
  else
  {
    str_json = str_response;
  }

  fname = coin_code;
  fname += ".json";
  std::ofstream ofs(fname, std::ios::out | std::ios::binary);
  ofs.write(str_json.c_str(), str_json.size());
  ofs.close();

  std::string err("{\"error\":true,\"exception\":\"Monthly request limit reached.\"}");
  if (str_json.compare(err) == 0)
  {
    std::cout << err.c_str() << "\n";
    exit(0);
  }

  curr.get_history(&str_json[0]);
  coin_history_t hist = curr.m_coin_history.back();
  size_t size_hist = hist.m_history.size();
  std::cout << size_hist << " elements...";

  if (size_hist == 0)
  {
    std::cout << std::endl;
    return 0;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //save history
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  fname = hist.m_code;
  fname += ".csv";
  FILE *stream = fopen(fname.c_str(), "w");
  for (size_t idx_hist = 0; idx_hist < size_hist; idx_hist++)
  {
    fprintf(stream, "%s,", hist.m_history.at(idx_hist).m_date.c_str());
    fprintf(stream, "%s\n", hist.m_history.at(idx_hist).m_price.c_str());
  }
  fclose(stream);
  std::cout << "saved " << fname << std::endl;

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//get_all
//traverse all coin codes in list and HTTP get each one with a request
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_all(const std::string &date_end, bool verbose)
{
  size_t size_list = curr.m_coin_list.size();
  for (size_t idx_coin = 0; idx_coin < size_list; idx_coin++)
  {
    std::string coin_code = curr.m_coin_list.at(idx_coin).code;

    if (get_coin(coin_code, date_end, verbose) < 0)
    {
      return -1;
    }

  }

  return 0;
}

