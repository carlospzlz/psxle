#  Psxle - Psx Learning Environment
#  Copyright (C) 2018  Carlos Perez-Lopez
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.


from ctypes import *
import os


psxle_lib = cdll.LoadLibrary(
    os.path.join(os.path.dirname(__file__), 'libpsxle_c.so'))


psxle_lib.PSXLE_new.argtypes = None
psxle_lib.PSXLE_new.restype = c_void_p


class PSXLEInterface(object):
    """
    """
    def __init__(self):
        self.__obj = psxle_lib.PSXLE_new()

    def loadROM(self):
        """
        """
        pass

    def loadState(self):
        """
        """
        pass

    def getLegalActionSet(self):
        """
        """
        pass

    def getMinimalActionSet(self):
        """
        """
        pass

    def act(self, action):
        """
        """
        pass

    def getScreenDims(self):
        """
        Returns a tuple that contains (screen_width, screen_height)
        """
        #width = ale_lib.getScreenWidth(self.obj)
        #height = ale_lib.getScreenHeight(self.obj)
        #return (width, height)
        pass

    def getScreen(self, screen_data=None):
        """
        """
        pass

    def game_over(self):
        """
        """
        pass
        #return psxle_lib.game_over(self.obj)
