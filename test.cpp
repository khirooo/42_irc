#include <iostream>
#include <vector>
#include <sstream>


int main() {
  std::vector<int> v = {1, 2, 3, 4};
  std::cout << v.end() - v.begin() << std::endl;

  return 0;
}
