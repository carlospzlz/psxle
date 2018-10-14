/*  Psxle - Psx Learning Environment
 *  Copyright (C) 2018  Carlos Perez-Lopez
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "psxle_interface.h"
#include "../libpcsxcore/plugins.h"

#include <sstream>

using namespace psxle;

namespace
{
std::string welcomeMessage()
{
  std::ostringstream oss;
  oss << "P.S.X.L.E: Play-Station X Learning Environment\n"
      << "[Powered by PCSXR] by Carlos Perez-Lopez\n";
  return oss.str();
}
}  // namespace

PSXLEInterface::PSXLEInterface()
{
  Logger::Info << ::welcomeMessage();
  if (LoadPlugins() == -1)
  {
    Logger::Error << "Could not load plugins\n";
    exit(-1);
  }
}

void PSXLEInterface::loadROM(const std::string& rom_file) {}
