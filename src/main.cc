#include <iostream>
#include "libpcsxcore/misc.h"

int main()
{
  if (LoadCdrom() == -1)
  {
    std::cout << "Could not load CD-ROM" << std::endl;
  }
  return 0;
}
