
from ..ModuleFactory import *
from q3.Timer import Timer

from q3.ui.PushButtonWidget import PushButtonWidget
import q3.ui as ui
class ModuleImplClock(ModuleImplBase):
    def __init__(self,**kwargs):
        self._intervalType = 'Stop'
        self._interval = 0
        self._intervalTypeDomainValues = {
            1:'1ms',
            10:'10ms',
            100:'100ms',
            1000:'1s',
            10000:'10s',
            'Stop':'Stop'
            }
        super(ModuleImplClock,self).__init__(**kwargs)
        self._moduleType = ModuleType.ATOMIC 

    def init(self,**kwargs) -> dict: 
        self._init = {} #q3c.cpc_init(initParms)
        cp = self._init['customProperties'] if 'customProperties' in self._init else {} 
        cp['intervalType']={
            'desc':f'Type of interval to emulate, currently handled:{self._intervalTypeDomainValues}',
            'required':True,
            'default':'Stop',
            'domainValues':self._intervalTypeDomainValues,
            'onChange':self.onIntervalTypeChange
        }
        self._customProperties=cp
        return self._init

    def onIntervalTypeChange(self, event=None):
        print (f'[onIntervalTypeChange]:{event}')
        self._intervalType = event
        if isinstance(self._intervalType,int):
            self._interval = self._intervalType
        else:
            self._interval = 0
        pass

    def open(self):
        self._timer = Timer()
        self._nod = self.newIO(
            name='Y',
            ioType = IoType.OUTPUT
            )
        #self._centralWidget = 
        pass

    def onSwitchClockState(self, state):
        if state: #off
            self._prevInterval = self._interval
            self._interval = 0
        else: #on
            self._interval = self._prevInterval

    def onToggleClockState(self, state):
        y = self.sig('Y')
        y.setValue(state)
        self._timer.reset()


    def __afterViewCreated__(self, viewImpl=None): 
        if viewImpl!=None:
            #tw = ui.STable()
            lay = ui.BoxLayout(viewImpl)
            pb = PushButtonWidget(lay)
            pb.onChangeState = self.onSwitchClockState
            pb2 = PushButtonWidget(lay)
            pb2.onChangeState = self.onToggleClockState
            #tw.setRowCount(1)
            #tw.setColumnCount(2)
            #tw.setItem(0,0,pb)
            #tw.setCellWidget(0,1,pb2)
            lay.addH(pb)
            lay.addH(pb2)
            #pb = PushButtonWidget(viewImpl)
            viewImpl.setCentralWidget(lay)

    # used by custom property builder to set default/current value of corresponding property    
    def intervalType(self):
        return self._intervalType    

    def calc(self):
        if self._interval>0 and self._timer.millisDelta()>self._interval:
            y = self.sig('Y')
            y.setValue(not y.value())
            self._timer.reset()

        pass 


    def interval(self):
        return self._interval

    def setInterval(self, interval:int):
        if interval in self._intervalTypeDomainValues:
            self._intervalType = interval
        if interval == 'Stop':
            interval = 0
        self._interval = interval