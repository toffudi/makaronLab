

from PyQt5.QtWidgets import QApplication, QDialog, QMainWindow, QWidget, QStyle
import wx

from . import consts
from . import Object
#, QWidget
class Element(Object.Object):
    def __init__(self,parent, impl=None, **kwargs): 
        super(Element, self).__init__(parent, impl, **kwargs)

    def resize(self, w:int, h:int):
        return self.wqD().doElement_Resize(w,h)

    def sizePolicy(self):
        return self.wqD().doElement_SizePolicy()

    def setSizePolicy(self, sizePolicy):
        return self.wqD().doElement_SetSizePolicy(sizePolicy)
                  