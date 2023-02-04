#include <iostream>
#include <vector>
#include <sstream>

std::vector<std::string> parse_string(const std::string& str) {
  std::vector<std::string> result;
  std::istringstream iss(str);

  std::string prefix;
  if (str[0] == ':') {
    std::getline(iss, prefix, ' ');
    result.push_back(prefix.substr(1));
  }

  std::string cmd;
  iss >> cmd;
  result.push_back(cmd);

  std::string param;
  while (std::getline(iss, param, ' ')) {
    if (param[0] == ':') {
      std::string  tmp;
      std::getline(iss, tmp, '\r');
      result.push_back(param.substr(1) + ' ' + tmp);
      break;
    }
    result.push_back(param);
  }

  return result;
}

int main() {
  std::string test_string = ":prefix cmd param1   param2 :param3     with    spaces\r\n";
  auto result = parse_string(test_string);

  std::cout << "Prefix: " << result[0] << std::endl;
  std::cout << "Command: " << result[1] << std::endl;
  std::cout << "Parameters: ";
  for (int i = 2; i < result.size(); i++) {
    std::cout << result[i] << "|";
  }
  std::cout << std::endl;

  return 0;
}
