from math import sin, cos

class FeatureExtractor:
    def __init__(self):
        self.ready = True

    def feed(self, line, output=None):
        return True

    def available(self):
        return self.ready
        
    def get_features(self):
        return []


class TrigFeatureExtractor(FeatureExtractor):
    def __init__(self, n_readings = 3):
        self.ready = False
        self.readings = [None]*n_readings
        self.head = 0

        
    def feed(self, line, output=None):
        sensors = line.split(":")
        if len(sensors) != 7:
            return False

        for (idx, sensor) in enumerate(sensors[1:]):
            self.readings[self.head] = [0]*(6*9*len(self.readings) + 17)
            reading = self.readings[self.head]
            
            if sensor != "-":
                continue

            vals = sensor.split(";")
            if len(vals) != 6:
                return False
            
            reading[idx*9    :idx*9 + 3] = [float(val.strip())      for val in vals[:3]]
            reading[idx*9 + 3:idx*9 + 6] = [sin(float(val.strip())) for val in vals[3:]]
            reading[idx*9 + 6:idx*9 + 9] = [cos(float(val.strip())) for val in vals[3:]]

        if self.readings[(self.head + 1) % len(self.readings)] != None:
            self.ready = True
            
        self.head = (self.head + 1) % len(self.readings)
            

    def get_features(self):
        self.ready = False
        
        readings = [
            self.readings[self.head - 2],
            self.readings[self.head - 1],
            self.readings[self.head],
        ]
        
        return [
            feature
            for reading in readings
            for feature in reading
        ]
