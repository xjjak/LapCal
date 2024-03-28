from pipe import Pipe

from math import sin, cos

class FeatureExtractor(Pipe):
    def __init__(self):
        self.ready = True

    def feed(self, line, output=None):
        return True

    def available(self):
        return self.ready
        
    def get_features(self):
        return []

    def retrieve(self):
        return self.get_features()
    

class StandardFeatureExtractor(FeatureExtractor):
    def __init__(self, n_readings = 3):
        self.ready = False
        self.readings = [None]*n_readings
        self.head = 0
        # self.output = [0]*17

        
    def feed(self, line, output=None):
        sensors = line.strip().split(":")
        if len(sensors) != 7:
            return False

        if output:
            self.output = output[:]

        self.readings[self.head] = [0]*(6*6)
        reading = self.readings[self.head]
        for (idx, sensor) in enumerate(sensors[1:]):
            if sensor == "-":
                continue

            vals = sensor.split(";")
            if len(vals) != 6:
                return False
            
            reading[idx*6    :idx*6 + 6] = [
                float(val.strip()) for val in vals
            ]

        self.head = (self.head + 1) % len(self.readings)

        if self.readings[self.head] != None:
            self.ready = True

        return True
            

    def get_features(self):
        self.ready = False
        
        readings = [
            self.readings[self.head - i]
            for i in range(len(self.readings))
        ]

        return [
            feature
            for reading in readings
            for feature in reading
        ]

    
class TrigFeatureExtractor(FeatureExtractor):
    def __init__(self, n_readings = 3):
        self.ready = False
        self.readings = [None]*n_readings
        self.head = 0
        self.output = [0]*17

        
    def feed(self, line, output=None):
        sensors = line.strip().split(":")
        if len(sensors) != 7:
            return False

        if output:
            self.output = output[:]

        self.readings[self.head] = [0]*(6*9)
        reading = self.readings[self.head]
        for (idx, sensor) in enumerate(sensors[1:]):
            if sensor == "-":
                continue

            vals = sensor.split(";")
            if len(vals) != 6:
                return False
            
            reading[idx*9    :idx*9 + 3] = [float(val.strip())      for val in vals[:3]]
            reading[idx*9 + 3:idx*9 + 6] = [sin(float(val.strip())) for val in vals[3:]]
            reading[idx*9 + 6:idx*9 + 9] = [cos(float(val.strip())) for val in vals[3:]]

        self.head = (self.head + 1) % len(self.readings)

        if self.readings[self.head] != None:
            self.ready = True

        return True

    def get_features(self):
        self.ready = False
        
        readings = [
            self.readings[self.head - 2],
            self.readings[self.head - 1],
            self.readings[self.head],
            [float(i) for i in self.output]
        ]

        return [
            feature
            for reading in readings
            for feature in reading
        ]
